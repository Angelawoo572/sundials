/*
 * -----------------------------------------------------------------
 * Programmer(s): Daniel R. Reynolds @ SMU
 *                S. D. Cohen, A. C. Hindmarsh, Radu Serban,
 *                and M. R. Wittman @ LLNL
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
 * The problem is solved by CVODES on NPE processors, treated
 * as a rectangular process grid of size NPEX by NPEY, with
 * NPE = NPEX*NPEY. Each processor contains a subgrid of size
 * MXSUB by MYSUB of the (x,y) mesh. Thus the actual mesh sizes
 * are MX = MXSUB*NPEX and MY = MYSUB*NPEY, and the ODE system size
 * is neq = 2*MX*MY.
 *
 * The solution with CVODES is done with the BDF/GMRES method (i.e.
 * using the SUNLinSol_SPGMR linear solver) and the block-diagonal part of
 * the Newton matrix as a left preconditioner. A copy of the
 * block-diagonal part of the Jacobian is saved and conditionally
 * reused within the Precond routine.
 *
 * Performance data and sampled solution values are printed at
 * selected output times, and all performance counters are printed
 * on completion.
 *
 * Optionally, CVODES can compute sensitivities with respect to the
 * problem parameters q1 and q2.
 * Any of three sensitivity methods (SIMULTANEOUS, STAGGERED, and
 * STAGGERED1) can be used and sensitivities may be included in the
 * error test or not (error control set on FULL or PARTIAL,
 * respectively).
 *
 * Execution:
 *
 * Note: This version uses MPI for user routines, and the CVODES
 *       solver. In what follows, N is the number of processors,
 *       N = NPEX*NPEY (see constants below) and it is assumed that
 *       the MPI script mpirun is used to run a parallel
 *       application.
 * If no sensitivities are desired:
 *    % mpirun -np N cvsDiurnal_FSA_kry_p -nosensi
 * If sensitivities are to be computed:
 *    % mpirun -np N cvsDiurnal_FSA_kry_p -sensi sensi_meth err_con
 * where sensi_meth is one of {sim, stg, stg1} and err_con is one of
 * {t, f}.
 * -----------------------------------------------------------------
 */

#include <cvodes/cvodes.h> /* main CVODES header file */
#include <math.h>
#include <mpi.h>
#include <nvector/nvector_parallel.h> /* defs of par. NVECTOR fcts. and macros */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sundials/sundials_dense.h> /* generic DENSE solver used in prec. */
#include <sundials/sundials_types.h> /* def. of sunrealtype */
#include <sunlinsol/sunlinsol_spgmr.h> /* defs. for SUNLinSol_SPGMR fcts. and constants */

/* helpful macros */

#ifndef SQR
#define SQR(A) ((A) * (A))
#endif

/* Problem Constants */

#define NVARS    2                 /* number of species                    */
#define C1_SCALE SUN_RCONST(1.0e6) /* coefficients in initial profiles     */
#define C2_SCALE SUN_RCONST(1.0e12)

#define T0      SUN_RCONST(0.0)    /* initial time                         */
#define NOUT    12                 /* number of output times               */
#define TWOHR   SUN_RCONST(7200.0) /* number of seconds in two hours       */
#define HALFDAY SUN_RCONST(4.32e4) /* number of seconds in a half day      */
#define PI      SUN_RCONST(3.1415926535898) /* pi                        */

#define XMIN SUN_RCONST(0.0) /* grid boundaries in x                 */
#define XMAX SUN_RCONST(20.0)
#define YMIN SUN_RCONST(30.0) /* grid boundaries in y                 */
#define YMAX SUN_RCONST(50.0)

#define NPEX 2  /* no. PEs in x direction of PE array   */
#define NPEY 2  /* no. PEs in y direction of PE array   */
                /* Total no. PEs = NPEX*NPEY            */
#define MXSUB 5 /* no. x points per subgrid             */
#define MYSUB 5 /* no. y points per subgrid             */

#define MX (NPEX * MXSUB) /* MX = number of x mesh points         */
#define MY (NPEY * MYSUB) /* MY = number of y mesh points         */
                          /* Spatial mesh is MX by MY             */

/* CVodeInit Constants */

#define RTOL  SUN_RCONST(1.0e-5) /* scalar relative tolerance             */
#define FLOOR SUN_RCONST(100.0)  /* value of C1 or C2 at which tols.      */
                                 /* change from relative to absolute      */
#define ATOL (RTOL * FLOOR)      /* scalar absolute tolerance             */

/* Sensitivity constants */
#define NP 8 /* number of problem parameters          */
#define NS 2 /* number of sensitivities               */

#define ZERO SUN_RCONST(0.0)

/* User-defined matrix accessor macro: IJth */

/* IJth is defined in order to write code which indexes into small dense
   matrices with a (row,column) pair, where 1 <= row,column <= NVARS.

   IJth(a,i,j) references the (i,j)th entry of the small matrix sunrealtype **a,
   where 1 <= i,j <= NVARS. The small matrix routines in sundials_dense.h
   work with matrices stored by column in a 2-dimensional array. In C,
   arrays are indexed starting at 0, not 1. */

#define IJth(a, i, j) (a[j - 1][i - 1])

/* Types : UserData and PreconData
   contain problem parameters, problem constants, preconditioner blocks,
   pivot arrays, grid constants, and processor indices, as
   well as data needed for preconditioning */

typedef struct
{
  sunrealtype* p;
  sunrealtype q4, om, dx, dy, hdco, haco, vdco;
  sunrealtype uext[NVARS * (MXSUB + 2) * (MYSUB + 2)];
  int my_pe, isubx, isuby;
  sunindextype nvmxsub, nvmxsub2;
  MPI_Comm comm;

  /* For preconditioner */
  sunrealtype **P[MXSUB][MYSUB], **Jbd[MXSUB][MYSUB];
  sunindextype* pivot[MXSUB][MYSUB];

}* UserData;

/* Functions Called by the CVODES Solver */

static int f(sunrealtype t, N_Vector u, N_Vector udot, void* user_data);

static int Precond(sunrealtype tn, N_Vector u, N_Vector fu, sunbooleantype jok,
                   sunbooleantype* jcurPtr, sunrealtype gamma, void* user_data);

static int PSolve(sunrealtype tn, N_Vector u, N_Vector fu, N_Vector r, N_Vector z,
                  sunrealtype gamma, sunrealtype delta, int lr, void* user_data);

/* Private Helper Functions */

static void ProcessArgs(int argc, char* argv[], int my_pe, sunbooleantype* sensi,
                        int* sensi_meth, sunbooleantype* err_con);
static void WrongArgs(int my_pe, char* name);

static void InitUserData(int my_pe, MPI_Comm comm, UserData data);
static void FreeUserData(UserData data);
static void SetInitialProfiles(N_Vector u, UserData data);

static void BSend(MPI_Comm comm, int my_pe, int isubx, int isuby,
                  sunindextype dsizex, sunindextype dsizey, sunrealtype udata[]);
static void BRecvPost(MPI_Comm comm, MPI_Request request[], int my_pe, int isubx,
                      int isuby, sunindextype dsizex, sunindextype dsizey,
                      sunrealtype uext[], sunrealtype buffer[]);
static void BRecvWait(MPI_Request request[], int isubx, int isuby,
                      sunindextype dsizex, sunrealtype uext[],
                      sunrealtype buffer[]);
static void ucomm(sunrealtype t, N_Vector u, UserData data);
static void fcalc(sunrealtype t, sunrealtype udata[], sunrealtype dudata[],
                  UserData data);

static void PrintOutput(void* cvode_mem, int my_pe, MPI_Comm comm,
                        sunrealtype t, N_Vector u);
static void PrintOutputS(int my_pe, MPI_Comm comm, N_Vector* uS);
static void PrintFinalStats(void* cvode_mem, sunbooleantype sensi,
                            sunbooleantype err_con, int sensi_meth);
static int check_retval(void* returnvalue, const char* funcname, int opt, int id);

/*
 *--------------------------------------------------------------------
 * MAIN PROGRAM
 *--------------------------------------------------------------------
 */

int main(int argc, char* argv[])
{
  SUNContext sunctx;
  sunrealtype abstol, reltol, t, tout;
  N_Vector u;
  UserData data;
  SUNLinearSolver LS;
  void* cvode_mem;
  int iout, retval, my_pe, npes;
  sunindextype neq, local_N;
  MPI_Comm comm;

  sunrealtype* pbar;
  int is, *plist;
  N_Vector* uS;
  sunbooleantype sensi, err_con;
  int sensi_meth;

  u         = NULL;
  data      = NULL;
  LS        = NULL;
  cvode_mem = NULL;
  pbar      = NULL;
  plist     = NULL;
  uS        = NULL;

  /* Set problem size neq */
  neq = NVARS * MX * MY;

  /* Get processor number and total number of pe's */
  MPI_Init(&argc, &argv);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &npes);
  MPI_Comm_rank(comm, &my_pe);

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

  /* Create the SUNDIALS simulation context that all SUNDIALS objects require */
  retval = SUNContext_Create(comm, &sunctx);
  if (check_retval(&retval, "SUNContext_Create", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  /* Process arguments */
  ProcessArgs(argc, argv, my_pe, &sensi, &sensi_meth, &err_con);

  /* Set local length */
  local_N = NVARS * MXSUB * MYSUB;

  /* Allocate and load user data block; allocate preconditioner block */
  data = (UserData)malloc(sizeof *data);
  if (check_retval((void*)data, "malloc", 2, my_pe)) { MPI_Abort(comm, 1); }
  data->p = NULL;
  data->p = (sunrealtype*)malloc(NP * sizeof(sunrealtype));
  if (check_retval((void*)data->p, "malloc", 2, my_pe)) { MPI_Abort(comm, 1); }
  InitUserData(my_pe, comm, data);

  /* Allocate u, and set initial values and tolerances */
  u = N_VNew_Parallel(comm, local_N, neq, sunctx);
  if (check_retval((void*)u, "N_VNew_Parallel", 0, my_pe))
  {
    MPI_Abort(comm, 1);
  }
  SetInitialProfiles(u, data);
  abstol = ATOL;
  reltol = RTOL;

  /* Create CVODES object, set optional input, allocate memory */
  cvode_mem = CVodeCreate(CV_BDF, sunctx);
  if (check_retval((void*)cvode_mem, "CVodeCreate", 0, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  retval = CVodeSetUserData(cvode_mem, data);
  if (check_retval(&retval, "CVodeSetUserData", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  retval = CVodeSetMaxNumSteps(cvode_mem, 2000);
  if (check_retval(&retval, "CVodeSetMaxNumSteps", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  retval = CVodeInit(cvode_mem, f, T0, u);
  if (check_retval(&retval, "CVodeInit", 1, my_pe)) { MPI_Abort(comm, 1); }

  retval = CVodeSStolerances(cvode_mem, reltol, abstol);
  if (check_retval(&retval, "CVodeSStolerances", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  /* Create SPGMR solver structure -- use left preconditioning
     and the default Krylov dimension maxl */
  LS = SUNLinSol_SPGMR(u, SUN_PREC_LEFT, 0, sunctx);
  if (check_retval((void*)LS, "SUNLinSol_SPGMR", 0, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  /* Attach linear solver */
  retval = CVodeSetLinearSolver(cvode_mem, LS, NULL);
  if (check_retval(&retval, "CVodeSetLinearSolver", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  retval = CVodeSetPreconditioner(cvode_mem, Precond, PSolve);
  if (check_retval(&retval, "CVodeSetPreconditioner", 1, my_pe))
  {
    MPI_Abort(comm, 1);
  }

  if (my_pe == 0)
  {
    printf("\n2-species diurnal advection-diffusion problem\n");
  }

  /* Sensitivity-related settings */
  if (sensi)
  {
    plist = (int*)malloc(NS * sizeof(int));
    if (check_retval((void*)plist, "malloc", 2, my_pe)) { MPI_Abort(comm, 1); }
    for (is = 0; is < NS; is++) { plist[is] = is; }

    pbar = (sunrealtype*)malloc(NS * sizeof(sunrealtype));
    if (check_retval((void*)pbar, "malloc", 2, my_pe)) { MPI_Abort(comm, 1); }
    for (is = 0; is < NS; is++) { pbar[is] = data->p[plist[is]]; }

    uS = N_VCloneVectorArray(NS, u);
    if (check_retval((void*)uS, "N_VCloneVectorArray", 0, my_pe))
    {
      MPI_Abort(comm, 1);
    }
    for (is = 0; is < NS; is++) { N_VConst(ZERO, uS[is]); }

    retval = CVodeSensInit1(cvode_mem, NS, sensi_meth, NULL, uS);
    if (check_retval(&retval, "CVodeSensInit1", 1, my_pe))
    {
      MPI_Abort(comm, 1);
    }

    retval = CVodeSensEEtolerances(cvode_mem);
    if (check_retval(&retval, "CVodeSensEEtolerances", 1, my_pe))
    {
      MPI_Abort(comm, 1);
    }

    retval = CVodeSetSensErrCon(cvode_mem, err_con);
    if (check_retval(&retval, "CVodeSetSensErrCon", 1, my_pe))
    {
      MPI_Abort(comm, 1);
    }

    retval = CVodeSetSensDQMethod(cvode_mem, CV_CENTERED, ZERO);
    if (check_retval(&retval, "CVodeSetSensDQMethod", 1, my_pe))
    {
      MPI_Abort(comm, 1);
    }

    retval = CVodeSetSensParams(cvode_mem, data->p, pbar, plist);
    if (check_retval(&retval, "CVodeSetSensParams", 1, my_pe))
    {
      MPI_Abort(comm, 1);
    }

    if (my_pe == 0)
    {
      printf("Sensitivity: YES ");
      if (sensi_meth == CV_SIMULTANEOUS) { printf("( SIMULTANEOUS +"); }
      else if (sensi_meth == CV_STAGGERED) { printf("( STAGGERED +"); }
      else { printf("( STAGGERED1 +"); }
      if (err_con) { printf(" FULL ERROR CONTROL )"); }
      else { printf(" PARTIAL ERROR CONTROL )"); }
    }
  }
  else
  {
    if (my_pe == 0) { printf("Sensitivity: NO "); }
  }

  if (my_pe == 0)
  {
    printf("\n\n");
    printf("==================================================================="
           "=====\n");
    printf("     T     Q       H      NST                    Bottom left  Top "
           "right \n");
    printf("==================================================================="
           "=====\n");
  }

  /* In loop over output points, call CVode, print results, test for error */
  for (iout = 1, tout = TWOHR; iout <= NOUT; iout++, tout += TWOHR)
  {
    retval = CVode(cvode_mem, tout, u, &t, CV_NORMAL);
    if (check_retval(&retval, "CVode", 1, my_pe)) { break; }
    PrintOutput(cvode_mem, my_pe, comm, t, u);
    if (sensi)
    {
      retval = CVodeGetSens(cvode_mem, &t, uS);
      if (check_retval(&retval, "CVodeGetSens", 1, my_pe)) { break; }
      PrintOutputS(my_pe, comm, uS);
    }
    if (my_pe == 0)
    {
      printf("-----------------------------------------------------------------"
             "-------\n");
    }
  }

  /* Print final statistics */
  if (my_pe == 0) { PrintFinalStats(cvode_mem, sensi, err_con, sensi_meth); }

  /* Free memory */
  N_VDestroy(u);
  if (sensi)
  {
    N_VDestroyVectorArray(uS, NS);
    free(plist);
    free(pbar);
  }
  FreeUserData(data);
  CVodeFree(&cvode_mem);
  SUNLinSolFree(LS);
  SUNContext_Free(&sunctx);

  MPI_Finalize();

  return (0);
}

/*
 *--------------------------------------------------------------------
 * FUNCTIONS CALLED BY CVODES
 *--------------------------------------------------------------------
 */

/*
 * f routine.  Evaluate f(t,y).  First call ucomm to do communication of
 * subgrid boundary data into uext.  Then calculate f by a call to fcalc.
 */

static int f(sunrealtype t, N_Vector u, N_Vector udot, void* user_data)
{
  sunrealtype *udata, *dudata;
  UserData data;

  udata  = N_VGetArrayPointer(u);
  dudata = N_VGetArrayPointer(udot);
  data   = (UserData)user_data;

  /* Call ucomm to do inter-processor communicaiton */
  ucomm(t, u, data);

  /* Call fcalc to calculate all right-hand sides */
  fcalc(t, udata, dudata, data);

  return (0);
}

/*
 * Preconditioner setup routine. Generate and preprocess P.
 */

static int Precond(sunrealtype tn, N_Vector u, N_Vector fu, sunbooleantype jok,
                   sunbooleantype* jcurPtr, sunrealtype gamma, void* user_data)
{
  sunrealtype c1, c2, cydn, cyup, diag, ydn, yup, q4coef, dely, verdco, hordco;
  sunrealtype**(*P)[MYSUB], **(*Jbd)[MYSUB];
  sunindextype*(*pivot)[MYSUB], retval, nvmxsub, offset;
  int lx, ly, jy, isuby;
  sunrealtype *udata, **a, **j;
  UserData data;
  sunrealtype Q1, Q2, C3;

  /* Make local copies of pointers in user_data, pointer to u's data,
     and PE index pair */
  data    = (UserData)user_data;
  P       = data->P;
  Jbd     = data->Jbd;
  pivot   = data->pivot;
  udata   = N_VGetArrayPointer(u);
  isuby   = data->isuby;
  nvmxsub = data->nvmxsub;

  /* Load problem coefficients and parameters */
  Q1 = data->p[0];
  Q2 = data->p[1];
  C3 = data->p[2];

  if (jok)
  { /* jok = SUNTRUE: Copy Jbd to P */

    for (ly = 0; ly < MYSUB; ly++)
    {
      for (lx = 0; lx < MXSUB; lx++)
      {
        SUNDlsMat_denseCopy(Jbd[lx][ly], P[lx][ly], NVARS, NVARS);
      }
    }
    *jcurPtr = SUNFALSE;
  }
  else
  { /* jok = SUNFALSE: Generate Jbd from scratch and copy to P */

    /* Make local copies of problem variables, for efficiency */
    q4coef = data->q4;
    dely   = data->dy;
    verdco = data->vdco;
    hordco = data->hdco;

    /* Compute 2x2 diagonal Jacobian blocks (using q4 values
       computed on the last f call).  Load into P. */
    for (ly = 0; ly < MYSUB; ly++)
    {
      jy   = ly + isuby * MYSUB;
      ydn  = YMIN + (jy - SUN_RCONST(0.5)) * dely;
      yup  = ydn + dely;
      cydn = verdco * exp(SUN_RCONST(0.2) * ydn);
      cyup = verdco * exp(SUN_RCONST(0.2) * yup);
      diag = -(cydn + cyup + SUN_RCONST(2.0) * hordco);
      for (lx = 0; lx < MXSUB; lx++)
      {
        offset        = lx * NVARS + ly * nvmxsub;
        c1            = udata[offset];
        c2            = udata[offset + 1];
        j             = Jbd[lx][ly];
        a             = P[lx][ly];
        IJth(j, 1, 1) = (-Q1 * C3 - Q2 * c2) + diag;
        IJth(j, 1, 2) = -Q2 * c1 + q4coef;
        IJth(j, 2, 1) = Q1 * C3 - Q2 * c2;
        IJth(j, 2, 2) = (-Q2 * c1 - q4coef) + diag;
        SUNDlsMat_denseCopy(j, a, NVARS, NVARS);
      }
    }

    *jcurPtr = SUNTRUE;
  }

  /* Scale by -gamma */
  for (ly = 0; ly < MYSUB; ly++)
  {
    for (lx = 0; lx < MXSUB; lx++)
    {
      SUNDlsMat_denseScale(-gamma, P[lx][ly], NVARS, NVARS);
    }
  }

  /* Add identity matrix and do LU decompositions on blocks in place */
  for (lx = 0; lx < MXSUB; lx++)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      SUNDlsMat_denseAddIdentity(P[lx][ly], NVARS);
      retval = SUNDlsMat_denseGETRF(P[lx][ly], NVARS, NVARS, pivot[lx][ly]);
      if (retval != 0) { return (1); }
    }
  }

  return (0);
}

/*
 * Preconditioner solve routine
 */

static int PSolve(sunrealtype tn, N_Vector u, N_Vector fu, N_Vector r, N_Vector z,
                  sunrealtype gamma, sunrealtype delta, int lr, void* user_data)
{
  sunrealtype**(*P)[MYSUB];
  sunindextype*(*pivot)[MYSUB], nvmxsub;
  int lx, ly;
  sunrealtype *zdata, *v;
  UserData data;

  /* Extract the P and pivot arrays from user_data */
  data  = (UserData)user_data;
  P     = data->P;
  pivot = data->pivot;

  /* Solve the block-diagonal system Px = r using LU factors stored
     in P and pivot data in pivot, and return the solution in z.
     First copy vector r to z. */
  N_VScale(SUN_RCONST(1.0), r, z);

  nvmxsub = data->nvmxsub;
  zdata   = N_VGetArrayPointer(z);

  for (lx = 0; lx < MXSUB; lx++)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      v = &(zdata[lx * NVARS + ly * nvmxsub]);
      SUNDlsMat_denseGETRS(P[lx][ly], NVARS, pivot[lx][ly], v);
    }
  }

  return (0);
}

/*
 *--------------------------------------------------------------------
 * PRIVATE FUNCTIONS
 *--------------------------------------------------------------------
 */

/*
 * Process and verify arguments to cvsfwdkryx_p.
 */

static void ProcessArgs(int argc, char* argv[], int my_pe, sunbooleantype* sensi,
                        int* sensi_meth, sunbooleantype* err_con)
{
  *sensi      = SUNFALSE;
  *sensi_meth = -1;
  *err_con    = SUNFALSE;

  if (argc < 2) { WrongArgs(my_pe, argv[0]); }

  if (strcmp(argv[1], "-nosensi") == 0) { *sensi = SUNFALSE; }
  else if (strcmp(argv[1], "-sensi") == 0) { *sensi = SUNTRUE; }
  else { WrongArgs(my_pe, argv[0]); }

  if (*sensi)
  {
    if (argc != 4) { WrongArgs(my_pe, argv[0]); }

    if (strcmp(argv[2], "sim") == 0) { *sensi_meth = CV_SIMULTANEOUS; }
    else if (strcmp(argv[2], "stg") == 0) { *sensi_meth = CV_STAGGERED; }
    else if (strcmp(argv[2], "stg1") == 0) { *sensi_meth = CV_STAGGERED1; }
    else { WrongArgs(my_pe, argv[0]); }

    if (strcmp(argv[3], "t") == 0) { *err_con = SUNTRUE; }
    else if (strcmp(argv[3], "f") == 0) { *err_con = SUNFALSE; }
    else { WrongArgs(my_pe, argv[0]); }
  }
}

static void WrongArgs(int my_pe, char* name)
{
  if (my_pe == 0)
  {
    printf("\nUsage: %s [-nosensi] [-sensi sensi_meth err_con]\n", name);
    printf("         sensi_meth = sim, stg, or stg1\n");
    printf("         err_con    = t or f\n");
  }
  MPI_Finalize();
  exit(0);
}

/*
 * Set user data.
 */

static void InitUserData(int my_pe, MPI_Comm comm, UserData data)
{
  int isubx, isuby;
  int lx, ly;
  sunrealtype KH, VEL, KV0;

  /* Set problem parameters */
  data->p[0] = SUN_RCONST(1.63e-16);    /* Q1  coeffs. q1, q2, c3             */
  data->p[1] = SUN_RCONST(4.66e-16);    /* Q2                                 */
  data->p[2] = SUN_RCONST(3.7e16);      /* C3                                 */
  data->p[3] = SUN_RCONST(22.62);       /* A3  coeff. in expression for q3(t) */
  data->p[4] = SUN_RCONST(7.601);       /* A4  coeff. in expression for q4(t) */
  KH = data->p[5] = SUN_RCONST(4.0e-6); /* KH  horizontal diffusivity Kh      */
  VEL = data->p[6] = SUN_RCONST(0.001); /* VEL advection velocity V           */
  KV0 = data->p[7] = SUN_RCONST(1.0e-8); /* KV0 coeff. in Kv(z)                */

  /* Set problem constants */
  data->om   = PI / HALFDAY;
  data->dx   = (XMAX - XMIN) / ((sunrealtype)(MX - 1));
  data->dy   = (YMAX - YMIN) / ((sunrealtype)(MY - 1));
  data->hdco = KH / SQR(data->dx);
  data->haco = VEL / (SUN_RCONST(2.0) * data->dx);
  data->vdco = (SUN_RCONST(1.0) / SQR(data->dy)) * KV0;

  /* Set machine-related constants */
  data->comm  = comm;
  data->my_pe = my_pe;

  /* isubx and isuby are the PE grid indices corresponding to my_pe */
  isuby       = my_pe / NPEX;
  isubx       = my_pe - isuby * NPEX;
  data->isubx = isubx;
  data->isuby = isuby;

  /* Set the sizes of a boundary x-line in u and uext */
  data->nvmxsub  = NVARS * MXSUB;
  data->nvmxsub2 = NVARS * (MXSUB + 2);

  /* Preconditioner-related fields */
  for (lx = 0; lx < MXSUB; lx++)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      (data->P)[lx][ly]     = SUNDlsMat_newDenseMat(NVARS, NVARS);
      (data->Jbd)[lx][ly]   = SUNDlsMat_newDenseMat(NVARS, NVARS);
      (data->pivot)[lx][ly] = SUNDlsMat_newIndexArray(NVARS);
    }
  }
}

/*
 * Free user data memory.
 */

static void FreeUserData(UserData data)
{
  int lx, ly;

  for (lx = 0; lx < MXSUB; lx++)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      SUNDlsMat_destroyMat((data->P)[lx][ly]);
      SUNDlsMat_destroyMat((data->Jbd)[lx][ly]);
      SUNDlsMat_destroyArray((data->pivot)[lx][ly]);
    }
  }

  free(data->p);

  free(data);
}

/*
 * Set initial conditions in u.
 */

static void SetInitialProfiles(N_Vector u, UserData data)
{
  int isubx, isuby;
  sunindextype lx, ly, jx, jy, offset;
  sunrealtype dx, dy, x, y, cx, cy, xmid, ymid;
  sunrealtype* udata;

  /* Set pointer to data array in vector u */
  udata = N_VGetArrayPointer(u);

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
      jx                = lx + isubx * MXSUB;
      x                 = XMIN + jx * dx;
      cx                = SQR(SUN_RCONST(0.1) * (x - xmid));
      cx                = SUN_RCONST(1.0) - cx + SUN_RCONST(0.5) * SQR(cx);
      udata[offset]     = C1_SCALE * cx * cy;
      udata[offset + 1] = C2_SCALE * cx * cy;
      offset            = offset + 2;
    }
  }
}

/*
 * Routine to send boundary data to neighboring PEs.
 */

static void BSend(MPI_Comm comm, int my_pe, int isubx, int isuby,
                  sunindextype dsizex, sunindextype dsizey, sunrealtype udata[])
{
  int i, ly;
  sunindextype offsetu, offsetbuf;
  sunrealtype bufleft[NVARS * MYSUB], bufright[NVARS * MYSUB];

  /* If isuby > 0, send data from bottom x-line of u */
  if (isuby != 0)
  {
    MPI_Send(&udata[0], (int)dsizex, MPI_SUNREALTYPE, my_pe - NPEX, 0, comm);
  }

  /* If isuby < NPEY-1, send data from top x-line of u */
  if (isuby != NPEY - 1)
  {
    offsetu = (MYSUB - 1) * dsizex;
    MPI_Send(&udata[offsetu], (int)dsizex, MPI_SUNREALTYPE, my_pe + NPEX, 0,
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
        bufleft[offsetbuf + i] = udata[offsetu + i];
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
        bufright[offsetbuf + i] = udata[offsetu + i];
      }
    }
    MPI_Send(&bufright[0], (int)dsizey, MPI_SUNREALTYPE, my_pe + 1, 0, comm);
  }
}

/*
 * Routine to start receiving boundary data from neighboring PEs.
 * Notes:
 *  1) buffer should be able to hold 2*NVARS*MYSUB sunrealtype entries, should be
 *     passed to both the BRecvPost and BRecvWait functions, and should not
 *     be manipulated between the two calls.
 *  2) request should have 4 entries, and should be passed in both calls also.
 */

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

/*
 * Routine to finish receiving boundary data from neighboring PEs.
 * Notes:
 *  1) buffer should be able to hold 2*NVARS*MYSUB sunrealtype entries, should be
 *     passed to both the BRecvPost and BRecvWait functions, and should not
 *     be manipulated between the two calls.
 *  2) request should have 4 entries, and should be passed in both calls also.
 */

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

/*
 * ucomm routine.  This routine performs all communication
 * between processors of data needed to calculate f.
 */

static void ucomm(sunrealtype t, N_Vector u, UserData data)
{
  sunrealtype *udata, *uext, buffer[2 * NVARS * MYSUB];
  MPI_Comm comm;
  int my_pe, isubx, isuby;
  sunindextype nvmxsub, nvmysub;
  MPI_Request request[4];

  udata = N_VGetArrayPointer(u);

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
  BSend(comm, my_pe, isubx, isuby, nvmxsub, nvmysub, udata);

  /* Finish receiving boundary data from neighboring PEs */
  BRecvWait(request, isubx, isuby, nvmxsub, uext, buffer);
}

/*
 * fcalc routine. Compute f(t,y).  This routine assumes that communication
 * between processors of data needed to calculate f has already been done,
 * and this data is in the work array uext.
 */

static void fcalc(sunrealtype t, sunrealtype udata[], sunrealtype dudata[],
                  UserData data)
{
  sunrealtype* uext;
  sunrealtype q3, c1, c2, c1dn, c2dn, c1up, c2up, c1lt, c2lt;
  sunrealtype c1rt, c2rt, cydn, cyup, hord1, hord2, horad1, horad2;
  sunrealtype qq1, qq2, qq3, qq4, rkin1, rkin2, s, vertd1, vertd2, ydn, yup;
  sunrealtype q4coef, dely, verdco, hordco, horaco;
  int i, lx, ly, jy, isubx, isuby;
  sunindextype nvmxsub, nvmxsub2, offsetu, offsetue;
  sunrealtype Q1, Q2, C3, A3, A4;

  /* Get subgrid indices, data sizes, extended work array uext */
  isubx    = data->isubx;
  isuby    = data->isuby;
  nvmxsub  = data->nvmxsub;
  nvmxsub2 = data->nvmxsub2;
  uext     = data->uext;

  /* Load problem coefficients and parameters */
  Q1 = data->p[0];
  Q2 = data->p[1];
  C3 = data->p[2];
  A3 = data->p[3];
  A4 = data->p[4];

  /* Copy local segment of u vector into the working extended array uext */
  offsetu  = 0;
  offsetue = nvmxsub2 + NVARS;
  for (ly = 0; ly < MYSUB; ly++)
  {
    for (i = 0; i < nvmxsub; i++) { uext[offsetue + i] = udata[offsetu + i]; }
    offsetu  = offsetu + nvmxsub;
    offsetue = offsetue + nvmxsub2;
  }

  /* To facilitate homogeneous Neumann boundary conditions, when this is
  a boundary PE, copy data from the first interior mesh line of u to uext */

  /* If isuby = 0, copy x-line 2 of u to uext */
  if (isuby == 0)
  {
    for (i = 0; i < nvmxsub; i++) { uext[NVARS + i] = udata[nvmxsub + i]; }
  }

  /* If isuby = NPEY-1, copy x-line MYSUB-1 of u to uext */
  if (isuby == NPEY - 1)
  {
    offsetu  = (MYSUB - 2) * nvmxsub;
    offsetue = (MYSUB + 1) * nvmxsub2 + NVARS;
    for (i = 0; i < nvmxsub; i++) { uext[offsetue + i] = udata[offsetu + i]; }
  }

  /* If isubx = 0, copy y-line 2 of u to uext */
  if (isubx == 0)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetu  = ly * nvmxsub + NVARS;
      offsetue = (ly + 1) * nvmxsub2;
      for (i = 0; i < NVARS; i++) { uext[offsetue + i] = udata[offsetu + i]; }
    }
  }

  /* If isubx = NPEX-1, copy y-line MXSUB-1 of u to uext */
  if (isubx == NPEX - 1)
  {
    for (ly = 0; ly < MYSUB; ly++)
    {
      offsetu  = (ly + 1) * nvmxsub - 2 * NVARS;
      offsetue = (ly + 2) * nvmxsub2 - NVARS;
      for (i = 0; i < NVARS; i++) { uext[offsetue + i] = udata[offsetu + i]; }
    }
  }

  /* Make local copies of problem variables, for efficiency */
  dely   = data->dy;
  verdco = data->vdco;
  hordco = data->hdco;
  horaco = data->haco;

  /* Set diurnal rate coefficients as functions of t, and save q4 in
  data block for use by preconditioner evaluation routine */
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
    ydn  = YMIN + (jy - .5) * dely;
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
      rkin1    = -qq1 - qq2 + SUN_RCONST(2.0) * qq3 + qq4;
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
      hord1  = hordco * (c1rt - 2.0 * c1 + c1lt);
      hord2  = hordco * (c2rt - 2.0 * c2 + c2lt);
      horad1 = horaco * (c1rt - c1lt);
      horad2 = horaco * (c2rt - c2lt);

      /* Load all terms into dudata */
      offsetu             = lx * NVARS + ly * nvmxsub;
      dudata[offsetu]     = vertd1 + hord1 + horad1 + rkin1;
      dudata[offsetu + 1] = vertd2 + hord2 + horad2 + rkin2;
    }
  }
}

/*
 * Print current t, step count, order, stepsize, and sampled c1,c2 values.
 */

static void PrintOutput(void* cvode_mem, int my_pe, MPI_Comm comm,
                        sunrealtype t, N_Vector u)
{
  long int nst;
  int qu, npelast, retval;
  sunrealtype hu, *udata, tempu[2];
  sunindextype i0, i1;
  MPI_Status status;

  npelast = NPEX * NPEY - 1;
  udata   = N_VGetArrayPointer(u);

  /* Send c at top right mesh point to PE 0 */
  if (my_pe == npelast)
  {
    i0 = NVARS * MXSUB * MYSUB - 2;
    i1 = i0 + 1;
    if (npelast != 0) { MPI_Send(&udata[i0], 2, MPI_SUNREALTYPE, 0, 0, comm); }
    else
    {
      tempu[0] = udata[i0];
      tempu[1] = udata[i1];
    }
  }

  /* On PE 0, receive c at top right, then print performance data
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
    printf("%8.3Le %2d  %8.3Le %5ld\n", t, qu, hu, nst);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("%8.3e %2d  %8.3e %5ld\n", t, qu, hu, nst);
#else
    printf("%8.3e %2d  %8.3e %5ld\n", t, qu, hu, nst);
#endif

    printf("                                Solution       ");
#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf("%12.4Le %12.4Le \n", udata[0], tempu[0]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("%12.4e %12.4e \n", udata[0], tempu[0]);
#else
    printf("%12.4e %12.4e \n", udata[0], tempu[0]);
#endif

    printf("                                               ");

#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf("%12.4Le %12.4Le \n", udata[1], tempu[1]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("%12.4e %12.4e \n", udata[1], tempu[1]);
#else
    printf("%12.4e %12.4e \n", udata[1], tempu[1]);
#endif
  }
}

/*
 * Print sampled sensitivity values.
 */

static void PrintOutputS(int my_pe, MPI_Comm comm, N_Vector* uS)
{
  sunrealtype *sdata, temps[2];
  int npelast;
  sunindextype i0, i1;
  MPI_Status status;

  npelast = NPEX * NPEY - 1;

  sdata = N_VGetArrayPointer(uS[0]);

  /* Send s1 at top right mesh point to PE 0 */
  if (my_pe == npelast)
  {
    i0 = NVARS * MXSUB * MYSUB - 2;
    i1 = i0 + 1;
    if (npelast != 0) { MPI_Send(&sdata[i0], 2, MPI_SUNREALTYPE, 0, 0, comm); }
    else
    {
      temps[0] = sdata[i0];
      temps[1] = sdata[i1];
    }
  }

  /* On PE 0, receive s1 at top right, then print sampled sensitivity values */
  if (my_pe == 0)
  {
    if (npelast != 0)
    {
      MPI_Recv(&temps[0], 2, MPI_SUNREALTYPE, npelast, 0, comm, &status);
    }
    printf("                                "
           "----------------------------------------\n");
    printf("                                Sensitivity 1  ");
#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf("%12.4Le %12.4Le \n", sdata[0], temps[0]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("%12.4e %12.4e \n", sdata[0], temps[0]);
#else
    printf("%12.4e %12.4e \n", sdata[0], temps[0]);
#endif
    printf("                                               ");
#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf("%12.4Le %12.4Le \n", sdata[1], temps[1]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("%12.4e %12.4e \n", sdata[1], temps[1]);
#else
    printf("%12.4e %12.4e \n", sdata[1], temps[1]);
#endif
  }

  sdata = N_VGetArrayPointer(uS[1]);

  /* Send s2 at top right mesh point to PE 0 */
  if (my_pe == npelast)
  {
    i0 = NVARS * MXSUB * MYSUB - 2;
    i1 = i0 + 1;
    if (npelast != 0) { MPI_Send(&sdata[i0], 2, MPI_SUNREALTYPE, 0, 0, comm); }
    else
    {
      temps[0] = sdata[i0];
      temps[1] = sdata[i1];
    }
  }

  /* On PE 0, receive s2 at top right, then print sampled sensitivity values */
  if (my_pe == 0)
  {
    if (npelast != 0)
    {
      MPI_Recv(&temps[0], 2, MPI_SUNREALTYPE, npelast, 0, comm, &status);
    }
    printf("                                "
           "----------------------------------------\n");
    printf("                                Sensitivity 2  ");
#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf("%12.4Le %12.4Le \n", sdata[0], temps[0]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("%12.4e %12.4e \n", sdata[0], temps[0]);
#else
    printf("%12.4e %12.4e \n", sdata[0], temps[0]);
#endif
    printf("                                               ");
#if defined(SUNDIALS_EXTENDED_PRECISION)
    printf("%12.4Le %12.4Le \n", sdata[1], temps[1]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
    printf("%12.4e %12.4e \n", sdata[1], temps[1]);
#else
    printf("%12.4e %12.4e \n", sdata[1], temps[1]);
#endif
  }
}

/*
 * Print final statistics from the CVODES memory.
 */

static void PrintFinalStats(void* cvode_mem, sunbooleantype sensi,
                            sunbooleantype err_con, int sensi_meth)
{
  long int nst;
  long int nfe, nsetups, nni, ncfn, netf;
  long int nfSe, nfeS, nsetupsS, nniS, ncfnS, netfS;
  int retval;

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

  if (sensi)
  {
    retval = CVodeGetSensNumRhsEvals(cvode_mem, &nfSe);
    check_retval(&retval, "CVodeGetSensNumRhsEvals", 1, 0);
    retval = CVodeGetNumRhsEvalsSens(cvode_mem, &nfeS);
    check_retval(&retval, "CVodeGetNumRhsEvalsSens", 1, 0);
    retval = CVodeGetSensNumLinSolvSetups(cvode_mem, &nsetupsS);
    check_retval(&retval, "CVodeGetSensNumLinSolvSetups", 1, 0);
    if (err_con)
    {
      retval = CVodeGetSensNumErrTestFails(cvode_mem, &netfS);
      check_retval(&retval, "CVodeGetSensNumErrTestFails", 1, 0);
    }
    else { netfS = 0; }
    if ((sensi_meth == CV_STAGGERED) || (sensi_meth == CV_STAGGERED1))
    {
      retval = CVodeGetSensNumNonlinSolvIters(cvode_mem, &nniS);
      check_retval(&retval, "CVodeGetSensNumNonlinSolvIters", 1, 0);
      retval = CVodeGetSensNumNonlinSolvConvFails(cvode_mem, &ncfnS);
      check_retval(&retval, "CVodeGetSensNumNonlinSolvConvFails", 1, 0);
    }
    else
    {
      nniS  = 0;
      ncfnS = 0;
    }
  }

  printf("\nFinal Statistics\n\n");
  printf("nst     = %5ld\n\n", nst);
  printf("nfe     = %5ld\n", nfe);
  printf("netf    = %5ld    nsetups  = %5ld\n", netf, nsetups);
  printf("nni     = %5ld    ncfn     = %5ld\n", nni, ncfn);

  if (sensi)
  {
    printf("\n");
    printf("nfSe    = %5ld    nfeS     = %5ld\n", nfSe, nfeS);
    printf("netfs   = %5ld    nsetupsS = %5ld\n", netfS, nsetupsS);
    printf("nniS    = %5ld    ncfnS    = %5ld\n", nniS, ncfnS);
  }
}

/*
 * Check function return value...
 *   opt == 0 means SUNDIALS function allocates memory so check if
 *            returned NULL pointer
 *   opt == 1 means SUNDIALS function returns an integer value so check if
 *            retval < 0
 *   opt == 2 means function allocates memory so check if returned
 *            NULL pointer
 */

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
