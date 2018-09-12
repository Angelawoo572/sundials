/*
 * -----------------------------------------------------------------
 * Programmer(s): Slaven Peles @ LLNL
 * -----------------------------------------------------------------
 * Example problem:
 *
 * -----------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <cvode/cvode.h>
#include <sunlinsol/sunlinsol_spgmr.h> /* access to SPGMR SUNLinearSolver        */
#include <cvode/cvode_spils.h>         /* access to CVSpils interface            */
#include <nvector/nvector_cuda.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>

#define SUNDIALS_HAVE_POSIX_TIMERS
#define _POSIX_TIMERS

#if defined( SUNDIALS_HAVE_POSIX_TIMERS) && defined(_POSIX_TIMERS)
#include <time.h>
#include <unistd.h>
#endif


typedef struct _UserData
{
  sunindextype Nx;
  sunindextype Ny;
  sunindextype NEQ;

  int block;
  int grid;

  realtype hx;
  realtype hy;

  realtype hordc;
  realtype verdc;
  realtype horac;
  realtype verac;
  realtype reacc;

} *UserData;

//typedef _UserData *UserData;

/* User defined functions */

static N_Vector SetIC(MPI_Comm comm, UserData data);
static UserData SetUserData(int argc, char *argv[]);
static int RHS(realtype t, N_Vector u, N_Vector udot, void *userData);
static int Jtv(N_Vector v, N_Vector Jv, realtype t, N_Vector u, N_Vector fu, void *userData, N_Vector tmp);


/* Private Helper Functions */

static void PrintOutput(void *cvode_mem, N_Vector u, realtype t);
static void PrintFinalStats(void *cvode_mem);
static int check_flag(void *flagvalue, const char *funcname, int opt);


/* private functions */
static double get_time();

/*
 *-------------------------------
 * Main Program
 *-------------------------------
 */

int main(int argc, char *argv[])
{
  realtype abstol, reltol, t;
  //realtype tout;
  const realtype t_in = 0.0;
  const realtype t_fi = 0.1;
  N_Vector u;
  UserData data;
  SUNLinearSolver LS;
  void *cvode_mem;
  //int iout;
  int flag;
  int npes;
  MPI_Comm comm;

  u = NULL;
  data = NULL;
  cvode_mem = NULL;

#if SUNDIALS_MPI_ENABLED
#warning "MPI ENABLED"
  MPI_Init(&argc, &argv);
  comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &npes);
#else
#warning "MPI NOT ENABLED"
  comm = 0;
  npes = 1;
#endif

  if (npes != 1) {
    printf("Warning: This test case works only with one MPI rank!");
    return -1;
  }

  /* Allocate memory, set problem data and initial values */
  data = SetUserData(argc, argv);
  u = SetIC(comm, data);

  reltol = RCONST(1.0e-5);         /* scalar relative tolerance */
  abstol = reltol * RCONST(100.0); /* scalar absolute tolerance */

  /* Call CVodeCreate to create the solver memory and specify the
   * Backward Differentiation Formula and the use of a Newton iteration */
  cvode_mem = CVodeCreate(CV_BDF, CV_NEWTON);
  if(check_flag((void *)cvode_mem, "CVodeCreate", 0)) return(1);

  /* Set the pointer to user-defined data */
  flag = CVodeSetUserData(cvode_mem, data);
  if(check_flag(&flag, "CVodeSetUserData", 1)) return(1);

  /* Call CVodeInit to initialize the integrator memory and specify the
   * user's right hand side function in u'=f(t,u), the inital time T0, and
   * the initial dependent variable vector u. */
  flag = CVodeInit(cvode_mem, RHS, t_in, u);
  if(check_flag(&flag, "CVodeInit", 1)) return(1);

  /* Call CVodeSStolerances to specify the scalar relative tolerance
   * and scalar absolute tolerances */
  flag = CVodeSStolerances(cvode_mem, reltol, abstol);
  if (check_flag(&flag, "CVodeSStolerances", 1)) return(1);

  /* Create SPGMR solver structure without preconditioning
   * and the maximum Krylov dimension maxl */
  LS = SUNSPGMR(u, PREC_NONE, 0);
  if(check_flag(&flag, "SUNSPGMR", 1)) return(1);

  /* Set CVSpils linear solver to LS */
  flag = CVSpilsSetLinearSolver(cvode_mem, LS);
  if(check_flag(&flag, "CVSpilsSetLinearSolver", 1)) return(1);

  /* set the JAcobian-times-vector function */
  flag = CVSpilsSetJacTimes(cvode_mem, NULL, Jtv);
  if(check_flag(&flag, "CVSpilsSetJacTimes", 1)) return(1);


  printf("Solving diffusion-advection-reaction problem with %ld unknowns...\n", data->NEQ);

  double start_time, stop_time;
  start_time = get_time();
  flag = CVode(cvode_mem, t_fi, u, &t, CV_NORMAL);
  cudaDeviceSynchronize(); /* Ensures execution time is captured correctly */
  stop_time = get_time();
  PrintOutput(cvode_mem, u, t);
  if(check_flag(&flag, "CVode", 1))
    return (-1);

  printf("Computation successful!\n");
  //printf("Execution time = %g\n", stop_time - start_time);
  printf("L2 norm = %14.6e\n", SUNRsqrt(N_VDotProd(u,u)));

  PrintFinalStats(cvode_mem);

  /* Free memory */
  N_VDestroy(u);
  free(data);
  CVodeFree(&cvode_mem);

#if SUNDIALS_MPI_ENABLED
  MPI_Finalize();
#endif

  return(0);
}


/*
 *-------------------------------
 * User defined functions
 *-------------------------------
 */

N_Vector SetIC(MPI_Comm comm, UserData data)
{
  const sunindextype Nx = data->Nx;
  const realtype hx = data->hx;
  const realtype hy = data->hy;

  N_Vector y     = N_VNew_Cuda(data->NEQ);
  realtype *ydat = N_VGetHostArrayPointer_Cuda(y);
  sunindextype i, j, index;

  for (index = 0; index < data->NEQ; ++index)
  {
    j = index/Nx;
    i = index%Nx;

    realtype y = j * hy;
    realtype x = i * hx;
    realtype tmp = (1 - x) * x * (1 - y) * y;
    ydat[index] = (256.0 * tmp * tmp) + 0.3;
  }
  N_VCopyToDevice_Cuda(y);
  return y;
}

UserData SetUserData(int argc, char *argv[])
{
  sunindextype dimX = 70; /* Default grid size */
  sunindextype dimY = 80;
  const realtype diffusionConst =  0.01;
  const realtype advectionConst = -10.0;
  const realtype reactionConst  = 100.0;

  const int maxthreads = 256;

  /* Allocate user data structure */
  UserData ud = (UserData) malloc(sizeof *ud);
  if(check_flag((void*) ud, "AllocUserData", 2)) return(NULL);

  /* Set grid size */
  if (argc == 3) {
    dimX = strtol(argv[1], (char**) NULL, 10);
    dimY = strtol(argv[2], (char**) NULL, 10);
  }

  ud->Nx = dimX + 1;
  ud->Ny = dimY + 1;
  ud->NEQ = ud->Nx * ud->Ny;

  /* Set thread partitioning for GPU execution */
  ud->block = maxthreads;
  ud->grid  = (ud->NEQ + maxthreads - 1) / maxthreads;

  /* Compute cell sizes */
  ud->hx = 1.0/((realtype) dimX);
  ud->hy = 1.0/((realtype) dimY);

  /* Compute diffusion coefficients */
  ud->hordc = diffusionConst/(ud->hx * ud->hx);
  ud->verdc = diffusionConst/(ud->hy * ud->hy);

  /* Compute advection coefficient */
  ud->horac = advectionConst/(2.0 * ud->hx);
  ud->verac = advectionConst/(2.0 * ud->hy);

  /* Set reaction coefficient */
  ud->reacc = reactionConst;

  return ud;
}


__global__ void phiKernel(const realtype *u, realtype *result, sunindextype NEQ, sunindextype Nx, sunindextype Ny,
                          realtype hordc, realtype verdc, realtype horac, realtype verac)
{
  sunindextype i, j, index;

  /* Loop over all grid points. */
  index = blockDim.x * blockIdx.x + threadIdx.x;

  realtype uij;
  realtype ult;
  realtype urt;
  realtype uup;
  realtype udn;

  realtype hdiff;
  realtype vdiff;
  realtype hadv;
  realtype vadv;

  if (index < NEQ)
  {
    i = index%Nx;
    j = index/Nx;

    uij = u[index];

    ult = (i == 0)    ? u[index + 1]  : u[index - 1];
    urt = (i == Nx-1) ? u[index - 1]  : u[index + 1];
    udn = (j == 0)    ? u[index + Nx] : u[index - Nx];
    uup = (j == Ny-1) ? u[index - Nx] : u[index + Nx];

    hdiff =  hordc*(ult -2.0*uij + urt);
    vdiff =  verdc*(udn -2.0*uij + uup);
    hadv  = -horac*(urt - ult);
    vadv  = -verac*(uup - udn);

    result[index] = hdiff + vdiff + hadv + vadv;
  }

}


__global__ void rhsKernel(const realtype* u, realtype* udot, sunindextype N, realtype reacc)
{
  const realtype a = -1.0 / 2.0;

  /* Loop over all grid points. */
  sunindextype tid = blockDim.x * blockIdx.x + threadIdx.x;

  if(tid < N)
  {
    udot[tid] += (reacc*(u[tid] + a)*(1.0 - u[tid])*u[tid]);
  }

}


int RHS(realtype t, N_Vector u, N_Vector udot, void *user_data)
{
  UserData data = (UserData) user_data;
  const int grid  = data->grid;
  const int block = data->block;

  const realtype *udata = N_VGetDeviceArrayPointer_Cuda(u);
  realtype *udotdata    = N_VGetDeviceArrayPointer_Cuda(udot);

  phiKernel<<<grid,block>>>(udata, udotdata, data->NEQ, data->Nx, data->Ny, data->hordc, data->verdc, data->horac, data->verac);
  rhsKernel<<<grid,block>>>(udata, udotdata, data->NEQ, data->reacc);

  return 0;
}

__global__ void jtvKernel(const realtype* v, realtype* Jv, const realtype* u, sunindextype N, realtype reacc)
{
  const realtype a = -1.0 / 2.0;

  /* Loop over all grid points. */
  sunindextype tid = blockDim.x * blockIdx.x + threadIdx.x;

  if(tid < N)
  {
    Jv[tid] += reacc*(3.0*u[tid] + a - 3.0*u[tid]*u[tid])*v[tid]; // original
  }

}


int Jtv(N_Vector v, N_Vector Jv, realtype t, N_Vector u, N_Vector fu, void *user_data, N_Vector tmp)
{
  UserData data = (UserData) user_data;
  const int grid  = data->grid;
  const int block = data->block;

  const realtype *udata  = N_VGetDeviceArrayPointer_Cuda(u);
  const realtype *vdata  = N_VGetDeviceArrayPointer_Cuda(v);
  realtype *Jvdata       = N_VGetDeviceArrayPointer_Cuda(Jv);

  phiKernel<<<grid,block>>>(vdata, Jvdata, data->NEQ, data->Nx, data->Ny, data->hordc, data->verdc, data->horac, data->verac);
  jtvKernel<<<grid,block>>>(vdata, Jvdata, udata, data->NEQ, data->reacc);

  return 0;
}



/*
 *-------------------------------
 * Private helper functions
 *-------------------------------
 */


/* Print current t, step count, order, stepsize, and sampled c1,c2 values */

static void PrintOutput(void *cvode_mem, N_Vector u, realtype t)
{
  long int nst;
  int qu, flag;
  realtype hu;
  //realtype *udata;

  //udata = N_VGetArrayPointer_Serial(u);

  flag = CVodeGetNumSteps(cvode_mem, &nst);
  check_flag(&flag, "CVodeGetNumSteps", 1);
  flag = CVodeGetLastOrder(cvode_mem, &qu);
  check_flag(&flag, "CVodeGetLastOrder", 1);
  flag = CVodeGetLastStep(cvode_mem, &hu);
  check_flag(&flag, "CVodeGetLastStep", 1);

}

/* Get and print final statistics */

static void PrintFinalStats(void *cvode_mem)
{
  long lenrw, leniw ;
  long lenrwLS, leniwLS;
  long int nst, nfe, nsetups, nni, ncfn, netf;
  long int nli, npe, nps, ncfl, nfeLS;
  int flag;

  flag = CVodeGetWorkSpace(cvode_mem, &lenrw, &leniw);
  check_flag(&flag, "CVodeGetWorkSpace", 1);
  flag = CVodeGetNumSteps(cvode_mem, &nst);
  check_flag(&flag, "CVodeGetNumSteps", 1);
  flag = CVodeGetNumRhsEvals(cvode_mem, &nfe);
  check_flag(&flag, "CVodeGetNumRhsEvals", 1);
  flag = CVodeGetNumLinSolvSetups(cvode_mem, &nsetups);
  check_flag(&flag, "CVodeGetNumLinSolvSetups", 1);
  flag = CVodeGetNumErrTestFails(cvode_mem, &netf);
  check_flag(&flag, "CVodeGetNumErrTestFails", 1);
  flag = CVodeGetNumNonlinSolvIters(cvode_mem, &nni);
  check_flag(&flag, "CVodeGetNumNonlinSolvIters", 1);
  flag = CVodeGetNumNonlinSolvConvFails(cvode_mem, &ncfn);
  check_flag(&flag, "CVodeGetNumNonlinSolvConvFails", 1);

  flag = CVSpilsGetWorkSpace(cvode_mem, &lenrwLS, &leniwLS);
  check_flag(&flag, "CVSpilsGetWorkSpace", 1);
  flag = CVSpilsGetNumLinIters(cvode_mem, &nli);
  check_flag(&flag, "CVSpilsGetNumLinIters", 1);
  flag = CVSpilsGetNumPrecEvals(cvode_mem, &npe);
  check_flag(&flag, "CVSpilsGetNumPrecEvals", 1);
  flag = CVSpilsGetNumPrecSolves(cvode_mem, &nps);
  check_flag(&flag, "CVSpilsGetNumPrecSolves", 1);
  flag = CVSpilsGetNumConvFails(cvode_mem, &ncfl);
  check_flag(&flag, "CVSpilsGetNumConvFails", 1);
  flag = CVSpilsGetNumRhsEvals(cvode_mem, &nfeLS);
  check_flag(&flag, "CVSpilsGetNumRhsEvals", 1);

  printf("\nFinal Statistics.. \n\n");
  printf("nst     = %5ld\n"                  , nst);
  printf("nfe     = %5ld     nfeLS   = %5ld\n"  , nfe, nfeLS);
  printf("nni     = %5ld     nli     = %5ld\n"  , nni, nli);
  printf("nsetups = %5ld     netf    = %5ld\n"  , nsetups, netf);
  printf("npe     = %5ld     nps     = %5ld\n"  , npe, nps);
  printf("ncfn    = %5ld     ncfl    = %5ld\n\n", ncfn, ncfl);
}

/* Check function return value...
     opt == 0 means SUNDIALS function allocates memory so check if
              returned NULL pointer
     opt == 1 means SUNDIALS function returns a flag so check if
              flag >= 0
     opt == 2 means function allocates memory so check if returned
              NULL pointer */

static int check_flag(void *flagvalue, const char *funcname, int opt)
{
  int *errflag;

  /* Check if SUNDIALS function returned NULL pointer - no memory allocated */
  if (opt == 0 && flagvalue == NULL) {
    fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return(1); }

  /* Check if flag < 0 */
  else if (opt == 1) {
    errflag = (int *) flagvalue;
    if (*errflag < 0) {
      fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n",
              funcname, *errflag);
      return(1); }}

  /* Check if function returned NULL pointer - no memory allocated */
  else if (opt == 2 && flagvalue == NULL) {
    fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
            funcname);
    return(1); }

  return(0);
}

/* ======================================================================
 * Timing functions
 * ====================================================================*/

#if defined( SUNDIALS_HAVE_POSIX_TIMERS) && defined(_POSIX_TIMERS)
time_t base_time_tv_sec = 0; /* Base time; makes time values returned
                                by get_time easier to read when
                                printed since they will be zero
                                based.
                              */
#else
#warning "No posix timers!\n"
#endif

void SetTiming(int onoff)
{
   //print_time = onoff;

#if defined( SUNDIALS_HAVE_POSIX_TIMERS) && defined(_POSIX_TIMERS)
  struct timespec spec;
  clock_gettime( CLOCK_MONOTONIC_RAW, &spec );
  base_time_tv_sec = spec.tv_sec;
#endif
}

/* ----------------------------------------------------------------------
 * Timer
 * --------------------------------------------------------------------*/
static double get_time()
{
#if defined( SUNDIALS_HAVE_POSIX_TIMERS) && defined(_POSIX_TIMERS)
  struct timespec spec;
  clock_gettime( CLOCK_MONOTONIC_RAW, &spec );
  double time = (double)(spec.tv_sec - base_time_tv_sec) + ((double)(spec.tv_nsec) / 1E9);
#else
  double time = 0;
#endif
  return time;
}
