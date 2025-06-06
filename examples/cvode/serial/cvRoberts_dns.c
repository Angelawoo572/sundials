/* -----------------------------------------------------------------
 * Programmer(s): Scott D. Cohen, Alan C. Hindmarsh and
 *                Radu Serban @ LLNL
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
 * The following is a simple example problem, with the coding
 * needed for its solution by CVODE. The problem is from
 * chemical kinetics, and consists of the following three rate
 * equations:
 *    dy1/dt = -.04*y1 + 1.e4*y2*y3
 *    dy2/dt = .04*y1 - 1.e4*y2*y3 - 3.e7*(y2)^2
 *    dy3/dt = 3.e7*(y2)^2
 * on the interval from t = 0.0 to t = 4.e10, with initial
 * conditions: y1 = 1.0, y2 = y3 = 0. The problem is stiff.
 * While integrating the system, we also use the rootfinding
 * feature to find the points at which y1 = 1e-4 or at which
 * y3 = 0.01. This program solves the problem with the BDF method,
 * Newton iteration with the dense linear solver, and a
 * user-supplied Jacobian routine.
 * It uses a scalar relative tolerance and a vector absolute
 * tolerance. Output is printed in decades from t = .4 to t = 4.e10.
 * Run statistics (optional outputs) are printed at the end.
 * -----------------------------------------------------------------*/

#include <cvode/cvode.h>            /* prototypes for CVODE fcts., consts.  */
#include <nvector/nvector_serial.h> /* access to serial N_Vector            */
#include <stdio.h>
#include <sunlinsol/sunlinsol_dense.h> /* access to dense SUNLinearSolver      */
#include <sunmatrix/sunmatrix_dense.h> /* access to dense SUNMatrix            */

#if defined(SUNDIALS_EXTENDED_PRECISION)
#define GSYM "Lg"
#else
#define GSYM "g"
#endif

/* User-defined vector and matrix accessor macros: Ith, IJth */

/* These macros are defined in order to write code which exactly matches
   the mathematical problem description given above.

   Ith(v,i) references the ith component of the vector v, where i is in
   the range [1..NEQ] and NEQ is defined below. The Ith macro is defined
   using the N_VIth macro in nvector.h. N_VIth numbers the components of
   a vector starting from 0.

   IJth(A,i,j) references the (i,j)th element of the dense matrix A, where
   i and j are in the range [1..NEQ]. The IJth macro is defined using the
   SM_ELEMENT_D macro. SM_ELEMENT_D numbers rows and columns of
   a dense matrix starting from 0. */

#define Ith(v, i) NV_Ith_S(v, i - 1) /* i-th vector component i=1..NEQ */
#define IJth(A, i, j) \
  SM_ELEMENT_D(A, i - 1, j - 1) /* (i,j)-th matrix component i,j=1..NEQ */

/* Problem Constants */

#define NEQ   3               /* number of equations  */
#define Y1    SUN_RCONST(1.0) /* initial y components */
#define Y2    SUN_RCONST(0.0)
#define Y3    SUN_RCONST(0.0)
#define RTOL  SUN_RCONST(1.0e-4) /* scalar relative tolerance            */
#define ATOL1 SUN_RCONST(1.0e-8) /* vector absolute tolerance components */
#define ATOL2 SUN_RCONST(1.0e-14)
#define ATOL3 SUN_RCONST(1.0e-6)
#define T0    SUN_RCONST(0.0)  /* initial time           */
#define T1    SUN_RCONST(0.4)  /* first output time      */
#define TMULT SUN_RCONST(10.0) /* output time factor     */
#define NOUT  12               /* number of output times */

#define ZERO SUN_RCONST(0.0)

/* Functions Called by the Solver */

static int f(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data);

static int g(sunrealtype t, N_Vector y, sunrealtype* gout, void* user_data);

static int Jac(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix J,
               void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

/* Private functions to output results */

static void PrintOutput(sunrealtype t, sunrealtype y1, sunrealtype y2,
                        sunrealtype y3);
static void PrintRootInfo(int root_f1, int root_f2);

/* Private function to check function return values */

static int check_retval(void* returnvalue, const char* funcname, int opt);

/* Private function to check computed solution */

static int check_ans(N_Vector y, sunrealtype t, sunrealtype rtol, N_Vector atol);

/*
 *-------------------------------
 * Main Program
 *-------------------------------
 */

int main(void)
{
  SUNContext sunctx;
  sunrealtype t, tout;
  N_Vector y;
  N_Vector abstol;
  SUNMatrix A;
  SUNLinearSolver LS;
  void* cvode_mem;
  int retval, iout;
  int retvalr;
  int rootsfound[2];
  FILE* FID;

  y         = NULL;
  abstol    = NULL;
  A         = NULL;
  LS        = NULL;
  cvode_mem = NULL;

  /* Create the SUNDIALS context */
  retval = SUNContext_Create(SUN_COMM_NULL, &sunctx);
  if (check_retval(&retval, "SUNContext_Create", 1)) { return (1); }

  /* Initial conditions */
  y = N_VNew_Serial(NEQ, sunctx);
  if (check_retval((void*)y, "N_VNew_Serial", 0)) { return (1); }

  /* Initialize y */
  Ith(y, 1) = Y1;
  Ith(y, 2) = Y2;
  Ith(y, 3) = Y3;

  /* Set the vector absolute tolerance */
  abstol = N_VNew_Serial(NEQ, sunctx);
  if (check_retval((void*)abstol, "N_VNew_Serial", 0)) { return (1); }

  Ith(abstol, 1) = ATOL1;
  Ith(abstol, 2) = ATOL2;
  Ith(abstol, 3) = ATOL3;

  /* Call CVodeCreate to create the solver memory and specify the
   * Backward Differentiation Formula */
  cvode_mem = CVodeCreate(CV_BDF, sunctx);
  if (check_retval((void*)cvode_mem, "CVodeCreate", 0)) { return (1); }

  /* Call CVodeInit to initialize the integrator memory and specify the
   * user's right hand side function in y'=f(t,y), the initial time T0, and
   * the initial dependent variable vector y. */
  retval = CVodeInit(cvode_mem, f, T0, y);
  if (check_retval(&retval, "CVodeInit", 1)) { return (1); }

  /* Call CVodeSVtolerances to specify the scalar relative tolerance
   * and vector absolute tolerances */
  retval = CVodeSVtolerances(cvode_mem, RTOL, abstol);
  if (check_retval(&retval, "CVodeSVtolerances", 1)) { return (1); }

  /* Call CVodeRootInit to specify the root function g with 2 components */
  retval = CVodeRootInit(cvode_mem, 2, g);
  if (check_retval(&retval, "CVodeRootInit", 1)) { return (1); }

  /* Create dense SUNMatrix for use in linear solves */
  A = SUNDenseMatrix(NEQ, NEQ, sunctx);
  if (check_retval((void*)A, "SUNDenseMatrix", 0)) { return (1); }

  /* Create dense SUNLinearSolver object for use by CVode */
  LS = SUNLinSol_Dense(y, A, sunctx);
  if (check_retval((void*)LS, "SUNLinSol_Dense", 0)) { return (1); }

  /* Attach the matrix and linear solver */
  retval = CVodeSetLinearSolver(cvode_mem, LS, A);
  if (check_retval(&retval, "CVodeSetLinearSolver", 1)) { return (1); }

  /* Set the user-supplied Jacobian routine Jac */
  retval = CVodeSetJacFn(cvode_mem, Jac);
  if (check_retval(&retval, "CVodeSetJacFn", 1)) { return (1); }

  /* In loop, call CVode, print results, and test for error.
     Break out of loop when NOUT preset output times have been reached.  */
  printf(" \n3-species kinetics problem\n\n");

  /* Open file for printing statistics */
  FID = fopen("cvRoberts_dns_stats.csv", "w");

  iout = 0;
  tout = T1;
  while (1)
  {
    retval = CVode(cvode_mem, tout, y, &t, CV_NORMAL);
    PrintOutput(t, Ith(y, 1), Ith(y, 2), Ith(y, 3));

    if (retval == CV_ROOT_RETURN)
    {
      retvalr = CVodeGetRootInfo(cvode_mem, rootsfound);
      if (check_retval(&retvalr, "CVodeGetRootInfo", 1)) { return (1); }
      PrintRootInfo(rootsfound[0], rootsfound[1]);
    }

    if (check_retval(&retval, "CVode", 1)) { break; }
    if (retval == CV_SUCCESS)
    {
      iout++;
      tout *= TMULT;
    }

    retval = CVodePrintAllStats(cvode_mem, FID, SUN_OUTPUTFORMAT_CSV);

    if (iout == NOUT) { break; }
  }
  fclose(FID);

  /* Print final statistics to the screen */
  printf("\nFinal Statistics:\n");
  retval = CVodePrintAllStats(cvode_mem, stdout, SUN_OUTPUTFORMAT_TABLE);

  /* check the solution error */
  retval = check_ans(y, t, RTOL, abstol);

  /* Free memory */
  N_VDestroy(y);            /* Free y vector */
  N_VDestroy(abstol);       /* Free abstol vector */
  CVodeFree(&cvode_mem);    /* Free CVODE memory */
  SUNLinSolFree(LS);        /* Free the linear solver memory */
  SUNMatDestroy(A);         /* Free the matrix memory */
  SUNContext_Free(&sunctx); /* Free the SUNDIALS context */

  return (retval);
}

/*
 *-------------------------------
 * Functions called by the solver
 *-------------------------------
 */

/*
 * f routine. Compute function f(t,y).
 */

static int f(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data)
{
  sunrealtype y1, y2, y3, yd1, yd3;

  y1 = Ith(y, 1);
  y2 = Ith(y, 2);
  y3 = Ith(y, 3);

  yd1 = Ith(ydot, 1) = SUN_RCONST(-0.04) * y1 + SUN_RCONST(1.0e4) * y2 * y3;
  yd3 = Ith(ydot, 3) = SUN_RCONST(3.0e7) * y2 * y2;
  Ith(ydot, 2)       = -yd1 - yd3;

  return (0);
}

/*
 * g routine. Compute functions g_i(t,y) for i = 0,1.
 */

static int g(sunrealtype t, N_Vector y, sunrealtype* gout, void* user_data)
{
  sunrealtype y1, y3;

  y1      = Ith(y, 1);
  y3      = Ith(y, 3);
  gout[0] = y1 - SUN_RCONST(0.0001);
  gout[1] = y3 - SUN_RCONST(0.01);

  return (0);
}

/*
 * Jacobian routine. Compute J(t,y) = df/dy. *
 */

static int Jac(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix J,
               void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  sunrealtype y2, y3;

  y2 = Ith(y, 2);
  y3 = Ith(y, 3);

  IJth(J, 1, 1) = SUN_RCONST(-0.04);
  IJth(J, 1, 2) = SUN_RCONST(1.0e4) * y3;
  IJth(J, 1, 3) = SUN_RCONST(1.0e4) * y2;

  IJth(J, 2, 1) = SUN_RCONST(0.04);
  IJth(J, 2, 2) = SUN_RCONST(-1.0e4) * y3 - SUN_RCONST(6.0e7) * y2;
  IJth(J, 2, 3) = SUN_RCONST(-1.0e4) * y2;

  IJth(J, 3, 1) = ZERO;
  IJth(J, 3, 2) = SUN_RCONST(6.0e7) * y2;
  IJth(J, 3, 3) = ZERO;

  return (0);
}

/*
 *-------------------------------
 * Private helper functions
 *-------------------------------
 */

static void PrintOutput(sunrealtype t, sunrealtype y1, sunrealtype y2,
                        sunrealtype y3)
{
#if defined(SUNDIALS_EXTENDED_PRECISION)
  printf("At t = %0.4Le      y =%14.6Le  %14.6Le  %14.6Le\n", t, y1, y2, y3);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
  printf("At t = %0.4e      y =%14.6e  %14.6e  %14.6e\n", t, y1, y2, y3);
#else
  printf("At t = %0.4e      y =%14.6e  %14.6e  %14.6e\n", t, y1, y2, y3);
#endif

  return;
}

static void PrintRootInfo(int root_f1, int root_f2)
{
  printf("    rootsfound[] = %3d %3d\n", root_f1, root_f2);

  return;
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

  /* Check if retval < 0 */
  else if (opt == 1)
  {
    retval = (int*)returnvalue;
    if (*retval < 0)
    {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with retval = %d\n\n",
              funcname, *retval);
      return (1);
    }
  }

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && returnvalue == NULL)
  {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return (1);
  }

  return (0);
}

/* compare the solution at the final time 4e10s to a reference solution computed
   using a relative tolerance of 1e-8 and absolute tolerance of 1e-14 */
static int check_ans(N_Vector y, sunrealtype t, sunrealtype rtol, N_Vector atol)
{
  int passfail = 0; /* answer pass (0) or fail (1) flag */
  N_Vector ref;     /* reference solution vector        */
  N_Vector ewt;     /* error weight vector              */
  sunrealtype err;  /* wrms error                       */
  sunrealtype ONE = SUN_RCONST(1.0);

  /* create reference solution and error weight vectors */
  ref = N_VClone(y);
  ewt = N_VClone(y);

  /* set the reference solution data */
  NV_Ith_S(ref, 0) = SUN_RCONST(5.2083495894337328e-08);
  NV_Ith_S(ref, 1) = SUN_RCONST(2.0833399429795671e-13);
  NV_Ith_S(ref, 2) = SUN_RCONST(9.9999994791629776e-01);

  /* compute the error weight vector, loosen atol */
  N_VAbs(ref, ewt);
  N_VLinearSum(rtol, ewt, SUN_RCONST(10.0), atol, ewt);
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
