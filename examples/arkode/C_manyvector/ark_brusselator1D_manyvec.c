/*---------------------------------------------------------------
 * Programmer(s): Cody J. Balos @ LLNL
 *                Daniel R. Reynolds @ SMU
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
 * The following test simulates a brusselator problem from chemical
 * kinetics.  This is n PDE system with 3 components, Y = [u,v,w],
 * satisfying the equations,
 *    u_t = du*u_xx + a - (w+1)*u + v*u^2
 *    v_t = dv*v_xx + w*u - v*u^2
 *    w_t = dw*w_xx + (b-w)/ep - w*u
 * for t in [0, 10], x in [0, 1], with initial conditions
 *    u(0,x) =  a  + 0.1*sin(pi*x)
 *    v(0,x) = b/a + 0.1*sin(pi*x)
 *    w(0,x) =  b  + 0.1*sin(pi*x),
 * and with stationary boundary conditions, i.e.
 *    u_t(t,0) = u_t(t,1) = 0,
 *    v_t(t,0) = v_t(t,1) = 0,
 *    w_t(t,0) = w_t(t,1) = 0.
 * Note: these can also be implemented as Dirichlet boundary
 * conditions with values identical to the initial conditions.
 *
 * The spatial derivatives are computed using second-order
 * centered differences, with the data distributed over N points
 * on a uniform spatial grid.
 *
 * The data is stored using the ManyVector structure for a
 * structure-of-arrays format, i.e., each of u, v and w are
 * stored in separate serial vectors, and are attached together
 * using the ManyVector infrastructure.
 *
 * This program solves the problem with the ARK method, treating
 * only the reaction terms implicitly (diffusion is treated
 * explicitly), using a Newton iteration with the SUNSPGMR
 * iterative linear solver, and a user-supplied
 * Jacobian-vector-product routine.
 *
 * 100 outputs are printed at equal intervals, and run statistics
 * are printed at the end.
 *---------------------------------------------------------------*/

/* Header files */
#include <arkode/arkode_arkstep.h> /* prototypes for ARKStep fcts., consts */
#include <math.h>
#include <nvector/nvector_manyvector.h>/* manyvector N_Vector types, fcts. etc */
#include <nvector/nvector_serial.h> /* serial N_Vector types, fcts., macros */
#include <stdio.h>
#include <stdlib.h>
#include <sundials/sundials_types.h> /* defs. of sunrealtype, sunindextype, etc */
#include <sunlinsol/sunlinsol_spgmr.h> /* access to SPGMR SUNLinearSolver      */

#if defined(SUNDIALS_EXTENDED_PRECISION)
#define GSYM "Lg"
#define ESYM "Le"
#define FSYM "Lf"
#else
#define GSYM "g"
#define ESYM "e"
#define FSYM "f"
#endif

/* sunrealtype constant macros */
#define ZERO SUN_RCONST(0.0)
#define ONE  SUN_RCONST(1.0)
#define TWO  SUN_RCONST(2.0)

/* user data structure */
typedef struct
{
  sunindextype N; /* number of intervals     */
  sunrealtype dx; /* mesh spacing            */
  sunrealtype a;  /* constant forcing on u   */
  sunrealtype b;  /* steady-state value of w */
  sunrealtype du; /* diffusion coeff for u   */
  sunrealtype dv; /* diffusion coeff for v   */
  sunrealtype dw; /* diffusion coeff for w   */
  sunrealtype ep; /* stiffness parameter     */
}* UserData;

/* User-supplied Functions Called by the Solver */
static int fe(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data);
static int fi(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data);
static int JacVI(N_Vector v, N_Vector Jv, sunrealtype t, N_Vector y,
                 N_Vector fy, void* user_data, N_Vector tmp1);

/* Private function to check function return values */
static int check_flag(void* flagvalue, const char* funcname, int opt);

/* Main Program */
int main(void)
{
  /* general problem parameters */
  sunrealtype T0    = ZERO;             /* initial time */
  sunrealtype Tf    = SUN_RCONST(10.0); /* final time */
  int Nt            = 100;              /* total number of output times */
  int Nvar          = 3;                /* number of solution fields */
  UserData userdata = NULL;
  sunrealtype *udata, *vdata, *wdata;
  sunindextype N     = 201;             /* spatial mesh size */
  sunrealtype a      = SUN_RCONST(0.6); /* problem parameters */
  sunrealtype b      = SUN_RCONST(2.0);
  sunrealtype du     = SUN_RCONST(0.001);
  sunrealtype dv     = SUN_RCONST(0.001);
  sunrealtype dw     = SUN_RCONST(0.001);
  sunrealtype ep     = SUN_RCONST(1.0e-5); /* stiffness parameter */
  sunrealtype reltol = SUN_RCONST(1.0e-6); /* tolerances */
  sunrealtype abstol = SUN_RCONST(1.0e-10);
  sunindextype i;

  /* general problem variables */
  int flag;          /* reusable error-checking flag */
  N_Vector y = NULL; /* empty manyvector for storing solution */
  N_Vector u = NULL; /* empty vectors for storing solution components */
  N_Vector v = NULL;
  N_Vector w = NULL;
  N_Vector uvw[3]; /* vector array composed of u,v,w component vectors */
  SUNLinearSolver LS = NULL; /* empty linear solver object */
  void* arkode_mem   = NULL; /* empty ARKode memory structure */
  sunrealtype pi, t, dTout, tout, unorm, vnorm, wnorm;
  FILE *FID, *UFID, *VFID, *WFID;
  int iout;
  long int nst, nst_a, nfe, nfi, nsetups, nli, nlcf, nJv, nfeLS, nni, ncfn, netf;

  /* Create the SUNDIALS context object for this simulation */
  SUNContext ctx;
  flag = SUNContext_Create(SUN_COMM_NULL, &ctx);
  if (check_flag(&flag, "SUNContext_Create", 1)) { return 1; }

  /* allocate udata structure */
  userdata = (UserData)malloc(sizeof(*userdata));
  if (check_flag((void*)userdata, "malloc", 2)) { return 1; }

  /* store the inputs in the UserData structure */
  userdata->N  = N;
  userdata->a  = a;
  userdata->b  = b;
  userdata->du = du;
  userdata->dv = dv;
  userdata->dw = dw;
  userdata->ep = ep;

  /* Initial problem output */
  printf("\n1D Brusselator PDE test problem:\n");
  printf("    N = %li\n", (long int)userdata->N);
  printf("    problem parameters:  a = %" GSYM ",  b = %" GSYM ",  ep = %" GSYM
         "\n",
         userdata->a, userdata->b, userdata->ep);
  printf("    diffusion coefficients:  du = %" GSYM ",  dv = %" GSYM
         ",  dw = %" GSYM "\n",
         userdata->du, userdata->dv, userdata->dw);
  printf("    reltol = %.1" ESYM ",  abstol = %.1" ESYM "\n\n", reltol, abstol);

  /* Initialize data structures */
  userdata->dx = ONE / (N - 1);         /* set spatial mesh spacing */
  u            = N_VNew_Serial(N, ctx); /* Create serial vectors */
  if (check_flag((void*)u, "N_VNew_Serial", 0)) { return 1; }
  v = N_VClone(u);
  if (check_flag((void*)v, "N_VClone", 0)) { return 1; }
  w = N_VClone(u);
  if (check_flag((void*)w, "N_VClone", 0)) { return 1; }

  /* Create manyvector for solution */
  uvw[0] = u;
  uvw[1] = v;
  uvw[2] = w;
  y      = N_VNew_ManyVector(Nvar, uvw, ctx);
  if (check_flag((void*)y, "N_VNew_ManyVector", 0)) { return 1; }

  udata = N_VGetArrayPointer(u); /* Access data array for new NVector u */
  if (check_flag((void*)udata, "N_VGetArrayPointer", 0)) { return 1; }
  vdata = N_VGetArrayPointer(v); /* Access data array for new NVector v */
  if (check_flag((void*)vdata, "N_VGetArrayPointer", 0)) { return 1; }
  wdata = N_VGetArrayPointer(w); /* Access data array for new NVector w */
  if (check_flag((void*)wdata, "N_VGetArrayPointer", 0)) { return 1; }

  /* Set initial conditions into y */
  pi = SUN_RCONST(4.0) * atan(ONE);
  for (i = 0; i < N; i++)
  {
    udata[i] = a + SUN_RCONST(0.1) * sin(pi * i * userdata->dx);     /* u */
    vdata[i] = b / a + SUN_RCONST(0.1) * sin(pi * i * userdata->dx); /* v */
    wdata[i] = b + SUN_RCONST(0.1) * sin(pi * i * userdata->dx);     /* w */
  }

  /* Call ARKStepCreate to initialize the ARK timestepper module and
     specify the right-hand side function in y'=f(t,y), the initial time
     T0, and the initial dependent variable vector y.  Note: since this
     problem is fully implicit, we set f_E to NULL and f_I to f. */
  arkode_mem = ARKStepCreate(fe, fi, T0, y, ctx);
  if (check_flag((void*)arkode_mem, "ARKStepCreate", 0)) { return 1; }

  /* Set routines */
  flag = ARKodeSetUserData(arkode_mem,
                           (void*)userdata); /* Pass udata to user functions */
  if (check_flag(&flag, "ARKodeSetUserData", 1)) { return 1; }
  flag = ARKodeSStolerances(arkode_mem, reltol, abstol); /* Specify tolerances */
  if (check_flag(&flag, "ARKodeSStolerances", 1)) { return 1; }

  /* Initialize spgmr solver */
  LS = SUNLinSol_SPGMR(y, SUN_PREC_NONE, 10, ctx);
  if (check_flag((void*)LS, "SUNLinSol_SPGMR", 0)) { return 1; }

  /* Linear solver interface */
  flag = ARKodeSetLinearSolver(arkode_mem, LS, NULL); /* Attach linear solver */
  if (check_flag(&flag, "ARKodeSetLinearSolver", 1)) { return 1; }
  flag = ARKodeSetJacTimes(arkode_mem, NULL,
                           JacVI); /* Set the Jacobian-vector product */
  if (check_flag(&flag, "ARKodeSetJacTimes", 1)) { return 1; }

  /* output spatial mesh to disk */
  FID = fopen("bruss_mesh.txt", "w");
  for (i = 0; i < N; i++)
  {
    fprintf(FID, "  %.16" ESYM "\n", userdata->dx * i);
  }
  fclose(FID);

  /* Open output streams for results, access data array */
  UFID = fopen("bruss_u.txt", "w");
  VFID = fopen("bruss_v.txt", "w");
  WFID = fopen("bruss_w.txt", "w");

  /* output initial condition to disk */
  for (i = 0; i < N; i++) { fprintf(UFID, " %.16" ESYM "", udata[i]); }
  for (i = 0; i < N; i++) { fprintf(VFID, " %.16" ESYM "", vdata[i]); }
  for (i = 0; i < N; i++) { fprintf(WFID, " %.16" ESYM "", wdata[i]); }
  fprintf(UFID, "\n");
  fprintf(VFID, "\n");
  fprintf(WFID, "\n");

  /* Main time-stepping loop: calls ARKodeEvolve to perform the integration, then
     prints results.  Stops when the final time has been reached */
  t     = T0;
  dTout = (Tf - T0) / Nt;
  tout  = T0 + dTout;
  printf("        t      ||u||_rms   ||v||_rms   ||w||_rms\n");
  printf("   ----------------------------------------------\n");
  for (iout = 0; iout < Nt; iout++)
  {
    /* call integrator */
    flag = ARKodeEvolve(arkode_mem, tout, y, &t, ARK_NORMAL);
    if (check_flag(&flag, "ARKodeEvolve", 1)) { break; }

    /* print solution statistics */
    unorm = N_VDotProd(u, u);
    unorm = sqrt(unorm / N);
    vnorm = N_VDotProd(v, v);
    vnorm = sqrt(vnorm / N);
    wnorm = N_VDotProd(w, w);
    wnorm = sqrt(wnorm / N);
    printf("  %10.6" FSYM "  %10.6" FSYM "  %10.6" FSYM "  %10.6" FSYM "\n", t,
           unorm, vnorm, wnorm);

    /* check integrator flag */
    if (flag >= 0)
    { /* successful solve: update output time */
      tout += dTout;
      tout = (tout > Tf) ? Tf : tout;
    }
    else
    { /* unsuccessful solve: break */
      fprintf(stderr, "Solver failure, stopping integration\n");
      break;
    }

    /* output results to disk */
    for (i = 0; i < N; i++) { fprintf(UFID, " %.16" ESYM "", udata[i]); }
    for (i = 0; i < N; i++) { fprintf(VFID, " %.16" ESYM "", vdata[i]); }
    for (i = 0; i < N; i++) { fprintf(WFID, " %.16" ESYM "", wdata[i]); }
    fprintf(UFID, "\n");
    fprintf(VFID, "\n");
    fprintf(WFID, "\n");
  }
  printf("   ----------------------------------------------\n");
  fclose(UFID);
  fclose(VFID);
  fclose(WFID);

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
  flag = ARKodeGetNumNonlinSolvIters(arkode_mem, &nni);
  check_flag(&flag, "ARKodeGetNumNonlinSolvIters", 1);
  flag = ARKodeGetNumNonlinSolvConvFails(arkode_mem, &ncfn);
  check_flag(&flag, "ARKodeGetNumNonlinSolvConvFails", 1);
  flag = ARKodeGetNumLinIters(arkode_mem, &nli);
  check_flag(&flag, "ARKodeGetNumLinIters", 1);
  flag = ARKodeGetNumLinConvFails(arkode_mem, &nlcf);
  check_flag(&flag, "ARKodeGetNumLinConvFails", 1);
  flag = ARKodeGetNumJtimesEvals(arkode_mem, &nJv);
  check_flag(&flag, "ARKodeGetNumJtimesEvals", 1);
  flag = ARKodeGetNumLinRhsEvals(arkode_mem, &nfeLS);
  check_flag(&flag, "ARKodeGetNumLinRhsEvals", 1);

  printf("\nFinal Solver Statistics:\n");
  printf("   Internal solver steps = %li (attempted = %li)\n", nst, nst_a);
  printf("   Total RHS evals:  Fe = %li,  Fi = %li\n", nfe, nfi);
  printf("   Total linear solver setups = %li\n", nsetups);
  printf("   Total linear iterations = %li\n", nli);
  printf("   Total linear convergence failures = %li\n", nlcf);
  printf("   Total J*v evaluations = %li\n", nJv);
  printf("   Total RHS evals in linear solver = %li\n", nfeLS);
  printf("   Total number of Newton iterations = %li\n", nni);
  printf("   Total number of nonlinear solver convergence failures = %li\n",
         ncfn);
  printf("   Total number of error test failures = %li\n\n", netf);

  /* Clean up and return with successful completion */
  N_VDestroy(y); /* Free vectors */
  N_VDestroy(u);
  N_VDestroy(v);
  N_VDestroy(w);
  free(userdata);          /* Free user data */
  ARKodeFree(&arkode_mem); /* Free integrator memory */
  SUNLinSolFree(LS);       /* Free linear solver */
  SUNContext_Free(&ctx);   /* Free context */

  return 0;
}

/*-------------------------------
 * Functions called by the solver
 *-------------------------------*/

/* fe routine to compute the diffusion portion of the ODE RHS. */
static int fe(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data)
{
  UserData userdata = (UserData)user_data; /* access problem data */
  sunindextype N    = userdata->N;         /* set variable shortcuts */
  sunrealtype du    = userdata->du;
  sunrealtype dv    = userdata->dv;
  sunrealtype dw    = userdata->dw;
  sunrealtype dx    = userdata->dx;
  sunrealtype *y_u = NULL, *y_v = NULL, *y_w = NULL;
  sunrealtype *f_u = NULL, *f_v = NULL, *f_w = NULL;
  sunrealtype uconst, vconst, wconst;
  sunindextype i;

  y_u = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 0));
  if (check_flag((void*)y_u, "N_VGetArrayPointer", 0)) { return 1; }
  y_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 1));
  if (check_flag((void*)y_v, "N_VGetArrayPointer", 0)) { return 1; }
  y_w = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 2));
  if (check_flag((void*)y_w, "N_VGetArrayPointer", 0)) { return 1; }

  f_u = N_VGetArrayPointer(N_VGetSubvector_ManyVector(ydot, 0));
  if (check_flag((void*)f_u, "N_VGetArrayPointer", 0)) { return 1; }
  f_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(ydot, 1));
  if (check_flag((void*)f_v, "N_VGetArrayPointer", 0)) { return 1; }
  f_w = N_VGetArrayPointer(N_VGetSubvector_ManyVector(ydot, 2));
  if (check_flag((void*)f_w, "N_VGetArrayPointer", 0)) { return 1; }

  N_VConst(SUN_RCONST(0.0), ydot); /* initialize ydot to zero */

  /* iterate over domain, computing all equations */
  uconst = du / dx / dx;
  vconst = dv / dx / dx;
  wconst = dw / dx / dx;
  for (i = 1; i < N - 1; i++)
  {
    /* Fill in ODE RHS for u */
    f_u[i] = (y_u[i - 1] - TWO * y_u[i] + y_u[i + 1]) * uconst;

    /* Fill in ODE RHS for v */
    f_v[i] = (y_v[i - 1] - TWO * y_v[i] + y_v[i + 1]) * vconst;

    /* Fill in ODE RHS for w */
    f_w[i] = (y_w[i - 1] - TWO * y_w[i] + y_w[i + 1]) * wconst;
  }

  /* enforce stationary boundaries */
  f_u[0] = f_v[0] = f_w[0] = ZERO;
  f_u[N - 1] = f_v[N - 1] = f_w[N - 1] = ZERO;

  return 0; /* Return with success */
}

/* fi routine to compute the reaction portion of the ODE RHS. */
static int fi(sunrealtype t, N_Vector y, N_Vector ydot, void* user_data)
{
  UserData userdata = (UserData)user_data; /* access problem data */
  sunindextype N    = userdata->N;         /* set variable shortcuts */
  sunrealtype a     = userdata->a;
  sunrealtype b     = userdata->b;
  sunrealtype ep    = userdata->ep;
  sunrealtype *y_u = NULL, *y_v = NULL, *y_w = NULL;
  sunrealtype *f_u = NULL, *f_v = NULL, *f_w = NULL;
  sunindextype i;

  y_u = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 0));
  if (check_flag((void*)y_u, "N_VGetArrayPointer", 0)) { return 1; }
  y_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 1));
  if (check_flag((void*)y_v, "N_VGetArrayPointer", 0)) { return 1; }
  y_w = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 2));
  if (check_flag((void*)y_w, "N_VGetArrayPointer", 0)) { return 1; }

  f_u = N_VGetArrayPointer(N_VGetSubvector_ManyVector(ydot, 0));
  if (check_flag((void*)f_u, "N_VGetArrayPointer", 0)) { return 1; }
  f_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(ydot, 1));
  if (check_flag((void*)f_v, "N_VGetArrayPointer", 0)) { return 1; }
  f_w = N_VGetArrayPointer(N_VGetSubvector_ManyVector(ydot, 2));
  if (check_flag((void*)f_w, "N_VGetArrayPointer", 0)) { return 1; }

  N_VConst(0.0, ydot); /* initialize ydot to zero */

  /* iterate over domain, computing all equations */
  for (i = 1; i < N - 1; i++)
  {
    /* Fill in ODE RHS for u */
    f_u[i] = a - (y_w[i] + ONE) * y_u[i] + y_v[i] * y_u[i] * y_u[i];

    /* Fill in ODE RHS for v */
    f_v[i] = y_w[i] * y_u[i] - y_v[i] * y_u[i] * y_u[i];

    /* Fill in ODE RHS for w */
    f_w[i] = (b - y_w[i]) / ep - y_w[i] * y_u[i];
  }

  /* enforce stationary boundaries */
  f_u[0] = f_v[0] = f_w[0] = ZERO;
  f_u[N - 1] = f_v[N - 1] = f_w[N - 1] = ZERO;

  return 0; /* Return with success */
}

/* Jacobian-vector product routine (implicit portion only) */
static int JacVI(N_Vector v, N_Vector Jv, sunrealtype t, N_Vector y,
                 N_Vector fy, void* user_data, N_Vector tmp1)
{
  UserData userdata = (UserData)user_data; /* access problem data */
  sunindextype N    = userdata->N;         /* set variable shortcuts */
  sunrealtype ep    = userdata->ep;
  sunrealtype *y_u = NULL, *y_v = NULL, *y_w = NULL;
  sunrealtype *v_u = NULL, *v_v = NULL, *v_w = NULL;
  sunrealtype *Ju_v = NULL, *Jv_v = NULL, *Jw_v = NULL;
  sunindextype i;

  y_u = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 0));
  if (check_flag((void*)y_u, "N_VGetArrayPointer", 0)) { return 1; }
  y_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 1));
  if (check_flag((void*)y_v, "N_VGetArrayPointer", 0)) { return 1; }
  y_w = N_VGetArrayPointer(N_VGetSubvector_ManyVector(y, 2));
  if (check_flag((void*)y_w, "N_VGetArrayPointer", 0)) { return 1; }

  v_u = N_VGetArrayPointer(N_VGetSubvector_ManyVector(v, 0));
  if (check_flag((void*)v_u, "N_VGetArrayPointer", 0)) { return 1; }
  v_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(v, 1));
  if (check_flag((void*)v_v, "N_VGetArrayPointer", 0)) { return 1; }
  v_w = N_VGetArrayPointer(N_VGetSubvector_ManyVector(v, 2));
  if (check_flag((void*)v_w, "N_VGetArrayPointer", 0)) { return 1; }

  Ju_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(Jv, 0));
  if (check_flag((void*)Ju_v, "N_VGetArrayPointer", 0)) { return 1; }
  Jv_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(Jv, 1));
  if (check_flag((void*)Jv_v, "N_VGetArrayPointer", 0)) { return 1; }
  Jw_v = N_VGetArrayPointer(N_VGetSubvector_ManyVector(Jv, 2));
  if (check_flag((void*)Jw_v, "N_VGetArrayPointer", 0)) { return 1; }

  N_VConst(ZERO, Jv); /* initialize Jv to zero */

  /* iterate over domain, computing Jacobian-vector products */
  for (i = 1; i < N - 1; i++)
  {
    /* Fill in Jacobian-vector product for Ju*v */
    Ju_v[i] = -v_w[i] * y_u[i] - y_w[i] * v_u[i] - v_u[i] +
              v_v[i] * y_u[i] * y_u[i] + TWO * y_v[i] * y_u[i] * v_u[i];

    /* Fill in Jacobian-vector product for Jv*v */
    Jv_v[i] = v_w[i] * y_u[i] + y_w[i] * v_u[i] - v_v[i] * y_u[i] * y_u[i] -
              TWO * y_v[i] * y_u[i] * v_u[i];

    /* Fill in Jacobian-vector product for Jw*v */
    Jw_v[i] = -v_w[i] / ep - v_w[i] * y_u[i] - y_w[i] * v_u[i];
  }

  /* enforce stationary boundaries */
  Ju_v[0] = Jv_v[0] = Jw_v[0] = ZERO;
  Ju_v[N - 1] = Jv_v[N - 1] = Jw_v[N - 1] = ZERO;

  return 0; /* Return with success */
}

/*-------------------------------
 * Private helper functions
 *-------------------------------*/

/* Check function return value...
    opt == 0 means SUNDIALS function allocates memory so check if
             returned NULL pointer
    opt == 1 means SUNDIALS function returns a flag so check if
             flag < 0
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

/*---- end of file ----*/
