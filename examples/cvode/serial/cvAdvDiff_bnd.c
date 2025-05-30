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
 * The following is a simple example problem with a banded Jacobian,
 * with the program for its solution by CVODE.
 * The problem is the semi-discrete form of the advection-diffusion
 * equation in 2-D:
 *   du/dt = d^2 u / dx^2 + .5 du/dx + d^2 u / dy^2
 * on the rectangle 0 <= x <= 2, 0 <= y <= 1, and the time
 * interval 0 <= t <= 1. Homogeneous Dirichlet boundary conditions
 * are posed, and the initial condition is
 *   u(x,y,t=0) = x(2-x)y(1-y)exp(5xy).
 * The PDE is discretized on a uniform MX+2 by MY+2 grid with
 * central differencing, and with boundary values eliminated,
 * leaving an ODE system of size NEQ = MX*MY.
 * This program solves the problem with the BDF method, Newton
 * iteration with the SUNBAND linear solver, and a user-supplied
 * Jacobian routine.
 * It uses scalar relative and absolute tolerances.
 * Output is printed at t = .1, .2, ..., 1.
 * Run statistics (optional outputs) are printed at the end.
 * -----------------------------------------------------------------*/

#include <cvode/cvode.h> /* prototypes for CVODE fcts., consts.  */
#include <math.h>
#include <nvector/nvector_serial.h> /* access to serial N_Vector            */
#include <stdio.h>
#include <stdlib.h>
#include <sunlinsol/sunlinsol_band.h> /* access to band SUNLinearSolver       */
#include <sunmatrix/sunmatrix_band.h> /* access to band SUNMatrix             */

/* Problem Constants */

#define XMAX  SUN_RCONST(2.0) /* domain boundaries         */
#define YMAX  SUN_RCONST(1.0)
#define MX    10 /* mesh dimensions           */
#define MY    5
#define NEQ   MX* MY             /* number of equations       */
#define ATOL  SUN_RCONST(1.0e-5) /* scalar absolute tolerance */
#define T0    SUN_RCONST(0.0)    /* initial time              */
#define T1    SUN_RCONST(0.1)    /* first output time         */
#define DTOUT SUN_RCONST(0.1)    /* output time increment     */
#define NOUT  10                 /* number of output times    */

#define ZERO SUN_RCONST(0.0)
#define HALF SUN_RCONST(0.5)
#define ONE  SUN_RCONST(1.0)
#define TWO  SUN_RCONST(2.0)
#define FIVE SUN_RCONST(5.0)

/* User-defined vector access macro IJth */

/* IJth is defined in order to isolate the translation from the
   mathematical 2-dimensional structure of the dependent variable vector
   to the underlying 1-dimensional storage.
   IJth(vdata,i,j) references the element in the vdata array for
   u at mesh point (i,j), where 1 <= i <= MX, 1 <= j <= MY.
   The vdata array is obtained via the call vdata = N_VGetArrayPointer(v),
   where v is an N_Vector.
   The variables are ordered by the y index j, then by the x index i. */

#define IJth(vdata, i, j) (vdata[(j - 1) + (i - 1) * MY])

/* Type : UserData (contains grid constants) */

typedef struct
{
  sunrealtype dx, dy, hdcoef, hacoef, vdcoef;
  SUNProfiler profobj;
}* UserData;

/* Private Helper Functions */

static void SetIC(N_Vector u, UserData data);
static void PrintHeader(sunrealtype reltol, sunrealtype abstol, sunrealtype umax);
static void PrintOutput(sunrealtype t, sunrealtype umax, long int nst);
static void PrintFinalStats(void* cvode_mem);

/* Private function to check function return values */

static int check_retval(int retval, const char* funcname);

/* Functions Called by the Solver */

static int f(sunrealtype t, N_Vector u, N_Vector udot, void* user_data);
static int Jac(sunrealtype t, N_Vector u, N_Vector fu, SUNMatrix J,
               void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

/*
 *-------------------------------
 * Main Program
 *-------------------------------
 */

int main(void)
{
  sunrealtype dx, dy, reltol, abstol, t, tout, umax;
  N_Vector u;
  UserData data;
  SUNMatrix A;
  SUNLinearSolver LS;
  void* cvode_mem;
  int iout, retval;
  long int nst;
  SUNContext sunctx;
  SUNProfiler profobj;

  /* Initialize variables */
  u         = NULL;
  data      = NULL;
  A         = NULL;
  LS        = NULL;
  cvode_mem = NULL;
  sunctx    = NULL;

  /* Create the SUNDIALS context */
  retval = SUNContext_Create(SUN_COMM_NULL, &sunctx);
  if (check_retval(retval, "SUNContext_Create")) { return (1); }

  /* Setup different error handler stack so that we abort after logging */
  SUNContext_PopErrHandler(sunctx);
  SUNContext_PushErrHandler(sunctx, SUNAbortErrHandlerFn, NULL);
  SUNContext_PushErrHandler(sunctx, SUNLogErrHandlerFn, NULL);

  /* Get a reference to the profiler */
  retval = SUNContext_GetProfiler(sunctx, &profobj);
  if (check_retval(retval, "SUNContext_GetProfiler")) { return (1); }

  SUNDIALS_MARK_FUNCTION_BEGIN(profobj);

  /* Create a serial vector */

  u = N_VNew_Serial(NEQ, sunctx); /* Allocate u vector */
  if (check_retval(SUNContext_GetLastError(sunctx), "N_VNew_Serial"))
  {
    return (1);
  }

  reltol = ZERO; /* Set the tolerances */
  abstol = ATOL;

  data = (UserData)malloc(sizeof *data); /* Allocate data memory */
  if (!data)
  {
    fprintf(stderr, "MEMORY_ERROR: malloc failed - returned NULL pointer\n");
    return 1;
  }

  dx = data->dx = XMAX / (MX + 1); /* Set grid coefficients in data */
  dy = data->dy = YMAX / (MY + 1);
  data->hdcoef  = ONE / (dx * dx);
  data->hacoef  = HALF / (TWO * dx);
  data->vdcoef  = ONE / (dy * dy);
  data->profobj = profobj;

  SUNDIALS_MARK_BEGIN(profobj, "Setup");

  SetIC(u, data); /* Initialize u vector */

  /* Call CVodeCreate to create the solver memory and specify the
   * Backward Differentiation Formula */

  cvode_mem = CVodeCreate(CV_BDF, sunctx);
  if (check_retval(SUNContext_GetLastError(sunctx), "CVodeCreate"))
  {
    return (1);
  }

  /* Call CVodeInit to initialize the integrator memory and specify the
   * user's right hand side function in u'=f(t,u), the initial time T0, and
   * the initial dependent variable vector u. */
  retval = CVodeInit(cvode_mem, f, T0, u);
  if (check_retval(retval, "CVodeInit")) { return (1); }

  /* Call CVodeSStolerances to specify the scalar relative tolerance
   * and scalar absolute tolerance */
  retval = CVodeSStolerances(cvode_mem, reltol, abstol);
  if (check_retval(retval, "CVodeSStolerances")) { return (1); }

  /* Set the pointer to user-defined data */
  retval = CVodeSetUserData(cvode_mem, data);
  if (check_retval(retval, "CVodeSetUserData")) { return (1); }

  /* Create banded SUNMatrix for use in linear solves -- since this will be factored,
     set the storage bandwidth to be the sum of upper and lower bandwidths */
  A = SUNBandMatrix(NEQ, MY, MY, sunctx);
  if (check_retval(SUNContext_GetLastError(sunctx), "SUNBandMatrix"))
  {
    return (1);
  }

  /* Create banded SUNLinearSolver object for use by CVode */
  LS = SUNLinSol_Band(u, A, sunctx);
  if (check_retval(SUNContext_GetLastError(sunctx), "SUNLinSol_Band"))
  {
    return (1);
  }

  /* Call CVodeSetLinearSolver to attach the matrix and linear solver to CVode */
  retval = CVodeSetLinearSolver(cvode_mem, LS, A);
  if (check_retval(retval, "CVodeSetLinearSolver")) { return (1); }

  /* Set the user-supplied Jacobian routine Jac */
  retval = CVodeSetJacFn(cvode_mem, Jac);
  if (check_retval(retval, "CVodeSetJacFn")) { return (1); }

  SUNDIALS_MARK_END(profobj, "Setup");

  /* In loop over output points: call CVode, print results, test for errors */

  SUNDIALS_MARK_BEGIN(profobj, "Integration loop");
  umax = N_VMaxNorm(u);
  PrintHeader(reltol, abstol, umax);
  for (iout = 1, tout = T1; iout <= NOUT; iout++, tout += DTOUT)
  {
    retval = CVode(cvode_mem, tout, u, &t, CV_NORMAL);
    if (check_retval(retval, "CVode")) { break; }
    umax   = N_VMaxNorm(u);
    retval = CVodeGetNumSteps(cvode_mem, &nst);
    check_retval(retval, "CVodeGetNumSteps");
    PrintOutput(t, umax, nst);
  }
  SUNDIALS_MARK_END(profobj, "Integration loop");
  PrintFinalStats(cvode_mem); /* Print some final statistics   */

  N_VDestroy(u);         /* Free the u vector          */
  CVodeFree(&cvode_mem); /* Free the integrator memory */
  SUNLinSolFree(LS);     /* Free linear solver memory  */
  SUNMatDestroy(A);      /* Free the matrix memory     */
  free(data);            /* Free the user data         */

  SUNDIALS_MARK_FUNCTION_END(profobj);
  SUNContext_Free(&sunctx);
  return (0);
}

/*
 *-------------------------------
 * Functions called by the solver
 *-------------------------------
 */

/* f routine. Compute f(t,u). */

static int f(sunrealtype t, N_Vector u, N_Vector udot, void* user_data)
{
  sunrealtype uij, udn, uup, ult, urt, hordc, horac, verdc, hdiff, hadv, vdiff;
  sunrealtype *udata, *dudata;
  int i, j;
  UserData data;

  udata  = N_VGetArrayPointer(u);
  dudata = N_VGetArrayPointer(udot);

  /* Extract needed constants from data */

  data  = (UserData)user_data;
  hordc = data->hdcoef;
  horac = data->hacoef;
  verdc = data->vdcoef;

  SUNDIALS_MARK_BEGIN(data->profobj, "RHS");

  /* Loop over all grid points. */

  for (j = 1; j <= MY; j++)
  {
    for (i = 1; i <= MX; i++)
    {
      /* Extract u at x_i, y_j and four neighboring points */

      uij = IJth(udata, i, j);
      udn = (j == 1) ? ZERO : IJth(udata, i, j - 1);
      uup = (j == MY) ? ZERO : IJth(udata, i, j + 1);
      ult = (i == 1) ? ZERO : IJth(udata, i - 1, j);
      urt = (i == MX) ? ZERO : IJth(udata, i + 1, j);

      /* Set diffusion and advection terms and load into udot */

      hdiff              = hordc * (ult - TWO * uij + urt);
      hadv               = horac * (urt - ult);
      vdiff              = verdc * (uup - TWO * uij + udn);
      IJth(dudata, i, j) = hdiff + hadv + vdiff;
    }
  }

  SUNDIALS_MARK_END(data->profobj, "RHS");

  return (0);
}

/* Jacobian routine. Compute J(t,u). */

static int Jac(sunrealtype t, N_Vector u, N_Vector fu, SUNMatrix J,
               void* user_data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
  sunindextype i, j, k;
  sunrealtype *kthCol, hordc, horac, verdc;
  UserData data;

  /*
   * The components of f = udot that depend on u(i,j) are
   * f(i,j), f(i-1,j), f(i+1,j), f(i,j-1), f(i,j+1), with
   *   df(i,j)/du(i,j) = -2 (1/dx^2 + 1/dy^2)
   *   df(i-1,j)/du(i,j) = 1/dx^2 + .25/dx  (if i > 1)
   *   df(i+1,j)/du(i,j) = 1/dx^2 - .25/dx  (if i < MX)
   *   df(i,j-1)/du(i,j) = 1/dy^2           (if j > 1)
   *   df(i,j+1)/du(i,j) = 1/dy^2           (if j < MY)
   */

  data  = (UserData)user_data;
  hordc = data->hdcoef;
  horac = data->hacoef;
  verdc = data->vdcoef;

  SUNDIALS_MARK_BEGIN(data->profobj, "Jac");

  /* set non-zero Jacobian entries */
  for (j = 1; j <= MY; j++)
  {
    for (i = 1; i <= MX; i++)
    {
      k      = j - 1 + (i - 1) * MY;
      kthCol = SUNBandMatrix_Column(J, k);

      /* set the kth column of J */

      SM_COLUMN_ELEMENT_B(kthCol, k, k) = -TWO * (verdc + hordc);
      if (i != 1) { SM_COLUMN_ELEMENT_B(kthCol, k - MY, k) = hordc + horac; }
      if (i != MX) { SM_COLUMN_ELEMENT_B(kthCol, k + MY, k) = hordc - horac; }
      if (j != 1) { SM_COLUMN_ELEMENT_B(kthCol, k - 1, k) = verdc; }
      if (j != MY) { SM_COLUMN_ELEMENT_B(kthCol, k + 1, k) = verdc; }
    }
  }

  SUNDIALS_MARK_END(data->profobj, "Jac");

  return (0);
}

/*
 *-------------------------------
 * Private helper functions
 *-------------------------------
 */

/* Set initial conditions in u vector */

static void SetIC(N_Vector u, UserData data)
{
  int i, j;
  sunrealtype x, y, dx, dy;
  sunrealtype* udata;

  /* Extract needed constants from data */

  dx = data->dx;
  dy = data->dy;

  /* Set pointer to data array in vector u. */

  udata = N_VGetArrayPointer(u);

  /* Load initial profile into u vector */

  for (j = 1; j <= MY; j++)
  {
    y = j * dy;
    for (i = 1; i <= MX; i++)
    {
      x = i * dx;
      IJth(udata, i, j) = x * (XMAX - x) * y * (YMAX - y) * SUNRexp(FIVE * x * y);
    }
  }
}

/* Print first lines of output (problem description) */

static void PrintHeader(sunrealtype reltol, sunrealtype abstol, sunrealtype umax)
{
  printf("\n2-D Advection-Diffusion Equation\n");
  printf("Mesh dimensions = %d X %d\n", MX, MY);
  printf("Total system size = %d\n", NEQ);
#if defined(SUNDIALS_EXTENDED_PRECISION)
  printf("Tolerance parameters: reltol = %Lg   abstol = %Lg\n\n", reltol, abstol);
  printf("At t = %Lg      max.norm(u) =%14.6Le \n", T0, umax);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
  printf("Tolerance parameters: reltol = %g   abstol = %g\n\n", reltol, abstol);
  printf("At t = %g      max.norm(u) =%14.6e \n", T0, umax);
#else
  printf("Tolerance parameters: reltol = %g   abstol = %g\n\n", reltol, abstol);
  printf("At t = %g      max.norm(u) =%14.6e \n", T0, umax);
#endif

  return;
}

/* Print current value */

static void PrintOutput(sunrealtype t, sunrealtype umax, long int nst)
{
#if defined(SUNDIALS_EXTENDED_PRECISION)
  printf("At t = %4.2Lf   max.norm(u) =%14.6Le   nst = %4ld\n", t, umax, nst);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
  printf("At t = %4.2f   max.norm(u) =%14.6e   nst = %4ld\n", t, umax, nst);
#else
  printf("At t = %4.2f   max.norm(u) =%14.6e   nst = %4ld\n", t, umax, nst);
#endif

  return;
}

/* Get and print some final statistics */

static void PrintFinalStats(void* cvode_mem)
{
  int retval;
  long int nst, nfe, nsetups, netf, nni, ncfn, nje, nfeLS;

  retval = CVodeGetNumSteps(cvode_mem, &nst);
  check_retval(retval, "CVodeGetNumSteps");
  retval = CVodeGetNumRhsEvals(cvode_mem, &nfe);
  check_retval(retval, "CVodeGetNumRhsEvals");
  retval = CVodeGetNumLinSolvSetups(cvode_mem, &nsetups);
  check_retval(retval, "CVodeGetNumLinSolvSetups");
  retval = CVodeGetNumErrTestFails(cvode_mem, &netf);
  check_retval(retval, "CVodeGetNumErrTestFails");
  retval = CVodeGetNumNonlinSolvIters(cvode_mem, &nni);
  check_retval(retval, "CVodeGetNumNonlinSolvIters");
  retval = CVodeGetNumNonlinSolvConvFails(cvode_mem, &ncfn);
  check_retval(retval, "CVodeGetNumNonlinSolvConvFails");

  retval = CVodeGetNumJacEvals(cvode_mem, &nje);
  check_retval(retval, "CVodeGetNumJacEvals");
  retval = CVodeGetNumLinRhsEvals(cvode_mem, &nfeLS);
  check_retval(retval, "CVodeGetNumLinRhsEvals");

  printf("\nFinal Statistics:\n");
  printf("nst = %-6ld nfe  = %-6ld nsetups = %-6ld nfeLS = %-6ld nje = %ld\n",
         nst, nfe, nsetups, nfeLS, nje);
  printf("nni = %-6ld ncfn = %-6ld netf = %ld\n", nni, ncfn, netf);

  return;
}

/* Check function return value */
static int check_retval(int retval, const char* funcname)
{
  /* Check if retval < 0 */
  if (retval < 0)
  {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with retval = %d\n\n",
            funcname, retval);
    return (1);
  }
  else { return (0); }
}
