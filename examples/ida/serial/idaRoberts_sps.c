/* -----------------------------------------------------------------
 * Programmer(s): Carol Woodward @ LLNL  (based on idaRoberts_dns.c)
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
 * This simple example problem for IDA, due to Robertson,
 * is from chemical kinetics, and consists of the following three
 * equations:
 *
 *      dy1/dt = -.04*y1 + 1.e4*y2*y3
 *      dy2/dt = .04*y1 - 1.e4*y2*y3 - 3.e7*y2**2
 *         0   = y1 + y2 + y3 - 1
 *
 * on the interval from t = 0.0 to t = 4.e10, with initial
 * conditions: y1 = 1, y2 = y3 = 0.
 *
 * While integrating the system, we also use the rootfinding
 * feature to find the points at which y1 = 1e-4 or at which
 * y3 = 0.01.
 *
 * The problem is solved with IDA using the SUPERLUMT linear
 * solver, with a user-supplied Jacobian. Output is printed at
 * t = .4, 4, 40, ..., 4e10.
 * -----------------------------------------------------------------*/

#include <ida/ida.h> /* prototypes for IDA fcts., consts.    */
#include <math.h>
#include <nvector/nvector_serial.h> /* access to serial N_Vector            */
#include <stdio.h>
#include <sundials/sundials_math.h> /* defs. of SUNRabs, SUNRexp, etc.      */
#include <sundials/sundials_types.h> /* defs. of sunrealtype, sunindextype      */
#include <sunlinsol/sunlinsol_superlumt.h> /* access to SuperLUMT linear solver    */
#include <sunmatrix/sunmatrix_sparse.h> /* access to sparse SUNMatrix           */

/* Problem Constants */

#define NEQ  3
#define NOUT 12

#define ZERO SUN_RCONST(0.0)
#define ONE  SUN_RCONST(1.0)

/* Prototypes of functions called by IDA */

int resrob(sunrealtype tres, N_Vector yy, N_Vector yp, N_Vector resval,
           void* user_data);

static int grob(sunrealtype t, N_Vector yy, N_Vector yp, sunrealtype* gout,
                void* user_data);

int jacrob(sunrealtype tt, sunrealtype cj, N_Vector yy, N_Vector yp,
           N_Vector resvec, SUNMatrix JJ, void* user_data, N_Vector tempv1,
           N_Vector tempv2, N_Vector tempv3);

/* Prototypes of private functions */
static void PrintHeader(sunrealtype rtol, N_Vector avtol, N_Vector y);
static void PrintOutput(void* mem, sunrealtype t, N_Vector y);
static void PrintRootInfo(int root_f1, int root_f2);
static void PrintFinalStats(void* mem);
static int check_retval(void* returnvalue, const char* funcname, int opt);

/*
 *--------------------------------------------------------------------
 * Main Program
 *--------------------------------------------------------------------
 */

int main(void)
{
  void* mem;
  N_Vector yy, yp, avtol;
  sunrealtype rtol, *yval, *ypval, *atval;
  sunrealtype t0, tout1, tout, tret;
  int iout, retval, retvalr;
  int rootsfound[2];
  SUNMatrix A;
  SUNLinearSolver LS;
  sunindextype nnz;
  SUNContext ctx;

  mem = NULL;
  yy = yp = avtol = NULL;
  yval = ypval = atval = NULL;
  A                    = NULL;
  LS                   = NULL;

  /* Create the SUNDIALS context object for this simulation */
  retval = SUNContext_Create(SUN_COMM_NULL, &ctx);
  if (check_retval(&retval, "SUNContext_Create", 1)) { return 1; }

  /* Allocate N-vectors. */
  yy = N_VNew_Serial(NEQ, ctx);
  if (check_retval((void*)yy, "N_VNew_Serial", 0)) { return (1); }
  yp = N_VClone(yy);
  if (check_retval((void*)yp, "N_VNew_Serial", 0)) { return (1); }
  avtol = N_VClone(yy);
  if (check_retval((void*)avtol, "N_VNew_Serial", 0)) { return (1); }

  /* Create and initialize  y, y', and absolute tolerance vectors. */
  yval    = N_VGetArrayPointer(yy);
  yval[0] = ONE;
  yval[1] = ZERO;
  yval[2] = ZERO;

  ypval    = N_VGetArrayPointer(yp);
  ypval[0] = SUN_RCONST(-0.04);
  ypval[1] = SUN_RCONST(0.04);
  ypval[2] = ZERO;

  rtol = SUN_RCONST(1.0e-4);

  atval    = N_VGetArrayPointer(avtol);
  atval[0] = SUN_RCONST(1.0e-8);
  atval[1] = SUN_RCONST(1.0e-6);
  atval[2] = SUN_RCONST(1.0e-6);

  /* Integration limits */
  t0    = ZERO;
  tout1 = SUN_RCONST(0.4);

  PrintHeader(rtol, avtol, yy);

  /* Call IDACreate and IDAInit to initialize IDA memory */
  mem = IDACreate(ctx);
  if (check_retval((void*)mem, "IDACreate", 0)) { return (1); }
  retval = IDAInit(mem, resrob, t0, yy, yp);
  if (check_retval(&retval, "IDAInit", 1)) { return (1); }
  /* Call IDASVtolerances to set tolerances */
  retval = IDASVtolerances(mem, rtol, avtol);
  if (check_retval(&retval, "IDASVtolerances", 1)) { return (1); }

  /* Free avtol */
  N_VDestroy(avtol);

  /* Call IDARootInit to specify the root function grob with 2 components */
  retval = IDARootInit(mem, 2, grob);
  if (check_retval(&retval, "IDARootInit", 1)) { return (1); }

  /* Create sparse SUNMatrix for use in linear solves */
  nnz = NEQ * NEQ;
  A   = SUNSparseMatrix(NEQ, NEQ, nnz, CSC_MAT, ctx);
  if (check_retval((void*)A, "SUNSparseMatrix", 0)) { return (1); }

  /* Create SuperLUMT SUNLinearSolver object (one thread) */
  LS = SUNLinSol_SuperLUMT(yy, A, 1, ctx);
  if (check_retval((void*)LS, "SUNLinSol_SuperLUMT", 0)) { return (1); }

  /* Attach the matrix and linear solver */
  retval = IDASetLinearSolver(mem, LS, A);
  if (check_retval(&retval, "IDASetLinearSolver", 1)) { return (1); }

  /* Set the user-supplied Jacobian routine */
  retval = IDASetJacFn(mem, jacrob);
  if (check_retval(&retval, "IDASetJacFn", 1)) { return (1); }

  /* In loop, call IDASolve, print results, and test for error.
     Break out of loop when NOUT preset output times have been reached. */

  iout = 0;
  tout = tout1;
  while (1)
  {
    retval = IDASolve(mem, tout, &tret, yy, yp, IDA_NORMAL);

    PrintOutput(mem, tret, yy);

    if (check_retval(&retval, "IDASolve", 1)) { return (1); }

    if (retval == IDA_ROOT_RETURN)
    {
      retvalr = IDAGetRootInfo(mem, rootsfound);
      check_retval(&retvalr, "IDAGetRootInfo", 1);
      PrintRootInfo(rootsfound[0], rootsfound[1]);
    }

    if (retval == IDA_SUCCESS)
    {
      iout++;
      tout *= SUN_RCONST(10.0);
    }

    if (iout == NOUT) { break; }
  }

  PrintFinalStats(mem);

  /* Free memory */

  IDAFree(&mem);
  SUNLinSolFree(LS);
  SUNMatDestroy(A);
  N_VDestroy(yy);
  N_VDestroy(yp);
  SUNContext_Free(&ctx);

  return (0);
}

/*
 *--------------------------------------------------------------------
 * Functions called by IDA
 *--------------------------------------------------------------------
 */

/*
 * Define the system residual function.
 */

int resrob(sunrealtype tres, N_Vector yy, N_Vector yp, N_Vector rr,
           void* user_data)
{
  sunrealtype *yval, *ypval, *rval;

  yval  = N_VGetArrayPointer(yy);
  ypval = N_VGetArrayPointer(yp);
  rval  = N_VGetArrayPointer(rr);

  rval[0] = SUN_RCONST(-0.04) * yval[0] + SUN_RCONST(1.0e4) * yval[1] * yval[2];
  rval[1] = -rval[0] - SUN_RCONST(3.0e7) * yval[1] * yval[1] - ypval[1];
  rval[0] -= ypval[0];
  rval[2] = yval[0] + yval[1] + yval[2] - ONE;

  return (0);
}

/*
 * Root function routine. Compute functions g_i(t,y) for i = 0,1.
 */

static int grob(sunrealtype t, N_Vector yy, N_Vector yp, sunrealtype* gout,
                void* user_data)
{
  sunrealtype *yval, y1, y3;

  yval    = N_VGetArrayPointer(yy);
  y1      = yval[0];
  y3      = yval[2];
  gout[0] = y1 - SUN_RCONST(0.0001);
  gout[1] = y3 - SUN_RCONST(0.01);

  return (0);
}

/*
 * Define the Jacobian function.
 */

int jacrob(sunrealtype tt, sunrealtype cj, N_Vector yy, N_Vector yp,
           N_Vector resvec, SUNMatrix JJ, void* user_data, N_Vector tempv1,
           N_Vector tempv2, N_Vector tempv3)
{
  sunrealtype* yval;
  sunindextype* colptrs = SUNSparseMatrix_IndexPointers(JJ);
  sunindextype* rowvals = SUNSparseMatrix_IndexValues(JJ);
  sunrealtype* data     = SUNSparseMatrix_Data(JJ);

  yval = N_VGetArrayPointer(yy);

  SUNMatZero(JJ);

  colptrs[0] = 0;
  colptrs[1] = 3;
  colptrs[2] = 6;
  colptrs[3] = 9;

  /* column 0 */
  data[0]    = SUN_RCONST(-0.04) - cj;
  rowvals[0] = 0;
  data[1]    = SUN_RCONST(0.04);
  rowvals[1] = 1;
  data[2]    = ONE;
  rowvals[2] = 2;

  /* column 1 */
  data[3]    = SUN_RCONST(1.0e4) * yval[2];
  rowvals[3] = 0;
  data[4] = (SUN_RCONST(-1.0e4) * yval[2]) - (SUN_RCONST(6.0e7) * yval[1]) - cj;
  rowvals[4] = 1;
  data[5]    = ONE;
  rowvals[5] = 2;

  /* column 2 */
  data[6]    = SUN_RCONST(1.0e4) * yval[1];
  rowvals[6] = 0;
  data[7]    = SUN_RCONST(-1.0e4) * yval[1];
  rowvals[7] = 1;
  data[8]    = ONE;
  rowvals[8] = 2;

  return (0);
}

/*
 *--------------------------------------------------------------------
 * Private functions
 *--------------------------------------------------------------------
 */

/*
 * Print first lines of output (problem description)
 */

static void PrintHeader(sunrealtype rtol, N_Vector avtol, N_Vector y)
{
  sunrealtype *atval, *yval;

  atval = N_VGetArrayPointer(avtol);
  yval  = N_VGetArrayPointer(y);

  printf("\nidaRoberts_sps: Robertson kinetics DAE serial example problem for "
         "IDA.\n");
  printf("               Three equation chemical kinetics problem.\n\n");
  printf("Linear solver: SUPERLUMT, with user-supplied Jacobian.\n");
#if defined(SUNDIALS_EXTENDED_PRECISION)
  printf("Tolerance parameters:  rtol = %Lg   atol = %Lg %Lg %Lg \n", rtol,
         atval[0], atval[1], atval[2]);
  printf("Initial conditions y0 = (%Lg %Lg %Lg)\n", yval[0], yval[1], yval[2]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
  printf("Tolerance parameters:  rtol = %g   atol = %g %g %g \n", rtol,
         atval[0], atval[1], atval[2]);
  printf("Initial conditions y0 = (%g %g %g)\n", yval[0], yval[1], yval[2]);
#else
  printf("Tolerance parameters:  rtol = %g   atol = %g %g %g \n", rtol,
         atval[0], atval[1], atval[2]);
  printf("Initial conditions y0 = (%g %g %g)\n", yval[0], yval[1], yval[2]);
#endif
  printf("Constraints and id not used.\n\n");
  printf("---------------------------------------------------------------------"
         "--\n");
  printf("  t             y1           y2           y3");
  printf("      | nst  k      h\n");
  printf("---------------------------------------------------------------------"
         "--\n");
}

/*
 * Print Output
 */

static void PrintOutput(void* mem, sunrealtype t, N_Vector y)
{
  sunrealtype* yval;
  int retval, kused;
  long int nst;
  sunrealtype hused;

  yval = N_VGetArrayPointer(y);

  retval = IDAGetLastOrder(mem, &kused);
  check_retval(&retval, "IDAGetLastOrder", 1);
  retval = IDAGetNumSteps(mem, &nst);
  check_retval(&retval, "IDAGetNumSteps", 1);
  retval = IDAGetLastStep(mem, &hused);
  check_retval(&retval, "IDAGetLastStep", 1);
#if defined(SUNDIALS_EXTENDED_PRECISION)
  printf("%10.4Le %12.4Le %12.4Le %12.4Le | %3ld  %1d %12.4Le\n", t, yval[0],
         yval[1], yval[2], nst, kused, hused);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
  printf("%10.4e %12.4e %12.4e %12.4e | %3ld  %1d %12.4e\n", t, yval[0],
         yval[1], yval[2], nst, kused, hused);
#else
  printf("%10.4e %12.4e %12.4e %12.4e | %3ld  %1d %12.4e\n", t, yval[0],
         yval[1], yval[2], nst, kused, hused);
#endif
}

static void PrintRootInfo(int root_f1, int root_f2)
{
  printf("    rootsfound[] = %3d %3d\n", root_f1, root_f2);
  return;
}

/*
 * Print final integrator statistics
 */

static void PrintFinalStats(void* mem)
{
  int retval;
  long int nst, nni, nnf, nje, nre, netf, ncfn, nge;

  retval = IDAGetNumSteps(mem, &nst);
  check_retval(&retval, "IDAGetNumSteps", 1);
  retval = IDAGetNumResEvals(mem, &nre);
  check_retval(&retval, "IDAGetNumResEvals", 1);
  retval = IDAGetNumJacEvals(mem, &nje);
  check_retval(&retval, "IDAGetNumJacEvals", 1);
  retval = IDAGetNumNonlinSolvIters(mem, &nni);
  check_retval(&retval, "IDAGetNumNonlinSolvIters", 1);
  retval = IDAGetNumErrTestFails(mem, &netf);
  check_retval(&retval, "IDAGetNumErrTestFails", 1);
  retval = IDAGetNumNonlinSolvConvFails(mem, &nnf);
  check_retval(&retval, "IDAGetNumNonlinSolvConvFails", 1);
  retval = IDAGetNumStepSolveFails(mem, &ncfn);
  check_retval(&retval, "IDAGetNumStepSolveFails", 1);
  retval = IDAGetNumGEvals(mem, &nge);
  check_retval(&retval, "IDAGetNumGEvals", 1);

  printf("\nFinal Run Statistics: \n\n");
  printf("Number of steps                    = %ld\n", nst);
  printf("Number of residual evaluations     = %ld\n", nre);
  printf("Number of Jacobian evaluations     = %ld\n", nje);
  printf("Number of nonlinear iterations     = %ld\n", nni);
  printf("Number of error test failures      = %ld\n", netf);
  printf("Number of nonlinear conv. failures = %ld\n", nnf);
  printf("Number of step solver failures     = %ld\n", ncfn);
  printf("Number of root fn. evaluations     = %ld\n", nge);
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

static int check_retval(void* returnvalue, const char* funcname, int opt)
{
  int* retval;
  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && returnvalue == NULL)
  {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return (1);
  }
  else if (opt == 1)
  {
    /* Check if retval < 0 */
    retval = (int*)returnvalue;
    if (*retval < 0)
    {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with retval = %d\n\n",
              funcname, *retval);
      return (1);
    }
  }
  else if (opt == 2 && returnvalue == NULL)
  {
    /* Check if function returned NULL pointer - no memory allocated */
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return (1);
  }

  return (0);
}
