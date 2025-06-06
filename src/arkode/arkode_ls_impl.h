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
 * Implementation header file for ARKODE's linear solver
 * interface.
 *--------------------------------------------------------------*/

#ifndef _ARKLS_IMPL_H
#define _ARKLS_IMPL_H

#include <arkode/arkode_ls.h>

#include "arkode_impl.h"

#ifdef __cplusplus /* wrapper to enable C++ usage */
extern "C" {
#endif

/*---------------------------------------------------------------
  ARKLS solver constants:

  ARKLS_MSBJ   default maximum number of steps between Jacobian /
               preconditioner evaluations

  ARKLS_EPLIN  default value for factor by which the tolerance
               on the nonlinear iteration is multiplied to get
               a tolerance on the linear iteration
  ---------------------------------------------------------------*/
#define ARKLS_MSBJ  51
#define ARKLS_EPLIN SUN_RCONST(0.05)

/*---------------------------------------------------------------
  Types: ARKLsMemRec, ARKLsMem

  The type ARKLsMem is pointer to a ARKLsMemRec.
  ---------------------------------------------------------------*/
typedef struct ARKLsMemRec
{
  /* Linear solver type information */
  sunbooleantype iterative;   /* is the solver iterative?    */
  sunbooleantype matrixbased; /* is a matrix structure used? */

  /* Jacobian construction & storage */
  sunbooleantype jacDQ; /* SUNTRUE if using internal DQ Jacobian approx. */
  ARKLsJacFn jac;       /* Jacobian routine to be called                 */
  void* J_data;         /* user data is passed to jac                    */
  sunbooleantype jbad;  /* heuristic suggestion for pset                 */

  /* Matrix-based solver, scale solution to account for change in gamma */
  sunbooleantype scalesol;

  /* Iterative solver tolerance */
  sunrealtype eplifac; /* nonlinear -> linear tol scaling factor        */
  sunrealtype nrmfac;  /* integrator -> LS norm conversion factor       */

  /* Linear solver, matrix and vector objects/pointers */
  SUNLinearSolver LS; /* generic linear solver object                  */
  SUNMatrix A;        /* A = M - gamma * df/dy                         */
  SUNMatrix savedJ;   /* savedJ = old Jacobian                         */
  N_Vector ytemp;     /* temp vector passed to jtimes and psolve       */
  N_Vector x;         /* solution vector used by SUNLinearSolver       */
  N_Vector ycur;      /* ptr to current y vector in ARKLs solve        */
  N_Vector fcur;      /* ptr to current fcur = fI(tcur, ycur)          */

  /* Statistics and associated parameters */
  long int msbj;     /* max num steps between jac/pset calls         */
  sunrealtype tcur;  /* 'time' for current ARKLs solve               */
  long int nje;      /* no. of calls to jac                          */
  long int nfeDQ;    /* no. of calls to f due to DQ Jacobian or J*v
                         approximations                               */
  long int nstlj;    /* value of nst at the last jac/pset call       */
  long int npe;      /* npe = total number of pset calls             */
  long int nli;      /* nli = total number of linear iterations      */
  long int nps;      /* nps = total number of psolve calls           */
  long int ncfl;     /* ncfl = total number of convergence failures  */
  long int njtsetup; /* njtsetup = total number of calls to jtsetup  */
  long int njtimes;  /* njtimes = total number of calls to jtimes    */
  sunrealtype tnlj;  /* tnlj = t_n at last jac/pset call             */

  /* Preconditioner computation
    (a) user-provided:
        - P_data == user_data
        - pfree == NULL (the user dealocates memory for user_data)
    (b) internal preconditioner module
        - P_data == arkode_mem
        - pfree == set by the prec. module and called in ARKodeFree  */
  ARKLsPrecSetupFn pset;
  ARKLsPrecSolveFn psolve;
  int (*pfree)(ARKodeMem ark_mem);
  void* P_data;

  /* Jacobian times vector computation
    (a) jtimes function provided by the user:
        - Jt_data == user_data
        - jtimesDQ == SUNFALSE
    (b) internal jtimes
        - Jt_data == arkode_mem
        - jtimesDQ == SUNTRUE   */
  sunbooleantype jtimesDQ;
  ARKLsJacTimesSetupFn jtsetup;
  ARKLsJacTimesVecFn jtimes;
  ARKRhsFn Jt_f;
  void* Jt_data;

  /* Linear system setup function
   * (a) user-provided linsys function:
   *     - user_linsys = SUNTRUE
   *     - A_data      = user_data
   * (b) internal linsys function:
   *     - user_linsys = SUNFALSE
   *     - A_data      = cvode_mem */
  sunbooleantype user_linsys;
  ARKLsLinSysFn linsys;
  void* A_data;

  int last_flag; /* last error flag returned by any function */

}* ARKLsMem;

/*---------------------------------------------------------------
  Types: ARKLsMassMemRec, ARKLsMassMem

  The type ARKLsMassMem is pointer to a ARKLsMassMemRec.
  ---------------------------------------------------------------*/
typedef struct ARKLsMassMemRec
{
  /* Linear solver type information */
  sunbooleantype iterative;   /* is the solver iterative?    */
  sunbooleantype matrixbased; /* is a matrix structure used? */

  /* Mass matrix construction & storage */
  ARKLsMassFn mass; /* user-provided mass matrix routine to call   */
  SUNMatrix M;      /* mass matrix structure                       */
  SUNMatrix M_lu;   /* mass matrix structure for LU decomposition  */
  void* M_data;     /* user data pointer */

  /* Iterative solver tolerance */
  sunrealtype eplifac; /* nonlinear -> linear tol scaling factor      */
  sunrealtype nrmfac;  /* integrator -> LS norm conversion factor     */

  /* Statistics and associated parameters */
  sunbooleantype time_dependent; /* flag whether M depends on t        */
  sunrealtype msetuptime;        /* "t" value at last msetup call      */
  long int nmsetups;             /* total # mass matrix-solver setups  */
  long int nmsolves;             /* total # mass matrix-solver solves  */
  long int nmtsetup;             /* total # calls to mtsetup           */
  long int nmtimes;              /* total # calls to mtimes            */
  long int nmvsetup;             /* total # calls to matvec setup      */
  long int npe;                  /* total # pset calls                 */
  long int nli;                  /* total # linear iterations          */
  long int nps;                  /* total # psolve calls               */
  long int ncfl;                 /* total # convergence failures       */

  /* Linear solver, matrix and vector objects/pointers */
  SUNLinearSolver LS; /* generic linear solver object                */
  N_Vector x;         /* solution vector used by SUNLinearSolver     */
  N_Vector ycur;      /* ptr to ARKODE current y vector              */

  /* Preconditioner computation
    (a) user-provided:
        - P_data == user_data
        - pfree == NULL (the user dealocates memory for user_data)
    (b) internal preconditioner module
        - P_data == arkode_mem
        - pfree == set by the prec. module and called in ARKodeFree  */
  ARKLsMassPrecSetupFn pset;
  ARKLsMassPrecSolveFn psolve;
  int (*pfree)(ARKodeMem ark_mem);
  void* P_data;

  /* Mass matrix times vector setup and product routines, data */
  ARKLsMassTimesSetupFn mtsetup;
  ARKLsMassTimesVecFn mtimes;
  void* mt_data;

  int last_flag; /* last error flag returned by any function    */

}* ARKLsMassMem;

/*---------------------------------------------------------------
  Prototypes of internal functions
  ---------------------------------------------------------------*/

/* Interface routines called by system SUNLinearSolver */
int arkLsATimes(void* arkode_mem, N_Vector v, N_Vector z);
int arkLsPSetup(void* arkode_mem);
int arkLsPSolve(void* arkode_mem, N_Vector r, N_Vector z, sunrealtype tol,
                int lr);

/* Interface routines called by mass SUNLinearSolver */
int arkLsMTimes(void* arkode_mem, N_Vector v, N_Vector z);
int arkLsMPSetup(void* arkode_mem);
int arkLsMPSolve(void* arkode_mem, N_Vector r, N_Vector z, sunrealtype tol,
                 int lr);

/* Difference quotient approximation for Jac times vector */
int arkLsDQJtimes(N_Vector v, N_Vector Jv, sunrealtype t, N_Vector y,
                  N_Vector fy, void* data, N_Vector work);

/* Difference-quotient Jacobian approximation routines */
int arkLsDQJac(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix Jac,
               void* data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);
int arkLsDenseDQJac(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix Jac,
                    ARKodeMem ark_mem, ARKLsMem arkls_mem, ARKRhsFn fi,
                    N_Vector tmp1);
int arkLsBandDQJac(sunrealtype t, N_Vector y, N_Vector fy, SUNMatrix Jac,
                   ARKodeMem ark_mem, ARKLsMem arkls_mem, ARKRhsFn fi,
                   N_Vector tmp1, N_Vector tmp2);

/* Generic linit/lsetup/lsolve/lfree interface routines for ARKODE to call */
int arkLsInitialize(ARKodeMem ark_mem);
int arkLsSetup(ARKodeMem ark_mem, int convfail, sunrealtype tpred,
               N_Vector ypred, N_Vector fpred, sunbooleantype* jcurPtr,
               N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3);
int arkLsSolve(ARKodeMem ark_mem, N_Vector b, sunrealtype tcur, N_Vector ycur,
               N_Vector fcur, sunrealtype eRnrm, int mnewt);
int arkLsFree(ARKodeMem ark_mem);

/* Generic minit/msetup/mmult/msolve/mfree routines for ARKODE to call */
int arkLsMassInitialize(ARKodeMem ark_mem);
int arkLsMassSetup(ARKodeMem ark_mem, sunrealtype t, N_Vector vtemp1,
                   N_Vector vtemp2, N_Vector vtemp3);
int arkLsMassMult(void* arkode_mem, N_Vector v, N_Vector Mv);
int arkLsMassSolve(ARKodeMem ark_mem, N_Vector b, sunrealtype nlscoef);
int arkLsMassFree(ARKodeMem ark_mem);

/* Auxiliary functions */
int arkLsInitializeCounters(ARKLsMem arkls_mem);
int arkLsInitializeMassCounters(ARKLsMassMem arkls_mem);
int arkLs_AccessARKODELMem(void* arkode_mem, const char* fname,
                           ARKodeMem* ark_mem, ARKLsMem* arkls_mem);
int arkLs_AccessLMem(ARKodeMem ark_mem, const char* fname, ARKLsMem* arkls_mem);
int arkLs_AccessARKODEMassMem(void* arkode_mem, const char* fname,
                              ARKodeMem* ark_mem, ARKLsMassMem* arkls_mem);
int arkLs_AccessMassMem(ARKodeMem ark_mem, const char* fname,
                        ARKLsMassMem* arkls_mem);

/* Set/get routines called by time-stepper module */
int arkLSSetLinearSolver(ARKodeMem ark_mem, SUNLinearSolver LS, SUNMatrix A);
int arkLSSetMassLinearSolver(ARKodeMem ark_mem, SUNLinearSolver LS, SUNMatrix M,
                             sunbooleantype time_dep);
int arkLSSetUserData(ARKodeMem ark_mem, void* user_data);
int arkLSSetMassUserData(ARKodeMem ark_mem, void* user_data);
int arkLSGetCurrentMassMatrix(ARKodeMem ark_mem, SUNMatrix* M);

/*---------------------------------------------------------------
  Error Messages
  ---------------------------------------------------------------*/
#define MSG_LS_ARKMEM_NULL  "Integrator memory is NULL."
#define MSG_LS_MEM_FAIL     "A memory request failed."
#define MSG_LS_BAD_NVECTOR  "A required vector operation is not implemented."
#define MSG_LS_BAD_LSTYPE   "Incompatible linear solver type."
#define MSG_LS_LMEM_NULL    "Linear solver memory is NULL."
#define MSG_LS_MASSMEM_NULL "Mass matrix solver memory is NULL."
#define MSG_LS_BAD_SIZES \
  "Illegal bandwidth parameter(s). Must have 0 <=  ml, mu <= N-1."

#define MSG_LS_PSET_FAILED \
  "The preconditioner setup routine failed in an unrecoverable manner."
#define MSG_LS_PSOLVE_FAILED \
  "The preconditioner solve routine failed in an unrecoverable manner."
#define MSG_LS_JTSETUP_FAILED \
  "The Jacobian x vector setup routine failed in an unrecoverable manner."
#define MSG_LS_JTIMES_FAILED \
  "The Jacobian x vector routine failed in an unrecoverable manner."
#define MSG_LS_MTSETUP_FAILED \
  "The mass matrix x vector setup routine failed in an unrecoverable manner."
#define MSG_LS_MTIMES_FAILED \
  "The mass matrix x vector routine failed in an unrecoverable manner."

#define MSG_LS_JACFUNC_FAILED \
  "The Jacobian routine failed in an unrecoverable manner."
#define MSG_LS_MASSFUNC_FAILED \
  "The mass matrix routine failed in an unrecoverable manner."
#define MSG_LS_SUNMAT_FAILED \
  "A SUNMatrix routine failed in an unrecoverable manner."

#ifdef __cplusplus
}
#endif

#endif
