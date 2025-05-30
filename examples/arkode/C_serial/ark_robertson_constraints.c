/*---------------------------------------------------------------
 * Programmer(s): Daniel R. Reynolds @ SMU
 *---------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2025, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 *---------------------------------------------------------------
 * Example problem:
 *
 * The following test simulates the Robertson problem,
 * corresponding to the kinetics of an autocatalytic reaction.
 * This is an ODE system with 3 components, Y = [u,v,w], satisfying
 * the equations,
 *    du/dt = -0.04*u + 1e4*v*w
 *    dv/dt = 0.04*u - 1e4*v*w - 3e7*v^2
 *    dw/dt = 3e7*v^2
 * for t in the interval [0.0, 1e11], with initial conditions
 * Y0 = [1,0,0].
 *
 * This program solves the problem with one of the solvers, ERK,
 * DIRK or ARK.  For DIRK and ARK, implicit subsystems are solved
 * using a Newton iteration with the dense SUNLinearSolver, and a
 * user-supplied Jacobian routine. The constraint y_i >= 0 is
 * posed for all components.
 *
 * 100 outputs are printed at equal intervals, and run statistics
 * are printed at the end.
 *---------------------------------------------------------------*/

/* Header files */
#include <arkode/arkode_arkstep.h> /* prototypes for ARKStep fcts., consts */
#include <math.h>
#include <nvector/nvector_serial.h> /* serial N_Vector types, fcts., macros */
#include <stdio.h>
#include <sundials/sundials_types.h> /* defs. of 'sunrealtype', 'sunindextype'  */
#include <sunlinsol/sunlinsol_dense.h> /* access to dense SUNLinearSolver      */
#include <sunmatrix/sunmatrix_dense.h> /* access to dense SUNMatrix            */

#if defined(SUNDIALS_EXTENDED_PRECISION)
#define GSYM "Lg"
#define ESYM "Le"
#else
#define GSYM "g"
#define ESYM "e"
#endif

/* User-supplied Functions Called by the Solver */
static int f(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data);
static int Jac(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix J,
               void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

/* Private function to check function return values */
static int check_flag(void* flagvalue, const char* funcname, int opt);

/* Private function to check computed solution */
static int check_ans(N_Vector y, sunrealtype t, sunrealtype rtol,
                     sunrealtype atol);

/* Main Program */
int main(void)
{
  sunrealtype ONE = SUN_RCONST(1.0);

  /* general problem parameters */
  sunrealtype T0    = SUN_RCONST(0.0);       /* initial time */
  sunrealtype Tf    = SUN_RCONST(1.e11);     /* final time */
  sunrealtype dTout = (Tf - T0) / 100;       /* time between outputs */
  int Nt            = (int)ceil(Tf / dTout); /* number of output times */
  sunindextype NEQ  = 3;                     /* number of dependent vars. */

  /* general problem variables */
  int flag;                    /* reusable error-checking flag */
  N_Vector y           = NULL; /* empty vector for storing solution */
  N_Vector constraints = NULL; /* empty vector for storing constraints */
  SUNMatrix A          = NULL; /* empty matrix for linear solver */
  SUNLinearSolver LS   = NULL; /* empty linear solver object */
  void* arkode_mem     = NULL; /* empty ARKode memory structure */
  FILE* UFID;
  sunrealtype t, tout;
  int iout;
  long int nst, nst_a, nfe, nfi, nsetups, nje, nfeLS, nni, nnf, ncfn, netf, nctf;

  /* set up the initial conditions, tolerances, initial time step size */
  sunrealtype u0     = SUN_RCONST(1.0);
  sunrealtype v0     = SUN_RCONST(0.0);
  sunrealtype w0     = SUN_RCONST(0.0);
  sunrealtype reltol = SUN_RCONST(1.0e-3);
  sunrealtype abstol = SUN_RCONST(1.0e-7);
  sunrealtype h0     = SUN_RCONST(1.0e-4) * reltol;

  /* Create the SUNDIALS context object for this simulation */
  SUNContext ctx;
  flag = SUNContext_Create(SUN_COMM_NULL, &ctx);
  if (check_flag(&flag, "SUNContext_Create", 1)) { return 1; }

  /* Initial problem output */
  printf("\nRobertson ODE test problem:\n");
  printf("    initial conditions:  u0 = %" GSYM ",  v0 = %" GSYM
         ",  w0 = %" GSYM "\n",
         u0, v0, w0);

  /* Initialize data structures */
  y = N_VNew_Serial(NEQ, ctx); /* Create serial vector for solution */
  if (check_flag((void*)y, "N_VNew_Serial", 0)) { return 1; }
  NV_Ith_S(y, 0) = u0; /* Set initial conditions into y */
  NV_Ith_S(y, 1) = v0;
  NV_Ith_S(y, 2) = w0;

  constraints = N_VClone(y);
  if (check_flag((void*)constraints, "N_VNew_Serial", 0)) { return 1; }
  /* Set constraints to all 1's for nonnegative solution values. */
  N_VConst(ONE, constraints);

  /* Call ARKStepCreate to initialize the ARK timestepper module and
     specify the right-hand side function in y'=f(t,y), the initial time
     T0, and the initial dependent variable vector y.  Note: since this
     problem is fully implicit, we set f_E to NULL and f_I to f. */
  arkode_mem = ARKStepCreate(NULL, f, T0, y, ctx);
  if (check_flag((void*)arkode_mem, "ARKStepCreate", 0)) { return 1; }

  /* Set routines */
  flag = ARKodeSetInitStep(arkode_mem, h0); /* Set custom initial step */
  if (check_flag(&flag, "ARKodeSetInitStep", 1)) { return 1; }
  flag = ARKodeSetMaxErrTestFails(arkode_mem,
                                  20); /* Increase max error test fails */
  if (check_flag(&flag, "ARKodeSetMaxErrTestFails", 1)) { return 1; }
  flag = ARKodeSetMaxNonlinIters(arkode_mem, 8); /* Increase max nonlin iters  */
  if (check_flag(&flag, "ARKodeSetMaxNonlinIters", 1)) { return 1; }
  flag = ARKodeSetNonlinConvCoef(arkode_mem,
                                 SUN_RCONST(1.e-7)); /* Set nonlinear convergence coeff. */
  if (check_flag(&flag, "ARKodeSetNonlinConvCoef", 1)) { return 1; }
  flag = ARKodeSetMaxNumSteps(arkode_mem, 100000); /* Increase max num steps */
  if (check_flag(&flag, "ARKodeSetMaxNumSteps", 1)) { return 1; }
  flag = ARKodeSetPredictorMethod(arkode_mem,
                                  1); /* Specify maximum-order predictor */
  if (check_flag(&flag, "ARKodeSetPredictorMethod", 1)) { return 1; }
  flag = ARKodeSStolerances(arkode_mem, reltol, abstol); /* Specify tolerances */
  if (check_flag(&flag, "ARKodeSStolerances", 1)) { return 1; }
  flag = ARKodeSetConstraints(arkode_mem, constraints); /* Set constraints */
  if (check_flag(&flag, "ARKodeSetConstraints", 1)) { return 1; }

  /* Initialize dense matrix data structure and solver */
  A = SUNDenseMatrix(NEQ, NEQ, ctx);
  if (check_flag((void*)A, "SUNDenseMatrix", 0)) { return 1; }
  LS = SUNLinSol_Dense(y, A, ctx);
  if (check_flag((void*)LS, "SUNLinSol_Dense", 0)) { return 1; }

  /* Linear solver interface */
  flag = ARKodeSetLinearSolver(arkode_mem, LS,
                               A); /* Attach matrix and linear solver */
  if (check_flag(&flag, "ARKodeSetLinearSolver", 1)) { return 1; }
  flag = ARKodeSetJacFn(arkode_mem, Jac); /* Set the Jacobian routine */
  if (check_flag(&flag, "ARKodeSetJacFn", 1)) { return 1; }

  /* Open output stream for results, output comment line */
  UFID = fopen("solution.txt", "w");
  fprintf(UFID, "# t u v w\n");

  /* output initial condition to disk */
  fprintf(UFID, " %.16" ESYM " %.16" ESYM " %.16" ESYM " %.16" ESYM "\n", T0,
          NV_Ith_S(y, 0), NV_Ith_S(y, 1), NV_Ith_S(y, 2));

  /* Main time-stepping loop: calls ARKodeEvolve to perform the integration, then
     prints results.  Stops when the final time has been reached */
  t    = T0;
  tout = T0 + dTout;
  printf("        t           u           v           w\n");
  printf("   --------------------------------------------------\n");
  printf("  %10.3" ESYM "  %12.5" ESYM "  %12.5" ESYM "  %12.5" ESYM "\n", t,
         NV_Ith_S(y, 0), NV_Ith_S(y, 1), NV_Ith_S(y, 2));
  for (iout = 0; iout < Nt; iout++)
  {
    flag = ARKodeEvolve(arkode_mem, tout, y, &t, ARK_NORMAL); /* call integrator */
    if (check_flag(&flag, "ARKodeEvolve", 1)) { break; }
    printf("  %10.3" ESYM "  %12.5" ESYM "  %12.5" ESYM "  %12.5" ESYM
           "\n", /* access/print solution */
           t, NV_Ith_S(y, 0), NV_Ith_S(y, 1), NV_Ith_S(y, 2));
    fprintf(UFID, " %.16" ESYM " %.16" ESYM " %.16" ESYM " %.16" ESYM "\n", t,
            NV_Ith_S(y, 0), NV_Ith_S(y, 1), NV_Ith_S(y, 2));
    if (flag >= 0)
    { /* successful solve: update time */
      tout += dTout;
      tout = (tout > Tf) ? Tf : tout;
    }
    else
    { /* unsuccessful solve: break */
      fprintf(stderr, "Solver failure, stopping integration\n");
      break;
    }
  }
  printf("   --------------------------------------------------\n");
  fclose(UFID);

  /* Print some final statistics */
  flag = ARKodeGetNumSteps(arkode_mem, &nst);
  check_flag(&flag, "ARKodeGetNumSteps", 1);
  flag = ARKodeGetNumStepAttempts(arkode_mem, &nst_a);
  check_flag(&flag, "ARKodeGetNumStepAttempts", 1);
  flag = ARKodeGetNumRhsEvals(arkode_mem, 0, &nfe);
  check_flag(&flag, "ARKodeGetNumRhsEvals", 1);
  flag = ARKodeGetNumRhsEvals(arkode_mem, 1, &nfi);
  check_flag(&flag, "ARKodeGetNumRhsEvals", 1);
  flag = ARKodeGetNumLinSolvSetups(arkode_mem, &nsetups);
  check_flag(&flag, "ARKodeGetNumLinSolvSetups", 1);
  flag = ARKodeGetNumErrTestFails(arkode_mem, &netf);
  check_flag(&flag, "ARKodeGetNumErrTestFails", 1);
  flag = ARKodeGetNumStepSolveFails(arkode_mem, &ncfn);
  check_flag(&flag, "ARKodeGetNumStepSolveFails", 1);
  flag = ARKodeGetNumNonlinSolvIters(arkode_mem, &nni);
  check_flag(&flag, "ARKodeGetNumNonlinSolvIters", 1);
  flag = ARKodeGetNumNonlinSolvConvFails(arkode_mem, &nnf);
  check_flag(&flag, "ARKodeGetNumNonlinSolvConvFails", 1);
  flag = ARKodeGetNumJacEvals(arkode_mem, &nje);
  check_flag(&flag, "ARKodeGetNumJacEvals", 1);
  flag = ARKodeGetNumLinRhsEvals(arkode_mem, &nfeLS);
  check_flag(&flag, "ARKodeGetNumLinRhsEvals", 1);
  flag = ARKodeGetNumConstrFails(arkode_mem, &nctf);
  check_flag(&flag, "ARKodeGetNumConstrFails", 1);

  printf("\nFinal Solver Statistics:\n");
  printf("   Internal solver steps = %li (attempted = %li)\n", nst, nst_a);
  printf("   Total RHS evals:  Fe = %li,  Fi = %li\n", nfe, nfi);
  printf("   Total linear solver setups = %li\n", nsetups);
  printf("   Total RHS evals for setting up the linear system = %li\n", nfeLS);
  printf("   Total number of Jacobian evaluations = %li\n", nje);
  printf("   Total number of Newton iterations = %li\n", nni);
  printf("   Total number of nonlinear solver convergence failures = %li\n", nnf);
  printf("   Total number of error test failures = %li\n", netf);
  printf("   Total number of constraint test failures = %li\n", nctf);
  printf("   Total number of failed steps from solver failure = %li\n", ncfn);

  /* check the solution error */
  flag = check_ans(y, t, reltol, abstol);

  /* Clean up and return with successful completion */
  N_VDestroy(y);           /* Free y vector */
  N_VDestroy(constraints); /* Free constraints vector */
  ARKodeFree(&arkode_mem); /* Free integrator memory */
  SUNLinSolFree(LS);       /* Free linear solver */
  SUNMatDestroy(A);        /* Free A matrix */
  SUNContext_Free(&ctx);   /* Free context */

  return flag;
}

/*-------------------------------
 * Functions called by the solver
 *-------------------------------*/

/* f routine to compute the ODE RHS function f(t,y). */
static int f(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data)
{
  sunrealtype u = NV_Ith_S(y, 0); /* access current solution */
  sunrealtype v = NV_Ith_S(y, 1);
  sunrealtype w = NV_Ith_S(y, 2);

  /* Fill in ODE RHS function */
  NV_Ith_S(ydot, 0) = -0.04 * u + 1.e4 * v * w;
  NV_Ith_S(ydot, 1) = 0.04 * u - 1.e4 * v * w - 3.e7 * v * v;
  NV_Ith_S(ydot, 2) = 3.e7 * v * v;

  return 0; /* Return with success */
}

/* Jacobian routine to compute J(t,y) = df/dy. */
static int Jac(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix J,
               void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  sunrealtype v = NV_Ith_S(y, 1); /* access current solution */
  sunrealtype w = NV_Ith_S(y, 2);
  SUNMatZero(J); /* initialize Jacobian to zero */

  /* Fill in the Jacobian of the ODE RHS function */
  SM_ELEMENT_D(J, 0, 0) = -0.04;
  SM_ELEMENT_D(J, 0, 1) = 1.e4 * w;
  SM_ELEMENT_D(J, 0, 2) = 1.e4 * v;

  SM_ELEMENT_D(J, 1, 0) = 0.04;
  SM_ELEMENT_D(J, 1, 1) = -1.e4 * w - 6.e7 * v;
  SM_ELEMENT_D(J, 1, 2) = -1.e4 * v;

  SM_ELEMENT_D(J, 2, 1) = 6.e7 * v;

  return 0; /* Return with success */
}

/*-------------------------------
 * Private helper functions
 *-------------------------------*/

/* Check function return value...
    opt == 0 means SUNDIALS function allocates memory so check if
             returned NULL pointer
    opt == 1 means SUNDIALS function returns a flag so check if
             flag >= 0
    opt == 2 means function allocates memory so check if returned
             NULL pointer
*/
static int check_flag(void* flagvalue, const char* funcname, int opt)
{
  int* errflag;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && flagvalue == NULL)
  {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return 1;
  }

  /* Check if flag < 0 */
  else if (opt == 1)
  {
    errflag = (int*)flagvalue;
    if (*errflag < 0)
    {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n",
              funcname, *errflag);
      return 1;
    }
  }

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && flagvalue == NULL)
  {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return 1;
  }

  return 0;
}

/* compare the solution at the final time 1e11s to a reference solution computed
   using a relative tolerance of 1e-8 and absolute tolerance of 1e-14 */
static int check_ans(N_Vector y, sunrealtype t, sunrealtype rtol, sunrealtype atol)
{
  int passfail = 0; /* answer pass (0) or fail (1) flag */
  N_Vector ref;     /* reference solution vector        */
  N_Vector ewt;     /* error weight vector              */
  sunrealtype err;  /* wrms error                       */
  sunrealtype ZERO = SUN_RCONST(0.0);
  sunrealtype ONE  = SUN_RCONST(1.0);

  /* create reference solution and error weight vectors */
  ref = N_VClone(y);
  ewt = N_VClone(y);

  /* set the reference solution data */
  NV_Ith_S(ref, 0) = SUN_RCONST(2.0833403356917897e-08);
  NV_Ith_S(ref, 1) = SUN_RCONST(8.1470714598028223e-14);
  NV_Ith_S(ref, 2) = SUN_RCONST(9.9999997916651040e-01);

  /* compute the error weight vector */
  N_VAbs(ref, ewt);
  N_VScale(rtol, ewt, ewt);
  N_VAddConst(ewt, atol, ewt);
  if (N_VMin(ewt) <= ZERO)
  {
    fprintf(stderr, "\nSUNDIALS_ERROR: check_ans failed - ewt <= 0\n\n");
    return (-1);
  }
  N_VInv(ewt, ewt);

  /* compute the solution error */
  N_VLinearSum(ONE, y, -ONE, ref, ref);
  err = N_VWrmsNorm(ref, ewt);

  /* is the solution within the tolerances? */
  passfail = (err < ONE) ? 0 : 1;

  if (passfail)
  {
    fprintf(stdout, "\nSUNDIALS_WARNING: check_ans error=%" GSYM "\n\n", err);
  }

  /* Free vectors */
  N_VDestroy(ref);
  N_VDestroy(ewt);

  return (passfail);
}

/*---- end of file ----*/
