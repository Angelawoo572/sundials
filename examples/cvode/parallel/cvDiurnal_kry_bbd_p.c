/*
 * -----------------------------------------------------------------
 * Programmer(s): S. D. Cohen, A. C. Hindmarsh, M. R. Wittman, and
 *                Radu Serban  @ LLNL
 * -----------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2025, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * -----------------------------------------------------------------
 * Example problem:
 *
 * An ODE system is generated from the following 2-species diurnal
 * kinetics advection-diffusion PDE system in 2 space dimensions:
 *
 * dc(i)/dt = Kh*(d/dx)^2 c(i) + V*dc(i)/dx + (d/dy)(Kv(y)*dc(i)/dy)
 *                 + Ri(c1,c2,t)      for i = 1,2,   where
 *   R1(c1,c2,t) = -q1*c1*c3 - q2*c1*c2 + 2*q3(t)*c3 + q4(t)*c2 ,
 *   R2(c1,c2,t) =  q1*c1*c3 - q2*c1*c2 - q4(t)*c2 ,
 *   Kv(y) = Kv0*exp(y/5) ,
 * Kh, V, Kv0, q1, q2, and c3 are constants, and q3(t) and q4(t)
 * vary diurnally. The problem is posed on the square
 *   0 <= x <= 20,    30 <= y <= 50   (all in km),
 * with homogeneous Neumann boundary conditions, and for time t in
 *   0 <= t <= 86400 sec (1 day).
 * The PDE system is treated by central differences on a uniform
 * mesh, with simple polynomial initial profiles.
 *
 * The problem is solved by CVODE on NPE processors, treated
 * as a rectangular process grid of size NPEX by NPEY, with
 * NPE = NPEX*NPEY. Each processor contains a subgrid of size MXSUB
 * by MYSUB of the (x,y) mesh. Thus the actual mesh sizes are
 * MX = MXSUB*NPEX and MY = MYSUB*NPEY, and the ODE system size is
 * neq = 2*MX*MY.
 *
 * The solution is done with the BDF/GMRES method (i.e. using the
 * SUNLinSol_SPGMR linear solver) and a block-diagonal matrix with
 * banded blocks as a preconditioner, using the CVBBDPRE module.
 * Each block is generated using difference quotients, with
 * half-bandwidths mudq = mldq = 2*MXSUB, but the retained banded
 * blocks have half-bandwidths mukeep = mlkeep = 2.
 * A copy of the approximate Jacobian is saved and conditionally
 * reused within the preconditioner routine.
 *
 * The problem is solved twice -- with left and right preconditioning.
 *
 * Performance data and sampled solution values are printed at
 * selected output times, and all performance counters are printed
 * on completion.
 *
 * This version uses MPI for user routines.
 * Execute with number of processors = NPEX*NPEY (see constants below).
 * --------------------------------------------------------------------
 */

#include <cvode/cvode.h>        /* prototypes for CVODE fcts., consts.  */
#include <cvode/cvode_bbdpre.h> /* access to CVBBDPRE module            */
#include <math.h>
#include <mpi.h>                      /* MPI constants and types */
#include <nvector/nvector_parallel.h> /* access to MPI-parallel N_Vector      */
#include <stdio.h>
#include <stdlib.h>
#include <sundials/sundials_types.h> /* definitions of sunrealtype, sunbooleantype */
#include <sunlinsol/sunlinsol_spgmr.h> /* access to SPGMR SUNLinearSolver      */

/* helpful macros */

#ifndef SQR
#define SQR(A) ((A) * (A))
#endif

/* Problem Constants */

#define ZERO SUN_RCONST(0.0)

#define NVARS    2                    /* number of species         */
#define KH       SUN_RCONST(4.0e-6)   /* horizontal diffusivity Kh */
#define VEL      SUN_RCONST(0.001)    /* advection velocity V      */
#define KV0      SUN_RCONST(1.0e-8)   /* coefficient in Kv(y)      */
#define Q1       SUN_RCONST(1.63e-16) /* coefficients q1, q2, c3   */
#define Q2       SUN_RCONST(4.66e-16)
#define C3       SUN_RCONST(3.7e16)
#define A3       SUN_RCONST(22.62) /* coefficient in expression for q3(t) */
#define A4       SUN_RCONST(7.601) /* coefficient in expression for q4(t) */
#define C1_SCALE SUN_RCONST(1.0e6) /* coefficients in initial profiles    */
#define C2_SCALE SUN_RCONST(1.0e12)

#define T0      ZERO               /* initial time */
#define NOUT    12                 /* number of output times */
#define TWOHR   SUN_RCONST(7200.0) /* number of seconds in two hours  */
#define HALFDAY SUN_RCONST(4.32e4) /* number of seconds in a half day */
#define PI      SUN_RCONST(3.1415926535898) /* pi */

#define XMIN ZERO /* grid boundaries in x  */
#define XMAX SUN_RCONST(20.0)
#define YMIN SUN_RCONST(30.0) /* grid boundaries in y  */
#define YMAX SUN_RCONST(50.0)

#define NPEX 2  /* no. PEs in x direction of PE array */
#define NPEY 2  /* no. PEs in y direction of PE array */
                /* Total no. PEs = NPEX*NPEY */
#define MXSUB 5 /* no. x points per subgrid */
#define MYSUB 5 /* no. y points per subgrid */

#define MX (NPEX * MXSUB) /* MX = number of x mesh points */
#define MY (NPEY * MYSUB) /* MY = number of y mesh points */
                          /* Spatial mesh is MX by MY */
/* CVodeInit Constants */

#define RTOL  SUN_RCONST(1.0e-5) /* scalar relative tolerance */
#define FLOOR SUN_RCONST(100.0)  /* value of C1 or C2 at which tolerances */
                                 /* change from relative to absolute      */
#define ATOL (RTOL * FLOOR)      /* scalar absolute tolerance */

/* Type : UserData
   contains problem constants, extended dependent variable array,
   grid constants, processor indices, MPI communicator */

typedef struct
{
  sunrealtype q4, om, dx, dy, hdco, haco, vdco;
  sunrealtype uext[NVARS * (MXSUB + 2) * (MYSUB + 2)];
  int my_pe, isubx, isuby;
  sunindextype nvmxsub, nvmxsub2, Nlocal;
  MPI_Comm comm;
}* UserData;

/* Prototypes of private helper functions */

static void InitUserData(int my_pe, sunindextype local_N, MPI_Comm comm,
                         UserData data);
static void SetInitialProfiles(N_Vector u, UserData data);
static void PrintIntro(int npes, sunindextype mudq, sunindextype mldq,
                       sunindextype mukeep, sunindextype mlkeep);
static void PrintOutput(void* cvode_mem, int my_pe, MPI_Comm comm, N_Vector u,
                        sunrealtype t);
static void PrintFinalStats(void* cvode_mem);
static void BSend(MPI_Comm comm, int my_pe, int isubx, int isuby,
                  sunindextype dsizex, sunindextype dsizey, sunrealtype uarray[]);
static void BRecvPost(MPI_Comm comm, MPI_Request request[], int my_pe, int isubx,
                      int isuby, sunindextype dsizex, sunindextype dsizey,
                      sunrealtype uext[], sunrealtype buffer[]);
static void BRecvWait(MPI_Request request[], int isubx, int isuby,
                      sunindextype dsizex, sunrealtype uext[],
                      sunrealtype buffer[]);

static void fucomm(sunrealtype t, N_Vector u, void* user_data);

/* Prototype of function called by the solver */

static int f(sunrealtype t, N_Vector u, N_Vector udot, void* user_data);

/* Prototype of functions called by the CVBBDPRE module */

static int flocal(sunindextype Nlocal, sunrealtype t, N_Vector u, N_Vector udot,
                  void* user_data);

/* Private function to check function return values */

static int check_retval(void* returnvalue, const char* funcname, int opt, int id);

/***************************** Main Program ******************************/

int main(int argc, char* argv[])
{
  SUNContext sunctx;
  UserData data;
  SUNLinearSolver LS;
  void* cvode_mem;
  sunrealtype abstol, reltol, t, tout;
  N_Vector u;
  int iout, my_pe, npes, retval, jpre;
  sunindextype neq, local_N, mudq, mldq, mukeep, mlkeep;
  MPI_Comm comm;

  data      = NULL;
  LS        = NULL;
  cvode_mem = NULL;
  u         = NULL;

  /* Set problem size neq */
  neq = NVARS * MX * MY;

  /* Get processor number and total number of pe's */
  MPI_Init(&argc, &argv);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &npes);
  MPI_Comm_rank(comm, &my_pe);

  /* Create the SUNDIALS context */
  retval = SUNContext_Create(comm, &sunctx);
  if (check_retval(&retval, "SUNContext_Create", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  if (npes != NPEX * NPEY)
  {
    if (my_pe == 0)
    {
      fprintf(stderr,
              "\nMPI_ERROR(0): npes = %d is not equal to NPEX*NPEY = %d\n\n",
              npes, NPEX * NPEY);
    }
    MPI_Finalize();
    return (1);
  }

  /* Set local length */
  local_N = NVARS * MXSUB * MYSUB;

  /* Allocate and load user data block */
  data = (UserData)malloc(sizeof *data);
  if (check_retval((void*)data, "malloc", 2, my_pe)) { MPI_Abort(comm, 1); }
  InitUserData(my_pe, local_N, comm, data);

  /* Allocate and initialize u, and set tolerances */
  u = N_VNew_Parallel(comm, local_N, neq, sunctx);
  if (check_retval((void*)u, "N_VNew_Parallel", 0, my_pe))
  {
    MPI_Abort(comm, 1);
  }
  SetInitialProfiles(u, data);
  abstol = ATOL;
  reltol = RTOL;

  /* Call CVodeCreate to create the solver memory and specify the
   * Backward Differentiation Formula */
  cvode_mem = CVodeCreate(CV_BDF, sunctx);
  if (check_retval((void*)cvode_mem, "CVodeCreate", 0, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  /* Set the pointer to user-defined data */
  retval = CVodeSetUserData(cvode_mem, data);
  if (check_retval(&retval, "CVodeSetUserData", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  /* Call CVodeInit to initialize the integrator memory and specify the
   * user's right hand side function in u'=f(t,u), the initial time T0, and
   * the initial dependent variable vector u. */
  retval = CVodeInit(cvode_mem, f, T0, u);
  if (check_retval(&retval, "CVodeInit", 1, my_pe)) { return (1); }

  /* Call CVodeSStolerances to specify the scalar relative tolerance
   * and scalar absolute tolerances */
  retval = CVodeSStolerances(cvode_mem, reltol, abstol);
  if (check_retval(&retval, "CVodeSStolerances", 1, my_pe)) { return (1); }

  /* Create SPGMR solver structure -- use left preconditioning
     and the default Krylov dimension maxl */
  LS = SUNLinSol_SPGMR(u, SUN_PREC_LEFT, 0, sunctx);
  if (check_retval((void*)LS, "SUNLinSol_SPGMR", 0, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  /* Attach SPGMR solver structure to CVode interface */
  retval = CVodeSetLinearSolver(cvode_mem, LS, NULL);
  if (check_retval(&retval, "CVodeSetLinearSolver", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  /* Initialize BBD preconditioner */
  mudq = mldq = NVARS * MXSUB;
  mukeep = mlkeep = NVARS;
  retval = CVBBDPrecInit(cvode_mem, local_N, mudq, mldq, mukeep, mlkeep, ZERO,
                         flocal, NULL);
  if (check_retval(&retval, "CVBBDPrecInit", 1, my_pe)) { MPI_Abort(comm, 1); }

  /* Print heading */
  if (my_pe == 0) { PrintIntro(npes, mudq, mldq, mukeep, mlkeep); }

  /* Loop over jpre (= SUN_PREC_LEFT, SUN_PREC_RIGHT), and solve the problem */
  for (jpre = SUN_PREC_LEFT; jpre <= SUN_PREC_RIGHT; jpre++)
  {
    /* On second run, re-initialize u, the integrator, CVBBDPRE, and SPGMR */

    if (jpre == SUN_PREC_RIGHT)
    {
      SetInitialProfiles(u, data);

      retval = CVodeReInit(cvode_mem, T0, u);
      if (check_retval(&retval, "CVodeReInit", 1, my_pe))
      {
        MPI_Abort(comm, 1);
      }

      retval = CVBBDPrecReInit(cvode_mem, mudq, mldq, ZERO);
      if (check_retval(&retval, "CVBBDPrecReInit", 1, my_pe))
      {
        MPI_Abort(comm, 1);
      }

      retval = SUNLinSol_SPGMRSetPrecType(LS, SUN_PREC_RIGHT);
      if (check_retval(&retval, "SUNLinSol_SPGMRSetPrecType", 1, my_pe))
      {
        MPI_Abort(comm, 1);
      }

      if (my_pe == 0)
      {
        printf("\n\n-------------------------------------------------------");
        printf("------------\n");
      }
    }

    if (my_pe == 0)
    {
      printf("\n\nPreconditioner type is:  jpre = %s\n\n",
             (jpre == SUN_PREC_LEFT) ? "SUN_PREC_LEFT" : "SUN_PREC_RIGHT");
    }

    /* In loop over output points, call CVode, print results, test for error */

    for (iout = 1, tout = TWOHR; iout <= NOUT; iout++, tout += TWOHR)
    {
      retval = CVode(cvode_mem, tout, u, &t, CV_NORMAL);
      if (check_retval(&retval, "CVode", 1, my_pe)) { break; }
      PrintOutput(cvode_mem, my_pe, comm, u, t);
    }

    /* Print final statistics */

    if (my_pe == 0) { PrintFinalStats(cvode_mem); }

  } /* End of jpre loop */

  /* Free memory */
  N_VDestroy(u);
  free(data);
  CVodeFree(&cvode_mem);
  SUNLinSolFree(LS);
  SUNContext_Free(&sunctx);

  MPI_Finalize();

  return (0);
}

/*********************** Private Helper Functions ************************/

/* Load constants in data */

static void InitUserData(int my_pe, sunindextype local_N, MPI_Comm comm,
                         UserData data)
{
  int isubx, isuby;

  /* Set problem constants */
  data->om   = PI / HALFDAY;
  data->dx   = (XMAX - XMIN) / ((sunrealtype)(MX - 1));
  data->dy   = (YMAX - YMIN) / ((sunrealtype)(MY - 1));
  data->hdco = KH / SQR(data->dx);
  data->haco = VEL / (SUN_RCONST(2.0) * data->dx);
  data->vdco = (SUN_RCONST(1.0) / SQR(data->dy)) * KV0;

  /* Set machine-related constants */
  data->comm   = comm;
  data->my_pe  = my_pe;
  data->Nlocal = local_N;
  /* isubx and isuby are the PE grid indices corresponding to my_pe */
  isuby       = my_pe / NPEX;
  isubx       = my_pe - isuby * NPEX;
  data->isubx = isubx;
  data->isuby = isuby;
  /* Set the sizes of a boundary x-line in u and uext */
  data->nvmxsub  = NVARS * MXSUB;
  data->nvmxsub2 = NVARS * (MXSUB + 2);
}

/* Set initial conditions in u */

static void SetInitialProfiles(N_Vector u, UserData data)
{
  int isubx, isuby;
  int lx, ly, jx, jy;
  sunindextype offset;
  sunrealtype dx, dy, x, y, cx, cy, xmid, ymid;
  sunrealtype* uarray;

  /* Set pointer to data array in vector u */

  uarray = N_VGetArrayPointer(u);

  /* Get mesh spacings, and subgrid indices for this PE */

  dx    = data->dx;
  dy    = data->dy;
  isubx = data->isubx;
  isuby = data->isuby;

  /* Load initial profiles of c1 and c2 into local u vector.
  Here lx and ly are local mesh point indices on the local subgrid,
  and jx and jy are the global mesh point indices. */

  offset = 0;
  xmid   = SUN_RCONST(0.5) * (XMIN + XMAX);
  ymid   = SUN_RCONST(0.5) * (YMIN + YMAX);
  for (ly = 0; ly < MYSUB; ly++)
  {
    jy = ly + isuby * MYSUB;
    y  = YMIN + jy * dy;
    cy = SQR(SUN_RCONST(0.1) * (y - ymid));
    cy = SUN_RCONST(1.0) - cy + SUN_RCONST(0.5) * SQR(cy);
    for (lx = 0; lx < MXSUB; lx++)
    {
      jx                 = lx + isubx * MXSUB;
      x                  = XMIN + jx * dx;
      cx                 = SQR(SUN_RCONST(0.1) * (x - xmid));
      cx                 = SUN_RCONST(1.0) - cx + SUN_RCONST(0.5) * SQR(cx);
      uarray[offset]     = C1_SCALE * cx * cy;
      uarray[offset + 1] = C2_SCALE * cx * cy;
      offset             = offset + 2;
    }
  }
}

/* Print problem introduction */

static void PrintIntro(int npes, sunindextype mudq, sunindextype mldq,
                       sunindextype mukeep, sunindextype mlkeep)
{
  printf("\n2-species diurnal advection-diffusion problem\n");
  printf("  %d by %d mesh on %d processors\n", MX, MY, npes);
  printf("  Using CVBBDPRE preconditioner module\n");
  printf("    Difference-quotient half-bandwidths are");
  printf(" mudq = %ld,  mldq = %ld\n", (long int)mudq, (long int)mldq);
  printf("    Retained band block half-bandwidths are");
  printf(" mukeep = %ld,  mlkeep = %ld", (long int)mukeep, (long int)mlkeep);

  return;
}

/* Print current t, step count, order, stepsize, and sampled c1,c2 values */

static void PrintOutput(void* cvode_mem, int my_pe, MPI_Comm comm, N_Vector u,
                        sunrealtype t)
{
  int qu, retval, npelast;
  long int nst;
  sunindextype i0, i1;
  sunrealtype hu, *uarray, tempu[2];
  MPI_Status status;

  npelast = NPEX * NPEY - 1;
  uarray  = N_VGetArrayPointer(u);

  /* Send c1,c2 at top right mesh point to PE 0 */
  if (my_pe == npelast)
  {
    i0 = NVARS * MXSUB * MYSUB - 2;
    i1 = i0 + 1;
    if (npelast != 0) { MPI_Send(&uarray[i0], 2, MPI_SUNREALTYPE, 0, 0, comm); }
    else
    {
      tempu[0] = uarray[i0];
      tempu[1] = uarray[i1];
    }
  }

  /* On PE 0, receive c1,c2 at top right, then print performance data
     and sampled solution values */
  if (my_pe == 0)
  {
    if (npelast != 0)
    {
      MPI_Recv(&tempu[0], 2, MPI_SUNREALTYPE, npelast, 0, comm, &status);
    }
    retval = CVodeGetNumSteps(cvode_mem, &nst);
    check_retval(&retval, "CVodeGetNumSteps", 1, my_pe);
    retval = CVodeGetLastOrder(cvode_mem, &qu);
    check_retval(&retval, "CVodeGetLastOrder", 1, my_pe);
    retval = CVodeGetLastStep(cvode_mem, &hu);
    check_retval(&retval, "CVodeGetLastStep", 1, my_pe);
#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf("t = %.2Le   no. steps = %ld   order = %d   stepsize = %.2Le\n", t,
           nst, qu, hu);
    printf("At bottom left:  c1, c2 = %12.3Le %12.3Le \n", uarray[0], uarray[1]);
    printf("At top right:    c1, c2 = %12.3Le %12.3Le \n\n", tempu[0], tempu[1]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("t = %.2e   no. steps = %ld   order = %d   stepsize = %.2e\n", t,
           nst, qu, hu);
    printf("At bottom left:  c1, c2 = %12.3e %12.3e \n", uarray[0], uarray[1]);
    printf("At top right:    c1, c2 = %12.3e %12.3e \n\n", tempu[0], tempu[1]);
#else
    printf("t = %.2e   no. steps = %ld   order = %d   stepsize = %.2e\n", t,
           nst, qu, hu);
    printf("At bottom left:  c1, c2 = %12.3e %12.3e \n", uarray[0], uarray[1]);
    printf("At top right:    c1, c2 = %12.3e %12.3e \n\n", tempu[0], tempu[1]);
#endif
  }
}

/* Print final statistics contained in iopt */

static void PrintFinalStats(void* cvode_mem)
{
  long int lenrw, leniw;
  long int lenrwLS, leniwLS;
  long int lenrwBBDP, leniwBBDP;
  long int nst, nfe, nsetups, nni, ncfn, netf, ngevalsBBDP;
  long int nli, npe, nps, ncfl, nfeLS;
  int retval;

  retval = CVodeGetWorkSpace(cvode_mem, &lenrw, &leniw);
  check_retval(&retval, "CVodeGetWorkSpace", 1, 0);
  retval = CVodeGetNumSteps(cvode_mem, &nst);
  check_retval(&retval, "CVodeGetNumSteps", 1, 0);
  retval = CVodeGetNumRhsEvals(cvode_mem, &nfe);
  check_retval(&retval, "CVodeGetNumRhsEvals", 1, 0);
  retval = CVodeGetNumLinSolvSetups(cvode_mem, &nsetups);
  check_retval(&retval, "CVodeGetNumLinSolvSetups", 1, 0);
  retval = CVodeGetNumErrTestFails(cvode_mem, &netf);
  check_retval(&retval, "CVodeGetNumErrTestFails", 1, 0);
  retval = CVodeGetNumNonlinSolvIters(cvode_mem, &nni);
  check_retval(&retval, "CVodeGetNumNonlinSolvIters", 1, 0);
  retval = CVodeGetNumNonlinSolvConvFails(cvode_mem, &ncfn);
  check_retval(&retval, "CVodeGetNumNonlinSolvConvFails", 1, 0);

  retval = CVodeGetLinWorkSpace(cvode_mem, &lenrwLS, &leniwLS);
  check_retval(&retval, "CVodeGetLinWorkSpace", 1, 0);
  retval = CVodeGetNumLinIters(cvode_mem, &nli);
  check_retval(&retval, "CVodeGetNumLinIters", 1, 0);
  retval = CVodeGetNumPrecEvals(cvode_mem, &npe);
  check_retval(&retval, "CVodeGetNumPrecEvals", 1, 0);
  retval = CVodeGetNumPrecSolves(cvode_mem, &nps);
  check_retval(&retval, "CVodeGetNumPrecSolves", 1, 0);
  retval = CVodeGetNumLinConvFails(cvode_mem, &ncfl);
  check_retval(&retval, "CVodeGetNumLinConvFails", 1, 0);
  retval = CVodeGetNumLinRhsEvals(cvode_mem, &nfeLS);
  check_retval(&retval, "CVodeGetNumLinRhsEvals", 1, 0);

  printf("\nFinal Statistics: \n\n");
  printf("lenrw   = %5ld     leniw   = %5ld\n", lenrw, leniw);
  printf("lenrwls = %5ld     leniwls = %5ld\n", lenrwLS, leniwLS);
  printf("nst     = %5ld\n", nst);
  printf("nfe     = %5ld     nfels   = %5ld\n", nfe, nfeLS);
  printf("nni     = %5ld     nli     = %5ld\n", nni, nli);
  printf("nsetups = %5ld     netf    = %5ld\n", nsetups, netf);
  printf("npe     = %5ld     nps     = %5ld\n", npe, nps);
  printf("ncfn    = %5ld     ncfl    = %5ld\n\n", ncfn, ncfl);

  retval = CVBBDPrecGetWorkSpace(cvode_mem, &lenrwBBDP, &leniwBBDP);
  check_retval(&retval, "CVBBDPrecGetWorkSpace", 1, 0);
  retval = CVBBDPrecGetNumGfnEvals(cvode_mem, &ngevalsBBDP);
  check_retval(&retval, "CVBBDPrecGetNumGfnEvals", 1, 0);
  printf("In CVBBDPRE: real/integer local work space sizes = %ld, %ld\n",
         lenrwBBDP, leniwBBDP);
  printf("             no. flocal evals. = %ld\n", ngevalsBBDP);
}

/* Routine to send boundary data to neighboring PEs */

static void BSend(MPI_Comm comm, int my_pe, int isubx, int isuby,
                  sunindextype dsizex, sunindextype dsizey, sunrealtype uarray[])
{
  int i, ly;
  sunindextype offsetu, offsetbuf;
  sunrealtype bufleft[NVARS * MYSUB], bufright[NVARS * MYSUB];

  /* If isuby > 0, send data from bottom x-line of u */

  if (isuby != 0)
  {
    MPI_Send(&uarray[0], (int)dsizex, MPI_SUNREALTYPE, my_pe - NPEX, 0, comm);
  }

  /* If isuby < NPEY-1, send data from top x-line of u */

  if (isuby != NPEY - 1)
  {
    offsetu = (MYSUB - 1) * dsizex;
    MPI_Send(&uarray[offsetu], (int)dsizex, MPI_SUNREALTYPE, my_pe + NPEX, 0,
             comm);
  }

  /* If isubx > 0, send data from left y-line of u (via bufleft) */

  if (isubx != 0)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetbuf = ly * NVARS;
      offsetu   = ly * dsizex;
      for (i = 0; i < NVARS; i++)
      {
        bufleft[offsetbuf + i] = uarray[offsetu + i];
      }
    }
    MPI_Send(&bufleft[0], (int)dsizey, MPI_SUNREALTYPE, my_pe - 1, 0, comm);
  }

  /* If isubx < NPEX-1, send data from right y-line of u (via bufright) */

  if (isubx != NPEX - 1)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetbuf = ly * NVARS;
      offsetu   = offsetbuf * MXSUB + (MXSUB - 1) * NVARS;
      for (i = 0; i < NVARS; i++)
      {
        bufright[offsetbuf + i] = uarray[offsetu + i];
      }
    }
    MPI_Send(&bufright[0], (int)dsizey, MPI_SUNREALTYPE, my_pe + 1, 0, comm);
  }
}

/* Routine to start receiving boundary data from neighboring PEs.
   Notes:
   1) buffer should be able to hold 2*NVARS*MYSUB sunrealtype entries, should be
   passed to both the BRecvPost and BRecvWait functions, and should not
   be manipulated between the two calls.
   2) request should have 4 entries, and should be passed in both calls also. */

static void BRecvPost(MPI_Comm comm, MPI_Request request[], int my_pe, int isubx,
                      int isuby, sunindextype dsizex, sunindextype dsizey,
                      sunrealtype uext[], sunrealtype buffer[])
{
  sunindextype offsetue;
  /* Have bufleft and bufright use the same buffer */
  sunrealtype *bufleft = buffer, *bufright = buffer + NVARS * MYSUB;

  /* If isuby > 0, receive data for bottom x-line of uext */
  if (isuby != 0)
  {
    MPI_Irecv(&uext[NVARS], (int)dsizex, MPI_SUNREALTYPE, my_pe - NPEX, 0, comm,
              &request[0]);
  }

  /* If isuby < NPEY-1, receive data for top x-line of uext */
  if (isuby != NPEY - 1)
  {
    offsetue = NVARS * (1 + (MYSUB + 1) * (MXSUB + 2));
    MPI_Irecv(&uext[offsetue], (int)dsizex, MPI_SUNREALTYPE, my_pe + NPEX, 0,
              comm, &request[1]);
  }

  /* If isubx > 0, receive data for left y-line of uext (via bufleft) */
  if (isubx != 0)
  {
    MPI_Irecv(&bufleft[0], (int)dsizey, MPI_SUNREALTYPE, my_pe - 1, 0, comm,
              &request[2]);
  }

  /* If isubx < NPEX-1, receive data for right y-line of uext (via bufright) */
  if (isubx != NPEX - 1)
  {
    MPI_Irecv(&bufright[0], (int)dsizey, MPI_SUNREALTYPE, my_pe + 1, 0, comm,
              &request[3]);
  }
}

/* Routine to finish receiving boundary data from neighboring PEs.
   Notes:
   1) buffer should be able to hold 2*NVARS*MYSUB sunrealtype entries, should be
   passed to both the BRecvPost and BRecvWait functions, and should not
   be manipulated between the two calls.
   2) request should have 4 entries, and should be passed in both calls also. */

static void BRecvWait(MPI_Request request[], int isubx, int isuby,
                      sunindextype dsizex, sunrealtype uext[],
                      sunrealtype buffer[])
{
  int i, ly;
  sunindextype dsizex2, offsetue, offsetbuf;
  sunrealtype *bufleft = buffer, *bufright = buffer + NVARS * MYSUB;
  MPI_Status status;

  dsizex2 = dsizex + 2 * NVARS;

  /* If isuby > 0, receive data for bottom x-line of uext */
  if (isuby != 0) { MPI_Wait(&request[0], &status); }

  /* If isuby < NPEY-1, receive data for top x-line of uext */
  if (isuby != NPEY - 1) { MPI_Wait(&request[1], &status); }

  /* If isubx > 0, receive data for left y-line of uext (via bufleft) */
  if (isubx != 0)
  {
    MPI_Wait(&request[2], &status);

    /* Copy the buffer to uext */
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetbuf = ly * NVARS;
      offsetue  = (ly + 1) * dsizex2;
      for (i = 0; i < NVARS; i++)
      {
        uext[offsetue + i] = bufleft[offsetbuf + i];
      }
    }
  }

  /* If isubx < NPEX-1, receive data for right y-line of uext (via bufright) */
  if (isubx != NPEX - 1)
  {
    MPI_Wait(&request[3], &status);

    /* Copy the buffer to uext */
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetbuf = ly * NVARS;
      offsetue  = (ly + 2) * dsizex2 - NVARS;
      for (i = 0; i < NVARS; i++)
      {
        uext[offsetue + i] = bufright[offsetbuf + i];
      }
    }
  }
}

/* fucomm routine.  This routine performs all inter-processor
   communication of data in u needed to calculate f.         */

static void fucomm(sunrealtype t, N_Vector u, void* user_data)
{
  UserData data;
  sunrealtype *uarray, *uext, buffer[2 * NVARS * MYSUB];
  MPI_Comm comm;
  int my_pe, isubx, isuby;
  sunindextype nvmxsub, nvmysub;
  MPI_Request request[4];

  data   = (UserData)user_data;
  uarray = N_VGetArrayPointer(u);

  /* Get comm, my_pe, subgrid indices, data sizes, extended array uext */

  comm    = data->comm;
  my_pe   = data->my_pe;
  isubx   = data->isubx;
  isuby   = data->isuby;
  nvmxsub = data->nvmxsub;
  nvmysub = NVARS * MYSUB;
  uext    = data->uext;

  /* Start receiving boundary data from neighboring PEs */

  BRecvPost(comm, request, my_pe, isubx, isuby, nvmxsub, nvmysub, uext, buffer);

  /* Send data from boundary of local grid to neighboring PEs */

  BSend(comm, my_pe, isubx, isuby, nvmxsub, nvmysub, uarray);

  /* Finish receiving boundary data from neighboring PEs */

  BRecvWait(request, isubx, isuby, nvmxsub, uext, buffer);
}

/***************** Function called by the solver **************************/

/* f routine.  Evaluate f(t,y).  First call fucomm to do communication of
   subgrid boundary data into uext.  Then calculate f by a call to flocal. */

static int f(sunrealtype t, N_Vector u, N_Vector udot, void* user_data)
{
  UserData data;

  data = (UserData)user_data;

  /* Call fucomm to do inter-processor communication */

  fucomm(t, u, user_data);

  /* Call flocal to calculate all right-hand sides */

  flocal(data->Nlocal, t, u, udot, user_data);

  return (0);
}

/***************** Functions called by the CVBBDPRE module ****************/

/* flocal routine.  Compute f(t,y).  This routine assumes that all
   inter-processor communication of data needed to calculate f has already
   been done, and this data is in the work array uext.                    */

static int flocal(sunindextype Nlocal, sunrealtype t, N_Vector u, N_Vector udot,
                  void* user_data)
{
  sunrealtype* uext;
  sunrealtype q3, c1, c2, c1dn, c2dn, c1up, c2up, c1lt, c2lt;
  sunrealtype c1rt, c2rt, cydn, cyup, hord1, hord2, horad1, horad2;
  sunrealtype qq1, qq2, qq3, qq4, rkin1, rkin2, s, vertd1, vertd2, ydn, yup;
  sunrealtype q4coef, dely, verdco, hordco, horaco;
  int i, lx, ly, jy;
  int isubx, isuby;
  sunindextype nvmxsub, nvmxsub2, offsetu, offsetue;
  UserData data;
  sunrealtype *uarray, *duarray;

  uarray  = N_VGetArrayPointer(u);
  duarray = N_VGetArrayPointer(udot);

  /* Get subgrid indices, array sizes, extended work array uext */

  data     = (UserData)user_data;
  isubx    = data->isubx;
  isuby    = data->isuby;
  nvmxsub  = data->nvmxsub;
  nvmxsub2 = data->nvmxsub2;
  uext     = data->uext;

  /* Copy local segment of u vector into the working extended array uext */

  offsetu  = 0;
  offsetue = nvmxsub2 + NVARS;
  for (ly = 0; ly < MYSUB; ly++)
  {
    for (i = 0; i < nvmxsub; i++) { uext[offsetue + i] = uarray[offsetu + i]; }
    offsetu  = offsetu + nvmxsub;
    offsetue = offsetue + nvmxsub2;
  }

  /* To facilitate homogeneous Neumann boundary conditions, when this is
  a boundary PE, copy data from the first interior mesh line of u to uext */

  /* If isuby = 0, copy x-line 2 of u to uext */
  if (isuby == 0)
  {
    for (i = 0; i < nvmxsub; i++) { uext[NVARS + i] = uarray[nvmxsub + i]; }
  }

  /* If isuby = NPEY-1, copy x-line MYSUB-1 of u to uext */
  if (isuby == NPEY - 1)
  {
    offsetu  = (MYSUB - 2) * nvmxsub;
    offsetue = (MYSUB + 1) * nvmxsub2 + NVARS;
    for (i = 0; i < nvmxsub; i++) { uext[offsetue + i] = uarray[offsetu + i]; }
  }

  /* If isubx = 0, copy y-line 2 of u to uext */
  if (isubx == 0)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetu  = ly * nvmxsub + NVARS;
      offsetue = (ly + 1) * nvmxsub2;
      for (i = 0; i < NVARS; i++) { uext[offsetue + i] = uarray[offsetu + i]; }
    }
  }

  /* If isubx = NPEX-1, copy y-line MXSUB-1 of u to uext */
  if (isubx == NPEX - 1)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetu  = (ly + 1) * nvmxsub - 2 * NVARS;
      offsetue = (ly + 2) * nvmxsub2 - NVARS;
      for (i = 0; i < NVARS; i++) { uext[offsetue + i] = uarray[offsetu + i]; }
    }
  }

  /* Make local copies of problem variables, for efficiency */

  dely   = data->dy;
  verdco = data->vdco;
  hordco = data->hdco;
  horaco = data->haco;

  /* Set diurnal rate coefficients as functions of t, and save q4 in
  data block for use by preconditioner evaluation routine            */

  s = sin((data->om) * t);
  if (s > ZERO)
  {
    q3     = exp(-A3 / s);
    q4coef = exp(-A4 / s);
  }
  else
  {
    q3     = ZERO;
    q4coef = ZERO;
  }
  data->q4 = q4coef;

  /* Loop over all grid points in local subgrid */

  for (ly = 0; ly < MYSUB; ly++)
  {
    jy = ly + isuby * MYSUB;

    /* Set vertical diffusion coefficients at jy +- 1/2 */

    ydn  = YMIN + (jy - SUN_RCONST(0.5)) * dely;
    yup  = ydn + dely;
    cydn = verdco * exp(SUN_RCONST(0.2) * ydn);
    cyup = verdco * exp(SUN_RCONST(0.2) * yup);
    for (lx = 0; lx < MXSUB; lx++)
    {
      /* Extract c1 and c2, and set kinetic rate terms */

      offsetue = (lx + 1) * NVARS + (ly + 1) * nvmxsub2;
      c1       = uext[offsetue];
      c2       = uext[offsetue + 1];
      qq1      = Q1 * c1 * C3;
      qq2      = Q2 * c1 * c2;
      qq3      = q3 * C3;
      qq4      = q4coef * c2;
      rkin1    = -qq1 - qq2 + 2.0 * qq3 + qq4;
      rkin2    = qq1 - qq2 - qq4;

      /* Set vertical diffusion terms */

      c1dn   = uext[offsetue - nvmxsub2];
      c2dn   = uext[offsetue - nvmxsub2 + 1];
      c1up   = uext[offsetue + nvmxsub2];
      c2up   = uext[offsetue + nvmxsub2 + 1];
      vertd1 = cyup * (c1up - c1) - cydn * (c1 - c1dn);
      vertd2 = cyup * (c2up - c2) - cydn * (c2 - c2dn);

      /* Set horizontal diffusion and advection terms */

      c1lt   = uext[offsetue - 2];
      c2lt   = uext[offsetue - 1];
      c1rt   = uext[offsetue + 2];
      c2rt   = uext[offsetue + 3];
      hord1  = hordco * (c1rt - SUN_RCONST(2.0) * c1 + c1lt);
      hord2  = hordco * (c2rt - SUN_RCONST(2.0) * c2 + c2lt);
      horad1 = horaco * (c1rt - c1lt);
      horad2 = horaco * (c2rt - c2lt);

      /* Load all terms into duarray */

      offsetu              = lx * NVARS + ly * nvmxsub;
      duarray[offsetu]     = vertd1 + hord1 + horad1 + rkin1;
      duarray[offsetu + 1] = vertd2 + hord2 + horad2 + rkin2;
    }
  }

  return (0);
}

/* Check function return value...
     opt == 0 means SUNDIALS function allocates memory so check if
              returned NULL pointer
     opt == 1 means SUNDIALS function returns an integer value so check if
              retval < 0
     opt == 2 means function allocates memory so check if returned
              NULL pointer */

static int check_retval(void* returnvalue, const char* funcname, int opt, int id)
{
  int* retval;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && returnvalue == NULL)
  {
    fprintf(stderr,
            "\nSUNDIALS_ERROR(%d): %s() failed - returned NULL pointer\n\n", id,
            funcname);
    return (1);
  }

  /* Check if retval < 0 */
  else if (opt == 1)
  {
    retval = (int*)returnvalue;
    if (*retval < 0)
    {
      fprintf(stderr, "\nSUNDIALS_ERROR(%d): %s() failed with retval = %d\n\n",
              id, funcname, *retval);
      return (1);
    }
  }

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && returnvalue == NULL)
  {
    fprintf(stderr, "\nMEMORY_ERROR(%d): %s() failed - returned NULL pointer\n\n",
            id, funcname);
    return (1);
  }

  return (0);
}
