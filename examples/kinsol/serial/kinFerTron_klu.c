/* -----------------------------------------------------------------
 * Programmer(s): Radu Serban @ LLNL
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
 * Example (serial):
 *
 * This example solves a nonlinear system from.
 *
 * Source: "Handbook of Test Problems in Local and Global Optimization",
 *             C.A. Floudas, P.M. Pardalos et al.
 *             Kluwer Academic Publishers, 1999.
 * Test problem 4 from Section 14.1, Chapter 14: Ferraris and Tronconi
 *
 * This problem involves a blend of trigonometric and exponential terms.
 *    0.5 sin(x1 x2) - 0.25 x2/pi - 0.5 x1 = 0
 *    (1-0.25/pi) ( exp(2 x1)-e ) + e x2 / pi - 2 e x1 = 0
 * such that
 *    0.25 <= x1 <=1.0
 *    1.5 <= x2 <= 2 pi
 *
 * The treatment of the bound constraints on x1 and x2 is done using
 * the additional variables
 *    l1 = x1 - x1_min >= 0
 *    L1 = x1 - x1_max <= 0
 *    l2 = x2 - x2_min >= 0
 *    L2 = x2 - x2_max >= 0
 *
 * and using the constraint feature in KINSOL to impose
 *    l1 >= 0    l2 >= 0
 *    L1 <= 0    L2 <= 0
 *
 * The Ferraris-Tronconi test problem has two known solutions.
 * The nonlinear system is solved by KINSOL using different
 * combinations of globalization and Jacobian update strategies
 * and with different initial guesses (leading to one or the other
 * of the known solutions).
 *
 * Constraints are imposed to make all components of the solution
 * positive.
 * -----------------------------------------------------------------
 */

#include <kinsol/kinsol.h> /* access to KINSOL func., consts. */
#include <math.h>
#include <nvector/nvector_serial.h> /* access to serial N_Vector       */
#include <stdio.h>
#include <stdlib.h>
#include <sundials/sundials_math.h>     /* access to SUNRexp               */
#include <sundials/sundials_types.h>    /* defs. of sunrealtype, sunindextype */
#include <sunlinsol/sunlinsol_klu.h>    /* access to KLU SUNLinearSolver   */
#include <sunmatrix/sunmatrix_sparse.h> /* access to sparse SUNMatrix      */

/* Problem Constants */

#define NVAR 2
#define NEQ  3 * NVAR

#define FTOL SUN_RCONST(1.e-5) /* function tolerance */
#define STOL SUN_RCONST(1.e-5) /* step tolerance     */

#define ZERO   SUN_RCONST(0.0)
#define PT25   SUN_RCONST(0.25)
#define PT5    SUN_RCONST(0.5)
#define ONE    SUN_RCONST(1.0)
#define ONEPT5 SUN_RCONST(1.5)
#define TWO    SUN_RCONST(2.0)

#define PI SUN_RCONST(3.1415926)
#define E  SUN_RCONST(2.7182818)

typedef struct
{
  sunrealtype lb[NVAR];
  sunrealtype ub[NVAR];
  int nnz;
}* UserData;

/* Functions Called by the KINSOL Solver */
static int func(N_Vector u, N_Vector f, void* user_data);
static int jac(N_Vector y, N_Vector f, SUNMatrix J, void* user_data,
               N_Vector tmp1, N_Vector tmp2);

/* Private Helper Functions */
static void SetInitialGuess1(N_Vector u, UserData data);
static void SetInitialGuess2(N_Vector u, UserData data);
static int SolveIt(void* kmem, N_Vector u, N_Vector s, int glstr, int mset);
static void PrintHeader(sunrealtype fnormtol, sunrealtype scsteptol);
static void PrintOutput(N_Vector u);
static void PrintFinalStats(void* kmem);
static int check_retval(void* retvalvalue, const char* funcname, int opt);

/*
 *--------------------------------------------------------------------
 * MAIN PROGRAM
 *--------------------------------------------------------------------
 */

int main(void)
{
  SUNContext sunctx;
  UserData data;
  sunrealtype fnormtol, scsteptol;
  N_Vector u1, u2, u, s, c;
  int glstr, mset, retval;
  void* kmem;
  SUNMatrix J;
  SUNLinearSolver LS;

  u1 = u2 = u = NULL;
  s = c = NULL;
  kmem  = NULL;
  J     = NULL;
  LS    = NULL;
  data  = NULL;

  /* User data */

  data        = (UserData)malloc(sizeof *data);
  data->lb[0] = PT25;
  data->ub[0] = ONE;
  data->lb[1] = ONEPT5;
  data->ub[1] = TWO * PI;
  data->nnz   = 12;

  /* Create the SUNDIALS context that all SUNDIALS objects require */
  retval = SUNContext_Create(SUN_COMM_NULL, &sunctx);
  if (check_retval(&retval, "SUNContext_Create", 1)) { return (1); }

  /* Create serial vectors of length NEQ */
  u1 = N_VNew_Serial(NEQ, sunctx);
  if (check_retval((void*)u1, "N_VNew_Serial", 0)) { return (1); }

  u2 = N_VNew_Serial(NEQ, sunctx);
  if (check_retval((void*)u2, "N_VNew_Serial", 0)) { return (1); }

  u = N_VNew_Serial(NEQ, sunctx);
  if (check_retval((void*)u, "N_VNew_Serial", 0)) { return (1); }

  s = N_VNew_Serial(NEQ, sunctx);
  if (check_retval((void*)s, "N_VNew_Serial", 0)) { return (1); }

  c = N_VNew_Serial(NEQ, sunctx);
  if (check_retval((void*)c, "N_VNew_Serial", 0)) { return (1); }

  SetInitialGuess1(u1, data);
  SetInitialGuess2(u2, data);

  N_VConst(ONE, s); /* no scaling */

  NV_Ith_S(c, 0) = ZERO; /* no constraint on x1 */
  NV_Ith_S(c, 1) = ZERO; /* no constraint on x2 */
  NV_Ith_S(c, 2) = ONE;  /* l1 = x1 - x1_min >= 0  */
  NV_Ith_S(c, 3) = -ONE; /* L1 = x1 - x1_max <= 0  */
  NV_Ith_S(c, 4) = ONE;  /* l2 = x2 - x2_min >= 0  */
  NV_Ith_S(c, 5) = -ONE; /* L2 = x2 - x22_min <= 0 */

  fnormtol  = FTOL;
  scsteptol = STOL;

  kmem = KINCreate(sunctx);
  if (check_retval((void*)kmem, "KINCreate", 0)) { return (1); }

  retval = KINSetUserData(kmem, data);
  if (check_retval(&retval, "KINSetUserData", 1)) { return (1); }
  retval = KINSetConstraints(kmem, c);
  if (check_retval(&retval, "KINSetConstraints", 1)) { return (1); }
  retval = KINSetFuncNormTol(kmem, fnormtol);
  if (check_retval(&retval, "KINSetFuncNormTol", 1)) { return (1); }
  retval = KINSetScaledStepTol(kmem, scsteptol);
  if (check_retval(&retval, "KINSetScaledStepTol", 1)) { return (1); }

  retval = KINInit(kmem, func, u);
  if (check_retval(&retval, "KINInit", 1)) { return (1); }

  /* Create sparse SUNMatrix */
  J = SUNSparseMatrix(NEQ, NEQ, data->nnz, CSR_MAT, sunctx);
  if (check_retval((void*)J, "SUNSparseMatrix", 0)) { return (1); }

  /* Create KLU solver object */
  LS = SUNLinSol_KLU(u, J, sunctx);
  if (check_retval((void*)LS, "SUNLinSol_KLU", 0)) { return (1); }

  /* Attach KLU linear solver */
  retval = KINSetLinearSolver(kmem, LS, J);
  if (check_retval(&retval, "KINSetLinearSolver", 1)) { return (1); }

  /* Set the Jacobian function */
  retval = KINSetJacFn(kmem, jac);
  if (check_retval(&retval, "KINSetJacFn", 1)) { return (1); }

  /* Print out the problem size, solution parameters, initial guess. */
  PrintHeader(fnormtol, scsteptol);

  /* --------------------------- */

  printf("\n------------------------------------------\n");
  printf("\nInitial guess on lower bounds\n");
  printf("  [x1,x2] = ");
  PrintOutput(u1);

  N_VScale(ONE, u1, u);
  glstr = KIN_NONE;
  mset  = 1;
  SolveIt(kmem, u, s, glstr, mset);

  /* --------------------------- */

  N_VScale(ONE, u1, u);
  glstr = KIN_LINESEARCH;
  mset  = 1;
  SolveIt(kmem, u, s, glstr, mset);

  /* --------------------------- */

  N_VScale(ONE, u1, u);
  glstr = KIN_NONE;
  mset  = 0;
  SolveIt(kmem, u, s, glstr, mset);

  /* --------------------------- */

  N_VScale(ONE, u1, u);
  glstr = KIN_LINESEARCH;
  mset  = 0;
  SolveIt(kmem, u, s, glstr, mset);

  /* --------------------------- */

  printf("\n------------------------------------------\n");
  printf("\nInitial guess in middle of feasible region\n");
  printf("  [x1,x2] = ");
  PrintOutput(u2);

  N_VScale(ONE, u2, u);
  glstr = KIN_NONE;
  mset  = 1;
  SolveIt(kmem, u, s, glstr, mset);

  /* --------------------------- */

  N_VScale(ONE, u2, u);
  glstr = KIN_LINESEARCH;
  mset  = 1;
  SolveIt(kmem, u, s, glstr, mset);

  /* --------------------------- */

  N_VScale(ONE, u2, u);
  glstr = KIN_NONE;
  mset  = 0;
  SolveIt(kmem, u, s, glstr, mset);

  /* --------------------------- */

  N_VScale(ONE, u2, u);
  glstr = KIN_LINESEARCH;
  mset  = 0;
  SolveIt(kmem, u, s, glstr, mset);

  /* Free memory */

  N_VDestroy(u1);
  N_VDestroy(u2);
  N_VDestroy(u);
  N_VDestroy(s);
  N_VDestroy(c);
  KINFree(&kmem);
  SUNLinSolFree(LS);
  SUNMatDestroy(J);
  free(data);
  SUNContext_Free(&sunctx);

  return (0);
}

static int SolveIt(void* kmem, N_Vector u, N_Vector s, int glstr, int mset)
{
  int retval;

  printf("\n");

  if (mset == 1) { printf("Exact Newton"); }
  else { printf("Modified Newton"); }

  if (glstr == KIN_NONE) { printf("\n"); }
  else { printf(" with line search\n"); }

  retval = KINSetMaxSetupCalls(kmem, mset);
  if (check_retval(&retval, "KINSetMaxSetupCalls", 1)) { return (1); }

  retval = KINSol(kmem, u, glstr, s, s);
  if (check_retval(&retval, "KINSol", 1)) { return (1); }

  printf("Solution:\n  [x1,x2] = ");
  PrintOutput(u);

  PrintFinalStats(kmem);

  return (0);
}

/*
 *--------------------------------------------------------------------
 * FUNCTIONS CALLED BY KINSOL
 *--------------------------------------------------------------------
 */

/*
 * System function for predator-prey system
 */

static int func(N_Vector u, N_Vector f, void* user_data)
{
  sunrealtype *udata, *fdata;
  sunrealtype x1, l1, L1, x2, l2, L2;
  sunrealtype *lb, *ub;
  UserData params;

  params = (UserData)user_data;
  lb     = params->lb;
  ub     = params->ub;

  udata = N_VGetArrayPointer(u);
  fdata = N_VGetArrayPointer(f);

  x1 = udata[0];
  x2 = udata[1];
  l1 = udata[2];
  L1 = udata[3];
  l2 = udata[4];
  L2 = udata[5];

  fdata[0] = PT5 * sin(x1 * x2) - PT25 * x2 / PI - PT5 * x1;
  fdata[1] = (ONE - PT25 / PI) * (SUNRexp(TWO * x1) - E) + E * x2 / PI -
             TWO * E * x1;
  fdata[2] = l1 - x1 + lb[0];
  fdata[3] = L1 - x1 + ub[0];
  fdata[4] = l2 - x2 + lb[1];
  fdata[5] = L2 - x2 + ub[1];

  return (0);
}

/*
 * System Jacobian
 */

static int jac(N_Vector y, N_Vector f, SUNMatrix J, void* user_data,
               N_Vector tmp1, N_Vector tmp2)
{
  sunrealtype* yd;
  sunindextype* rowptrs = SUNSparseMatrix_IndexPointers(J);
  sunindextype* colvals = SUNSparseMatrix_IndexValues(J);
  sunrealtype* data     = SUNSparseMatrix_Data(J);

  yd = N_VGetArrayPointer(y);

  SUNMatZero(J);

  rowptrs[0] = 0;
  rowptrs[1] = 2;
  rowptrs[2] = 4;
  rowptrs[3] = 6;
  rowptrs[4] = 8;
  rowptrs[5] = 10;
  rowptrs[6] = 12;

  /* row 0: J(0,0) and J(0,1) */
  data[0]    = PT5 * cos(yd[0] * yd[1]) * yd[1] - PT5;
  colvals[0] = 0;
  data[1]    = PT5 * cos(yd[0] * yd[1]) * yd[0] - PT25 / PI;
  colvals[1] = 1;

  /* row 1: J(1,0) and J(1,1) */
  data[2]    = TWO * (ONE - PT25 / PI) * (SUNRexp(TWO * yd[0]) - E);
  colvals[2] = 0;
  data[3]    = E / PI;
  colvals[3] = 1;

  /* row 2: J(2,0) and J(2,2) */
  data[4]    = -ONE;
  colvals[4] = 0;
  data[5]    = ONE;
  colvals[5] = 2;

  /* row 3: J(3,0) and J(3,3) */
  data[6]    = -ONE;
  colvals[6] = 0;
  data[7]    = ONE;
  colvals[7] = 3;

  /* row 4: J(4,1) and J(4,4) */
  data[8]    = -ONE;
  colvals[8] = 1;
  data[9]    = ONE;
  colvals[9] = 4;

  /* row 5: J(5,1) and J(5,5) */
  data[10]    = -ONE;
  colvals[10] = 1;
  data[11]    = ONE;
  colvals[11] = 5;

  return (0);
}

/*
 *--------------------------------------------------------------------
 * PRIVATE FUNCTIONS
 *--------------------------------------------------------------------
 */

/*
 * Initial guesses
 */

static void SetInitialGuess1(N_Vector u, UserData data)
{
  sunrealtype x1, x2;
  sunrealtype* udata;
  sunrealtype *lb, *ub;

  udata = N_VGetArrayPointer(u);

  lb = data->lb;
  ub = data->ub;

  /* There are two known solutions for this problem */

  /* this init. guess should take us to (0.29945; 2.83693) */
  x1 = lb[0];
  x2 = lb[1];

  udata[0] = x1;
  udata[1] = x2;
  udata[2] = x1 - lb[0];
  udata[3] = x1 - ub[0];
  udata[4] = x2 - lb[1];
  udata[5] = x2 - ub[1];
}

static void SetInitialGuess2(N_Vector u, UserData data)
{
  sunrealtype x1, x2;
  sunrealtype* udata;
  sunrealtype *lb, *ub;

  udata = N_VGetArrayPointer(u);

  lb = data->lb;
  ub = data->ub;

  /* There are two known solutions for this problem */

  /* this init. guess should take us to (0.5; 3.1415926) */
  x1 = PT5 * (lb[0] + ub[0]);
  x2 = PT5 * (lb[1] + ub[1]);

  udata[0] = x1;
  udata[1] = x2;
  udata[2] = x1 - lb[0];
  udata[3] = x1 - ub[0];
  udata[4] = x2 - lb[1];
  udata[5] = x2 - ub[1];
}

/*
 * Print first lines of output (problem description)
 */

static void PrintHeader(sunrealtype fnormtol, sunrealtype scsteptol)
{
  printf("\nFerraris and Tronconi test problem\n");
  printf("Tolerance parameters:\n");
#if defined(SUNDIALS_EXTENDED_PRECISION)
  printf("  fnormtol  = %10.6Lg\n  scsteptol = %10.6Lg\n", fnormtol, scsteptol);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
  printf("  fnormtol  = %10.6g\n  scsteptol = %10.6g\n", fnormtol, scsteptol);
#else
  printf("  fnormtol  = %10.6g\n  scsteptol = %10.6g\n", fnormtol, scsteptol);
#endif
}

/*
 * Print solution
 */

static void PrintOutput(N_Vector u)
{
#if defined(SUNDIALS_EXTENDED_PRECISION)
  printf(" %8.6Lg  %8.6Lg\n", NV_Ith_S(u, 0), NV_Ith_S(u, 1));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
  printf(" %8.6g  %8.6g\n", NV_Ith_S(u, 0), NV_Ith_S(u, 1));
#else
  printf(" %8.6g  %8.6g\n", NV_Ith_S(u, 0), NV_Ith_S(u, 1));
#endif
}

/*
 * Print final statistics contained in iopt
 */

static void PrintFinalStats(void* kmem)
{
  long int nni, nfe, nje;
  int retval;

  retval = KINGetNumNonlinSolvIters(kmem, &nni);
  check_retval(&retval, "KINGetNumNonlinSolvIters", 1);
  retval = KINGetNumFuncEvals(kmem, &nfe);
  check_retval(&retval, "KINGetNumFuncEvals", 1);
  retval = KINGetNumJacEvals(kmem, &nje);
  check_retval(&retval, "KINGetNumJacEvals", 1);

  printf("Final Statistics:\n");
  printf("  nni = %5ld    nfe  = %5ld \n", nni, nfe);
  printf("  nje = %5ld    \n", nje);
}

/*
 * Check function return value...
 *    opt == 0 means SUNDIALS function allocates memory so check if
 *             returned NULL pointer
 *    opt == 1 means SUNDIALS function returns a retval so check if
 *             retval >= 0
 *    opt == 2 means function allocates memory so check if returned
 *             NULL pointer
 */

static int check_retval(void* retvalvalue, const char* funcname, int opt)
{
  int* errretval;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && retvalvalue == NULL)
  {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return (1);
  }

  /* Check if retval < 0 */
  else if (opt == 1)
  {
    errretval = (int*)retvalvalue;
    if (*errretval < 0)
    {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with retval = %d\n\n",
              funcname, *errretval);
      return (1);
    }
  }

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && retvalvalue == NULL)
  {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return (1);
  }

  return (0);
}
