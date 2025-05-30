/* --------------------------------------------------------------
 * Programmer(s): David J. Gardner @ LLNL
 * --------------------------------------------------------------
 * Based on the ark_brusselator1D_omp.c ARKode example by
 * Daniel R. Reynolds @ SMU.
 * --------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2025, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * --------------------------------------------------------------
 * This program simulates 1D advection-reaction problem. The
 * brusselator problem from chemical kinetics is used for the
 * reaction terms. This is a PDE system with 3 components,
 * Y = [u,v,w], satisfying the equations,
 *
 *    u_t = -c*u_x + a - (w+1)*u + v*u^2
 *    v_t = -c*v_x + w*u - v*u^2
 *    w_t = -c*w_x + (b-w)/ep - w*u
 *
 * for t in [0, 10], x in [0, 1], with initial conditions
 *
 *    u(0,x) =  a  + 0.1*exp(-(x-0.5)^2 / 0.1)
 *    v(0,x) = b/a + 0.1*exp(-(x-0.5)^2 / 0.1)
 *    w(0,x) =  b  + 0.1*exp(-(x-0.5)^2 / 0.1),
 *
 * and with periodic boundary conditions i.e.,
 *
 *    u(t,0) = u(t,1),
 *    v(t,0) = v(t,1),
 *    w(t,0) = w(t,1).
 *
 * The spatial derivatives are computed using first-order
 * upwind differences for the advection terms. The data is
 * distributed over N points on a uniform spatial grid.
 *
 * This program use the MRIStep module with an explicit slow
 * method and an implicit fast method. The explicit method
 * uses a fixed step size and the implicit method uses adaptive
 * steps. Implicit systems are solved using a Newton iteration
 * with the band linear solver, and a user-supplied Jacobian
 * routine for the fast RHS.
 *
 * 100 outputs are printed at equal intervals, and run statistics
 * are printed at the end.
 * --------------------------------------------------------------*/

/* Header files */
#include <arkode/arkode_arkstep.h> /* prototypes for ARKStep fcts., consts */
#include <arkode/arkode_mristep.h> /* prototypes for MRIStep fcts., consts */
#include <math.h>
#include <nvector/nvector_serial.h> /* access to Serial N_Vector */
#include <stdio.h>
#include <stdlib.h>
#include <sundials/sundials_math.h>   /* def. of SUNRsqrt, etc. */
#include <sundials/sundials_types.h>  /* def. of type 'sunrealtype' */
#include <sunlinsol/sunlinsol_band.h> /* access to band SUNLinearSolver */
#include <sunmatrix/sunmatrix_band.h> /* access to band SUNMatrix */

#if defined(SUNDIALS_EXTENDED_PRECISION)
#define GSYM "Lg"
#define ESYM "Le"
#define FSYM "Lf"
#else
#define GSYM "g"
#define ESYM "e"
#define FSYM "f"
#endif

/* Define some constants */
#define ZERO SUN_RCONST(0.0)
#define ONE  SUN_RCONST(1.0)
#define TWO  SUN_RCONST(2.0)

/* accessor macros between (x,v) location and 1D NVector array */
#define IDX(x, v) (3 * (x) + v)

/* user data structure */
typedef struct
{
  sunindextype N; /* number of intervals      */
  sunrealtype dx; /* mesh spacing             */
  sunrealtype a;  /* constant forcing on u    */
  sunrealtype b;  /* steady-state value of w  */
  sunrealtype c;  /* advection coefficient    */
  sunrealtype ep; /* stiffness parameter      */
}* UserData;

/* user-provided functions called by the integrator */
static int ff(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data);
static int fs(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data);
static int Jf(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix Jac,
              void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

/* function for setting initial condition */
static int SetIC(N_Vector y, void* user_data);

/* function for checking return values */
static int check_retval(void* flagvalue, const char* funcname, int opt);

/* Main Program */
int main(int argc, char* argv[])
{
  /* general problem parameters */
  sunrealtype T0     = SUN_RCONST(0.0);  /* initial time                    */
  sunrealtype Tf     = SUN_RCONST(10.0); /* final time                      */
  int Nt             = 100;              /* total number of output times    */
  int Nvar           = 3;                /* number of solution fields       */
  sunindextype N     = 200;              /* spatial mesh size (N intervals) */
  sunrealtype a      = SUN_RCONST(1.0);  /* problem parameters              */
  sunrealtype b      = SUN_RCONST(3.5);
  sunrealtype c      = SUN_RCONST(0.25);
  sunrealtype ep     = SUN_RCONST(1.0e-6); /* stiffness parameter */
  sunrealtype reltol = SUN_RCONST(1.0e-6); /* tolerances          */
  sunrealtype abstol = SUN_RCONST(1.0e-10);

  /* general problem variables */
  sunrealtype hs;                           /* slow step size                 */
  int retval;                               /* reusable return flag           */
  N_Vector y                        = NULL; /* empty solution vector          */
  N_Vector umask                    = NULL; /* empty mask vectors             */
  N_Vector vmask                    = NULL;
  N_Vector wmask                    = NULL;
  SUNMatrix A                       = NULL; /* empty matrix for linear solver */
  SUNLinearSolver LS                = NULL; /* empty linear solver structure  */
  void* arkode_mem                  = NULL; /* empty ARKode memory structure  */
  void* inner_arkode_mem            = NULL; /* empty ARKode memory structure  */
  MRIStepInnerStepper inner_stepper = NULL; /* inner stepper                  */
  sunrealtype t, dTout, tout;               /* current/output time data       */
  sunrealtype u, v, w;                      /* temp data values               */
  FILE *FID, *UFID, *VFID, *WFID;           /* output file pointers           */
  int iout;                                 /* output counter                 */
  long int nsts, nstf, nstf_a, netf;        /* step stats                     */
  long int nfse, nffi;                      /* RHS stats                      */
  long int nsetups, nje, nfeLS;             /* linear solver stats            */
  long int nni, ncfn;                       /* nonlinear solver stats         */
  sunindextype NEQ;                         /* number of equations            */
  sunindextype i;                           /* counter                        */
  UserData udata    = NULL;                 /* user data pointer              */
  sunrealtype* data = NULL;                 /* array for vector data          */

  /* Create the SUNDIALS context object for this simulation */
  SUNContext ctx;
  retval = SUNContext_Create(SUN_COMM_NULL, &ctx);
  if (check_retval(&retval, "SUNContext_Create", 1)) { return 1; }

  /* allocate udata structure */
  udata = (UserData)malloc(sizeof(*udata));
  if (check_retval((void*)udata, "malloc", 2)) { return 1; }

  /* store the inputs in the UserData structure */
  udata->N  = N;
  udata->a  = a;
  udata->b  = b;
  udata->c  = c;
  udata->ep = ep;
  udata->dx = SUN_RCONST(1.0) / N; /* periodic BC, divide by N not N-1 */

  /* set total allocated vector length */
  NEQ = Nvar * udata->N;

  /* set the slow step size */
  hs = SUN_RCONST(0.5) * (udata->dx / SUNRabs(c));

  /* Initial problem output */
  printf("\n1D Advection-Reaction example problem:\n");
  printf("    N = %li,  NEQ = %li\n", (long int)udata->N, (long int)NEQ);
  printf("    problem parameters:  a = %" GSYM ",  b = %" GSYM ",  ep = %" GSYM
         "\n",
         udata->a, udata->b, udata->ep);
  printf("    advection coefficient:  c = %" GSYM "\n", udata->c);
  printf("    reltol = %.1" ESYM ",  abstol = %.1" ESYM "\n\n", reltol, abstol);

  /* Create solution vector */
  y = N_VNew_Serial(NEQ, ctx); /* Create vector for solution */
  if (check_retval((void*)y, "N_VNew_Serial", 0)) { return 1; }

  /* Set initial condition */
  retval = SetIC(y, udata);
  if (check_retval(&retval, "SetIC", 1)) { return 1; }

  /* Create vector masks */
  umask = N_VClone(y);
  if (check_retval((void*)umask, "N_VClone", 0)) { return 1; }

  vmask = N_VClone(y);
  if (check_retval((void*)vmask, "N_VClone", 0)) { return 1; }

  wmask = N_VClone(y);
  if (check_retval((void*)wmask, "N_VClone", 0)) { return 1; }

  /* Set mask array values for each solution component */
  N_VConst(0.0, umask);
  data = N_VGetArrayPointer(umask);
  if (check_retval((void*)data, "N_VGetArrayPointer", 0)) { return 1; }
  for (i = 0; i < N; i++) { data[IDX(i, 0)] = SUN_RCONST(1.0); }

  N_VConst(0.0, vmask);
  data = N_VGetArrayPointer(vmask);
  if (check_retval((void*)data, "N_VGetArrayPointer", 0)) { return 1; }
  for (i = 0; i < N; i++) { data[IDX(i, 1)] = SUN_RCONST(1.0); }

  N_VConst(0.0, wmask);
  data = N_VGetArrayPointer(wmask);
  if (check_retval((void*)data, "N_VGetArrayPointer", 0)) { return 1; }
  for (i = 0; i < N; i++) { data[IDX(i, 2)] = SUN_RCONST(1.0); }

  /*
   * Create the fast integrator and set options
   */

  /* Initialize matrix and linear solver data structures */
  A = SUNBandMatrix(NEQ, 4, 4, ctx);
  if (check_retval((void*)A, "SUNBandMatrix", 0)) { return 1; }

  LS = SUNLinSol_Band(y, A, ctx);
  if (check_retval((void*)LS, "SUNLinSol_Band", 0)) { return 1; }

  /* Initialize the fast integrator. Specify the implicit fast right-hand side
     function in y'=fe(t,y)+fi(t,y)+ff(t,y), the initial time T0, and the
     initial dependent variable vector y. */
  inner_arkode_mem = ARKStepCreate(NULL, ff, T0, y, ctx);
  if (check_retval((void*)inner_arkode_mem, "ARKStepCreate", 0)) { return 1; }

  /* Attach user data to fast integrator */
  retval = ARKodeSetUserData(inner_arkode_mem, (void*)udata);
  if (check_retval(&retval, "ARKodeSetUserData", 1)) { return 1; }

  /* Set the fast method */
  retval = ARKStepSetTableNum(inner_arkode_mem, ARKODE_ARK324L2SA_DIRK_4_2_3, -1);
  if (check_retval(&retval, "ARKStepSetTableNum", 1)) { return 1; }

  /* Specify fast tolerances */
  retval = ARKodeSStolerances(inner_arkode_mem, reltol, abstol);
  if (check_retval(&retval, "ARKodeSStolerances", 1)) { return 1; }

  /* Attach matrix and linear solver */
  retval = ARKodeSetLinearSolver(inner_arkode_mem, LS, A);
  if (check_retval(&retval, "ARKodeSetLinearSolver", 1)) { return 1; }

  /* Set the Jacobian routine */
  retval = ARKodeSetJacFn(inner_arkode_mem, Jf);
  if (check_retval(&retval, "ARKodeSetJacFn", 1)) { return 1; }

  /* Create inner stepper */
  retval = ARKodeCreateMRIStepInnerStepper(inner_arkode_mem, &inner_stepper);
  if (check_retval(&retval, "ARKodeCreateMRIStepInnerStepper", 1)) { return 1; }

  /*
   * Create the slow integrator and set options
   */

  /* Initialize the slow integrator. Specify the explicit slow right-hand side
     function in y'=fe(t,y)+fi(t,y)+ff(t,y), the initial time T0, the
     initial dependent variable vector y, and the fast integrator. */
  arkode_mem = MRIStepCreate(fs, NULL, T0, y, inner_stepper, ctx);
  if (check_retval((void*)arkode_mem, "MRIStepCreate", 0)) { return 1; }

  /* Pass udata to user functions */
  retval = ARKodeSetUserData(arkode_mem, (void*)udata);
  if (check_retval(&retval, "ARKodeSetUserData", 1)) { return 1; }

  /* Set the slow step size */
  retval = ARKodeSetFixedStep(arkode_mem, hs);
  if (check_retval(&retval, "ARKodeSetFixedStep", 1)) { return 1; }

  /* output spatial mesh to disk (add extra point for periodic BC) */
  FID = fopen("mesh.txt", "w");
  for (i = 0; i < N + 1; i++)
  {
    fprintf(FID, "  %.16" ESYM "\n", udata->dx * i);
  }
  fclose(FID);

  /* Open output stream for results, access data arrays */
  UFID = fopen("u.txt", "w");
  VFID = fopen("v.txt", "w");
  WFID = fopen("w.txt", "w");

  /* output initial condition to disk (extra output for periodic BC) */
  data = N_VGetArrayPointer(y);
  if (check_retval((void*)data, "N_VGetArrayPointer", 0)) { return 1; }

  for (i = 0; i < N; i++) { fprintf(UFID, " %.16" ESYM, data[IDX(i, 0)]); }
  fprintf(UFID, " %.16" ESYM, data[IDX(0, 0)]);
  fprintf(UFID, "\n");

  for (i = 0; i < N; i++) { fprintf(VFID, " %.16" ESYM, data[IDX(i, 1)]); }
  fprintf(VFID, " %.16" ESYM, data[IDX(0, 1)]);
  fprintf(VFID, "\n");

  for (i = 0; i < N; i++) { fprintf(WFID, " %.16" ESYM, data[IDX(i, 2)]); }
  fprintf(WFID, " %.16" ESYM, data[IDX(0, 2)]);
  fprintf(WFID, "\n");

  /* Main time-stepping loop: calls ARKodeEvolve to perform the integration,
     then prints results.  Stops when the final time has been reached */
  t     = T0;
  dTout = (Tf - T0) / Nt;
  tout  = T0 + dTout;
  printf("        t      ||u||_rms   ||v||_rms   ||w||_rms\n");
  printf("   ----------------------------------------------\n");
  for (iout = 0; iout < Nt; iout++)
  {
    /* call integrator */
    retval = ARKodeEvolve(arkode_mem, tout, y, &t, ARK_NORMAL);
    if (check_retval(&retval, "ARKodeEvolve", 1)) { break; }

    /* access/print solution statistics */
    u = N_VWL2Norm(y, umask);
    u = SUNRsqrt(u * u / N);
    v = N_VWL2Norm(y, vmask);
    v = SUNRsqrt(v * v / N);
    w = N_VWL2Norm(y, wmask);
    w = SUNRsqrt(w * w / N);
    printf("  %10.6" FSYM "  %10.6" FSYM "  %10.6" FSYM "  %10.6" FSYM "\n", t,
           u, v, w);

    /* output results to disk (extr output for periodic BC) */
    for (i = 0; i < N; i++) { fprintf(UFID, " %.16" ESYM, data[IDX(i, 0)]); }
    fprintf(UFID, " %.16" ESYM, data[IDX(0, 0)]);
    fprintf(UFID, "\n");

    for (i = 0; i < N; i++) { fprintf(VFID, " %.16" ESYM, data[IDX(i, 1)]); }
    fprintf(VFID, " %.16" ESYM, data[IDX(0, 1)]);
    fprintf(VFID, "\n");

    for (i = 0; i < N; i++) { fprintf(WFID, " %.16" ESYM, data[IDX(i, 2)]); }
    fprintf(WFID, " %.16" ESYM, data[IDX(0, 2)]);
    fprintf(WFID, "\n");

    /* successful solve: update output time */
    tout += dTout;
    tout = (tout > Tf) ? Tf : tout;
  }
  printf("   ----------------------------------------------\n");
  fclose(UFID);
  fclose(VFID);
  fclose(WFID);

  /* Get some slow integrator statistics */
  retval = ARKodeGetNumSteps(arkode_mem, &nsts);
  check_retval(&retval, "ARKodeGetNumSteps", 1);
  retval = ARKodeGetNumRhsEvals(arkode_mem, 0, &nfse);
  check_retval(&retval, "ARKodeGetNumRhsEvals", 1);

  /* Get some fast integrator statistics */
  retval = ARKodeGetNumSteps(inner_arkode_mem, &nstf);
  check_retval(&retval, "ARKodeGetNumSteps", 1);
  retval = ARKodeGetNumStepAttempts(inner_arkode_mem, &nstf_a);
  check_retval(&retval, "ARKodeGetNumStepAttempts", 1);
  retval = ARKodeGetNumRhsEvals(inner_arkode_mem, 1, &nffi);
  check_retval(&retval, "ARKodeGetNumRhsEvals", 1);
  retval = ARKodeGetNumLinSolvSetups(inner_arkode_mem, &nsetups);
  check_retval(&retval, "ARKodeGetNumLinSolvSetups", 1);
  retval = ARKodeGetNumErrTestFails(inner_arkode_mem, &netf);
  check_retval(&retval, "ARKodeGetNumErrTestFails", 1);
  retval = ARKodeGetNumNonlinSolvIters(inner_arkode_mem, &nni);
  check_retval(&retval, "ARKodeGetNumNonlinSolvIters", 1);
  retval = ARKodeGetNumNonlinSolvConvFails(inner_arkode_mem, &ncfn);
  check_retval(&retval, "ARKodeGetNumNonlinSolvConvFails", 1);
  retval = ARKodeGetNumJacEvals(inner_arkode_mem, &nje);
  check_retval(&retval, "ARKodeGetNumJacEvals", 1);
  retval = ARKodeGetNumLinRhsEvals(inner_arkode_mem, &nfeLS);
  check_retval(&retval, "ARKodeGetNumLinRhsEvals", 1);

  /* Print some final statistics */
  printf("\nFinal Solver Statistics:\n");
  printf("   Slow Steps: nsts = %li\n", nsts);
  printf("   Fast Steps: nstf = %li (attempted = %li)\n", nstf, nstf_a);
  printf("   Total RHS evals:  Fs = %li,  Ff = %li\n", nfse, nffi);
  printf("   Total number of fast error test failures = %li\n", netf);
  printf("   Total linear solver setups = %li\n", nsetups);
  printf("   Total RHS evals for setting up the linear system = %li\n", nfeLS);
  printf("   Total number of Jacobian evaluations = %li\n", nje);
  printf("   Total number of Newton iterations = %li\n", nni);
  printf("   Total number of nonlinear solver convergence failures = %li\n",
         ncfn);

  /* Clean up and return with successful completion */
  free(udata);                              /* Free user data         */
  ARKodeFree(&inner_arkode_mem);            /* Free integrator memory */
  MRIStepInnerStepper_Free(&inner_stepper); /* Free inner stepper */
  ARKodeFree(&arkode_mem);                  /* Free integrator memory */
  SUNLinSolFree(LS);                        /* Free linear solver     */
  SUNMatDestroy(A);                         /* Free matrix            */
  N_VDestroy(y);                            /* Free vectors           */
  N_VDestroy(umask);
  N_VDestroy(vmask);
  N_VDestroy(wmask);
  SUNContext_Free(&ctx); /* Free context */

  return 0;
}

/* -----------------------------------
 * Functions called by the integrator
 * -----------------------------------*/

/* ff routine to compute the fast portion of the ODE RHS. */
static int ff(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data)
{
  UserData udata      = (UserData)user_data; /* access problem data    */
  sunindextype N      = udata->N;            /* set variable shortcuts */
  sunrealtype a       = udata->a;
  sunrealtype b       = udata->b;
  sunrealtype ep      = udata->ep;
  sunrealtype* Ydata  = NULL;
  sunrealtype* dYdata = NULL;
  sunrealtype u, v, w;
  sunindextype i;

  /* access data arrays */
  Ydata = N_VGetArrayPointer(y);
  if (check_retval((void*)Ydata, "N_VGetArrayPointer", 0)) { return 1; }

  dYdata = N_VGetArrayPointer(ydot);
  if (check_retval((void*)dYdata, "N_VGetArrayPointer", 0)) { return 1; }

  /* iterate over domain, computing reactions */
  for (i = 0; i < N; i++)
  {
    /* set shortcuts */
    u = Ydata[IDX(i, 0)];
    v = Ydata[IDX(i, 1)];
    w = Ydata[IDX(i, 2)];

    /* u_t = a - (w+1)*u + v*u^2 */
    dYdata[IDX(i, 0)] = a - (w + ONE) * u + v * u * u;

    /* v_t = w*u - v*u^2 */
    dYdata[IDX(i, 1)] = w * u - v * u * u;

    /* w_t = (b-w)/ep - w*u */
    dYdata[IDX(i, 2)] = (b - w) / ep - w * u;
  }

  /* return success */
  return (0);
}

/* fs routine to compute the slow portion of the ODE RHS. */
static int fs(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data)
{
  UserData udata      = (UserData)user_data; /* access problem data    */
  sunindextype N      = udata->N;            /* set variable shortcuts */
  sunrealtype c       = udata->c;
  sunrealtype dx      = udata->dx;
  sunrealtype* Ydata  = NULL;
  sunrealtype* dYdata = NULL;
  sunrealtype tmp;
  sunindextype i;

  /* access data arrays */
  Ydata = N_VGetArrayPointer(y);
  if (check_retval((void*)Ydata, "N_VGetArrayPointer", 0)) { return 1; }

  dYdata = N_VGetArrayPointer(ydot);
  if (check_retval((void*)dYdata, "N_VGetArrayPointer", 0)) { return 1; }

  /* iterate over domain, computing advection */
  tmp = -c / dx;

  if (c > ZERO)
  {
    /*
     * right moving flow
     */

    /* left boundary Jacobian entries */
    dYdata[IDX(0, 0)] = tmp * (Ydata[IDX(0, 0)] - Ydata[IDX(N - 1, 0)]);
    dYdata[IDX(0, 1)] = tmp * (Ydata[IDX(0, 1)] - Ydata[IDX(N - 1, 1)]);
    dYdata[IDX(0, 2)] = tmp * (Ydata[IDX(0, 2)] - Ydata[IDX(N - 1, 2)]);

    /* interior Jacobian entries */
    for (i = 1; i < N; i++)
    {
      dYdata[IDX(i, 0)] = tmp * (Ydata[IDX(i, 0)] - Ydata[IDX(i - 1, 0)]);
      dYdata[IDX(i, 1)] = tmp * (Ydata[IDX(i, 1)] - Ydata[IDX(i - 1, 1)]);
      dYdata[IDX(i, 2)] = tmp * (Ydata[IDX(i, 2)] - Ydata[IDX(i - 1, 2)]);
    }
  }
  else if (c < ZERO)
  {
    /*
     * left moving flow
     */

    /* interior Jacobian entries */
    for (i = 0; i < N - 1; i++)
    {
      dYdata[IDX(i, 0)] = tmp * (Ydata[IDX(i + 1, 0)] - Ydata[IDX(i, 0)]);
      dYdata[IDX(i, 1)] = tmp * (Ydata[IDX(i + 1, 1)] - Ydata[IDX(i, 1)]);
      dYdata[IDX(i, 2)] = tmp * (Ydata[IDX(i + 1, 2)] - Ydata[IDX(i, 2)]);
    }

    /* right boundary Jacobian entries */
    dYdata[IDX(N - 1, 0)] = tmp * (Ydata[IDX(N - 1, 0)] - Ydata[IDX(0, 0)]);
    dYdata[IDX(N - 1, 1)] = tmp * (Ydata[IDX(N - 1, 1)] - Ydata[IDX(0, 1)]);
    dYdata[IDX(N - 1, 2)] = tmp * (Ydata[IDX(N - 1, 2)] - Ydata[IDX(0, 2)]);
  }

  /* return success */
  return (0);
}

/* Js routine to compute the Jacobian of the fast portion of the ODE RHS. */
static int Jf(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix Jac,
              void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  UserData udata = (UserData)user_data; /* access problem data */
  sunindextype N = udata->N;            /* set shortcuts */
  sunrealtype ep = udata->ep;
  sunindextype i;
  sunrealtype u, v, w;
  sunrealtype* Ydata = NULL;

  /* access solution array */
  Ydata = N_VGetArrayPointer(y);
  if (check_retval((void*)Ydata, "N_VGetArrayPointer", 0)) { return 1; }

  /* iterate over nodes, filling in Jacobian entries */
  for (i = 0; i < N; i++)
  {
    /* set nodal value shortcuts (shifted index due to start at first interior node) */
    u = Ydata[IDX(i, 0)];
    v = Ydata[IDX(i, 1)];
    w = Ydata[IDX(i, 2)];

    /* all vars wrt u */
    SM_ELEMENT_B(Jac, IDX(i, 0), IDX(i, 0)) = TWO * u * v - (w + ONE);
    SM_ELEMENT_B(Jac, IDX(i, 1), IDX(i, 0)) = w - TWO * u * v;
    SM_ELEMENT_B(Jac, IDX(i, 2), IDX(i, 0)) = -w;

    /* all vars wrt v */
    SM_ELEMENT_B(Jac, IDX(i, 0), IDX(i, 1)) = u * u;
    SM_ELEMENT_B(Jac, IDX(i, 1), IDX(i, 1)) = -u * u;

    /* all vars wrt w */
    SM_ELEMENT_B(Jac, IDX(i, 0), IDX(i, 2)) = -u;
    SM_ELEMENT_B(Jac, IDX(i, 1), IDX(i, 2)) = u;
    SM_ELEMENT_B(Jac, IDX(i, 2), IDX(i, 2)) = -ONE / ep - u;
  }

  /* return success */
  return (0);
}

/* ------------------------------
 * Private helper functions
 * ------------------------------*/

/* Set the initial condition */
static int SetIC(N_Vector y, void* user_data)
{
  UserData udata    = (UserData)user_data; /* access problem data    */
  sunindextype N    = udata->N;            /* set variable shortcuts */
  sunrealtype a     = udata->a;
  sunrealtype b     = udata->b;
  sunrealtype dx    = udata->dx;
  sunrealtype* data = NULL;

  sunrealtype x, p;
  sunindextype i;

  /* Access data array from NVector y */
  data = N_VGetArrayPointer(y);

  /* Set initial conditions into y */
  for (i = 0; i < N; i++)
  {
    x = i * dx;
    p = SUN_RCONST(0.1) *
        SUNRexp(-(SUNSQR(x - SUN_RCONST(0.5))) / SUN_RCONST(0.1));
    data[IDX(i, 0)] = a + p;
    data[IDX(i, 1)] = b / a + p;
    data[IDX(i, 2)] = b + p;
  }

  /* return success */
  return (0);
}

/* --------------------------------------------------------------
 * Function to check return values:
 *
 * opt == 0  means SUNDIALS function allocates memory so check if
 *           returned NULL pointer
 * opt == 1  means SUNDIALS function returns a flag so check if
 *           flag < 0
 * opt == 2  means function allocates memory so check if returned
 *           NULL pointer
 * --------------------------------------------------------------*/
static int check_retval(void* returnvalue, const char* funcname, int opt)
{
  int* errvalue;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && returnvalue == NULL)
  {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return 1;
  }

  /* Check if flag < 0 */
  else if (opt == 1)
  {
    errvalue = (int*)returnvalue;
    if (*errvalue < 0)
    {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n",
              funcname, *errvalue);
      return 1;
    }
  }

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && returnvalue == NULL)
  {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return 1;
  }

  return 0;
}

/*---- end of file ----*/
