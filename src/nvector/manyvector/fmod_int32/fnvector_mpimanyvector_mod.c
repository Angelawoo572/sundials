/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.0
 *
 * This file is not intended to be easily readable and contains a number of
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG
 * interface file instead.
 * ----------------------------------------------------------------------------- */

/* ---------------------------------------------------------------
 * Programmer(s): Auto-generated by swig.
 * ---------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2025, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * -------------------------------------------------------------*/

/* -----------------------------------------------------------------------------
 *  This section contains generic SWIG labels for method/variable
 *  declarations/attributes, and other compiler dependent labels.
 * ----------------------------------------------------------------------------- */

/* template workaround for compilers that cannot correctly implement the C++ standard */
#ifndef SWIGTEMPLATEDISAMBIGUATOR
# if defined(__SUNPRO_CC) && (__SUNPRO_CC <= 0x560)
#  define SWIGTEMPLATEDISAMBIGUATOR template
# elif defined(__HP_aCC)
/* Needed even with `aCC -AA' when `aCC -V' reports HP ANSI C++ B3910B A.03.55 */
/* If we find a maximum version that requires this, the test would be __HP_aCC <= 35500 for A.03.55 */
#  define SWIGTEMPLATEDISAMBIGUATOR template
# else
#  define SWIGTEMPLATEDISAMBIGUATOR
# endif
#endif

/* inline attribute */
#ifndef SWIGINLINE
# if defined(__cplusplus) || (defined(__GNUC__) && !defined(__STRICT_ANSI__))
#   define SWIGINLINE inline
# else
#   define SWIGINLINE
# endif
#endif

/* attribute recognised by some compilers to avoid 'unused' warnings */
#ifndef SWIGUNUSED
# if defined(__GNUC__)
#   if !(defined(__cplusplus)) || (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
#     define SWIGUNUSED __attribute__ ((__unused__))
#   else
#     define SWIGUNUSED
#   endif
# elif defined(__ICC)
#   define SWIGUNUSED __attribute__ ((__unused__))
# else
#   define SWIGUNUSED
# endif
#endif

#ifndef SWIG_MSC_UNSUPPRESS_4505
# if defined(_MSC_VER)
#   pragma warning(disable : 4505) /* unreferenced local function has been removed */
# endif
#endif

#ifndef SWIGUNUSEDPARM
# ifdef __cplusplus
#   define SWIGUNUSEDPARM(p)
# else
#   define SWIGUNUSEDPARM(p) p SWIGUNUSED
# endif
#endif

/* internal SWIG method */
#ifndef SWIGINTERN
# define SWIGINTERN static SWIGUNUSED
#endif

/* internal inline SWIG method */
#ifndef SWIGINTERNINLINE
# define SWIGINTERNINLINE SWIGINTERN SWIGINLINE
#endif

/* qualifier for exported *const* global data variables*/
#ifndef SWIGEXTERN
# ifdef __cplusplus
#   define SWIGEXTERN extern
# else
#   define SWIGEXTERN
# endif
#endif

/* exporting methods */
#if defined(__GNUC__)
#  if (__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#    ifndef GCC_HASCLASSVISIBILITY
#      define GCC_HASCLASSVISIBILITY
#    endif
#  endif
#endif

#ifndef SWIGEXPORT
# if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   if defined(STATIC_LINKED)
#     define SWIGEXPORT
#   else
#     define SWIGEXPORT __declspec(dllexport)
#   endif
# else
#   if defined(__GNUC__) && defined(GCC_HASCLASSVISIBILITY)
#     define SWIGEXPORT __attribute__ ((visibility("default")))
#   else
#     define SWIGEXPORT
#   endif
# endif
#endif

/* calling conventions for Windows */
#ifndef SWIGSTDCALL
# if defined(_WIN32) || defined(__WIN32__) || defined(__CYGWIN__)
#   define SWIGSTDCALL __stdcall
# else
#   define SWIGSTDCALL
# endif
#endif

/* Deal with Microsoft's attempt at deprecating C standard runtime functions */
#if !defined(SWIG_NO_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER) && !defined(_CRT_SECURE_NO_DEPRECATE)
# define _CRT_SECURE_NO_DEPRECATE
#endif

/* Deal with Microsoft's attempt at deprecating methods in the standard C++ library */
#if !defined(SWIG_NO_SCL_SECURE_NO_DEPRECATE) && defined(_MSC_VER) && !defined(_SCL_SECURE_NO_DEPRECATE)
# define _SCL_SECURE_NO_DEPRECATE
#endif

/* Deal with Apple's deprecated 'AssertMacros.h' from Carbon-framework */
#if defined(__APPLE__) && !defined(__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES)
# define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#endif

/* Intel's compiler complains if a variable which was never initialised is
 * cast to void, which is a common idiom which we use to indicate that we
 * are aware a variable isn't used.  So we just silence that warning.
 * See: https://github.com/swig/swig/issues/192 for more discussion.
 */
#ifdef __INTEL_COMPILER
# pragma warning disable 592
#endif

/*  Errors in SWIG */
#define  SWIG_UnknownError    	   -1
#define  SWIG_IOError        	   -2
#define  SWIG_RuntimeError   	   -3
#define  SWIG_IndexError     	   -4
#define  SWIG_TypeError      	   -5
#define  SWIG_DivisionByZero 	   -6
#define  SWIG_OverflowError  	   -7
#define  SWIG_SyntaxError    	   -8
#define  SWIG_ValueError     	   -9
#define  SWIG_SystemError    	   -10
#define  SWIG_AttributeError 	   -11
#define  SWIG_MemoryError    	   -12
#define  SWIG_NullReferenceError   -13




#include <assert.h>
#define SWIG_exception_impl(DECL, CODE, MSG, RETURNNULL) \
 { printf("In " DECL ": " MSG); assert(0); RETURNNULL; }


#include <stdio.h>
#if defined(_MSC_VER) || defined(__BORLANDC__) || defined(_WATCOM)
# ifndef snprintf
#  define snprintf _snprintf
# endif
#endif


/* Support for the `contract` feature.
 *
 * Note that RETURNNULL is first because it's inserted via a 'Replaceall' in
 * the fortran.cxx file.
 */
#define SWIG_contract_assert(RETURNNULL, EXPR, MSG) \
 if (!(EXPR)) { SWIG_exception_impl("$decl", SWIG_ValueError, MSG, RETURNNULL); } 


#define SWIGVERSION 0x040000 
#define SWIG_VERSION SWIGVERSION


#define SWIG_as_voidptr(a) (void *)((const void *)(a)) 
#define SWIG_as_voidptrptr(a) ((void)SWIG_as_voidptr(*a),(void**)(a)) 


#include "sundials/sundials_nvector.h"


#include "nvector/nvector_mpimanyvector.h"

SWIGEXPORT N_Vector _wrap_FN_VMake_MPIManyVector(int const *farg1, int32_t const *farg2, void *farg3, void *farg4) {
  N_Vector fresult ;
  MPI_Comm arg1 ;
  sunindextype arg2 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  SUNContext arg4 = (SUNContext) 0 ;
  N_Vector result;
  
#if SUNDIALS_MPI_ENABLED
  int flag = 0;
  MPI_Initialized(&flag);
  if(flag) {
    arg1 = MPI_Comm_f2c((MPI_Fint)(*farg1));
  } else {
    arg1 = SUN_COMM_NULL;
  }
#else
  arg1 = *farg1;
#endif
  arg2 = (sunindextype)(*farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (SUNContext)(farg4);
  result = (N_Vector)N_VMake_MPIManyVector(arg1,arg2,arg3,arg4);
  fresult = result;
  return fresult;
}


SWIGEXPORT N_Vector _wrap_FN_VNew_MPIManyVector(int32_t const *farg1, void *farg2, void *farg3) {
  N_Vector fresult ;
  sunindextype arg1 ;
  N_Vector *arg2 = (N_Vector *) 0 ;
  SUNContext arg3 = (SUNContext) 0 ;
  N_Vector result;
  
  arg1 = (sunindextype)(*farg1);
  arg2 = (N_Vector *)(farg2);
  arg3 = (SUNContext)(farg3);
  result = (N_Vector)N_VNew_MPIManyVector(arg1,arg2,arg3);
  fresult = result;
  return fresult;
}


SWIGEXPORT N_Vector _wrap_FN_VGetSubvector_MPIManyVector(N_Vector farg1, int32_t const *farg2) {
  N_Vector fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype arg2 ;
  N_Vector result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (sunindextype)(*farg2);
  result = (N_Vector)N_VGetSubvector_MPIManyVector(arg1,arg2);
  fresult = result;
  return fresult;
}


SWIGEXPORT int _wrap_FN_VSetSubvectorArrayPointer_MPIManyVector(double *farg1, N_Vector farg2, int32_t const *farg3) {
  int fresult ;
  sunrealtype *arg1 = (sunrealtype *) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunindextype arg3 ;
  SUNErrCode result;
  
  arg1 = (sunrealtype *)(farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (sunindextype)(*farg3);
  result = (SUNErrCode)N_VSetSubvectorArrayPointer_MPIManyVector(arg1,arg2,arg3);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int32_t _wrap_FN_VGetNumSubvectors_MPIManyVector(N_Vector farg1) {
  int32_t fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype result;
  
  arg1 = (N_Vector)(farg1);
  result = N_VGetNumSubvectors_MPIManyVector(arg1);
  fresult = (sunindextype)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VGetVectorID_MPIManyVector(N_Vector farg1) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector_ID result;
  
  arg1 = (N_Vector)(farg1);
  result = (N_Vector_ID)N_VGetVectorID_MPIManyVector(arg1);
  fresult = (int)(result);
  return fresult;
}


SWIGEXPORT void _wrap_FN_VPrint_MPIManyVector(N_Vector farg1) {
  N_Vector arg1 = (N_Vector) 0 ;
  
  arg1 = (N_Vector)(farg1);
  N_VPrint_MPIManyVector(arg1);
}


SWIGEXPORT void _wrap_FN_VPrintFile_MPIManyVector(N_Vector farg1, void *farg2) {
  N_Vector arg1 = (N_Vector) 0 ;
  FILE *arg2 = (FILE *) 0 ;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (FILE *)(farg2);
  N_VPrintFile_MPIManyVector(arg1,arg2);
}


SWIGEXPORT N_Vector _wrap_FN_VCloneEmpty_MPIManyVector(N_Vector farg1) {
  N_Vector fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector result;
  
  arg1 = (N_Vector)(farg1);
  result = (N_Vector)N_VCloneEmpty_MPIManyVector(arg1);
  fresult = result;
  return fresult;
}


SWIGEXPORT N_Vector _wrap_FN_VClone_MPIManyVector(N_Vector farg1) {
  N_Vector fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector result;
  
  arg1 = (N_Vector)(farg1);
  result = (N_Vector)N_VClone_MPIManyVector(arg1);
  fresult = result;
  return fresult;
}


SWIGEXPORT void _wrap_FN_VDestroy_MPIManyVector(N_Vector farg1) {
  N_Vector arg1 = (N_Vector) 0 ;
  
  arg1 = (N_Vector)(farg1);
  N_VDestroy_MPIManyVector(arg1);
}


SWIGEXPORT void _wrap_FN_VSpace_MPIManyVector(N_Vector farg1, int32_t *farg2, int32_t *farg3) {
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype *arg2 = (sunindextype *) 0 ;
  sunindextype *arg3 = (sunindextype *) 0 ;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (sunindextype *)(farg2);
  arg3 = (sunindextype *)(farg3);
  N_VSpace_MPIManyVector(arg1,arg2,arg3);
}


SWIGEXPORT int _wrap_FN_VGetCommunicator_MPIManyVector(N_Vector farg1) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  MPI_Comm result;
  
  arg1 = (N_Vector)(farg1);
  result = N_VGetCommunicator_MPIManyVector(arg1);
#if SUNDIALS_MPI_ENABLED
  int flag = 0;
  MPI_Initialized(&flag);
  if(flag) {
    fresult = (int)(MPI_Comm_c2f(result));
  } else {
    fresult = 0;
  }
#else
  fresult = result;
#endif
  return fresult;
}


SWIGEXPORT int32_t _wrap_FN_VGetLength_MPIManyVector(N_Vector farg1) {
  int32_t fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype result;
  
  arg1 = (N_Vector)(farg1);
  result = N_VGetLength_MPIManyVector(arg1);
  fresult = (sunindextype)(result);
  return fresult;
}


SWIGEXPORT int32_t _wrap_FN_VGetSubvectorLocalLength_MPIManyVector(N_Vector farg1, int32_t const *farg2) {
  int32_t fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype arg2 ;
  sunindextype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (sunindextype)(*farg2);
  result = N_VGetSubvectorLocalLength_MPIManyVector(arg1,arg2);
  fresult = (sunindextype)(result);
  return fresult;
}


SWIGEXPORT void _wrap_FN_VLinearSum_MPIManyVector(double const *farg1, N_Vector farg2, double const *farg3, N_Vector farg4, N_Vector farg5) {
  sunrealtype arg1 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype arg3 ;
  N_Vector arg4 = (N_Vector) 0 ;
  N_Vector arg5 = (N_Vector) 0 ;
  
  arg1 = (sunrealtype)(*farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (sunrealtype)(*farg3);
  arg4 = (N_Vector)(farg4);
  arg5 = (N_Vector)(farg5);
  N_VLinearSum_MPIManyVector(arg1,arg2,arg3,arg4,arg5);
}


SWIGEXPORT void _wrap_FN_VConst_MPIManyVector(double const *farg1, N_Vector farg2) {
  sunrealtype arg1 ;
  N_Vector arg2 = (N_Vector) 0 ;
  
  arg1 = (sunrealtype)(*farg1);
  arg2 = (N_Vector)(farg2);
  N_VConst_MPIManyVector(arg1,arg2);
}


SWIGEXPORT void _wrap_FN_VProd_MPIManyVector(N_Vector farg1, N_Vector farg2, N_Vector farg3) {
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  N_VProd_MPIManyVector(arg1,arg2,arg3);
}


SWIGEXPORT void _wrap_FN_VDiv_MPIManyVector(N_Vector farg1, N_Vector farg2, N_Vector farg3) {
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  N_VDiv_MPIManyVector(arg1,arg2,arg3);
}


SWIGEXPORT void _wrap_FN_VScale_MPIManyVector(double const *farg1, N_Vector farg2, N_Vector farg3) {
  sunrealtype arg1 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  
  arg1 = (sunrealtype)(*farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  N_VScale_MPIManyVector(arg1,arg2,arg3);
}


SWIGEXPORT void _wrap_FN_VAbs_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  N_VAbs_MPIManyVector(arg1,arg2);
}


SWIGEXPORT void _wrap_FN_VInv_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  N_VInv_MPIManyVector(arg1,arg2);
}


SWIGEXPORT void _wrap_FN_VAddConst_MPIManyVector(N_Vector farg1, double const *farg2, N_Vector farg3) {
  N_Vector arg1 = (N_Vector) 0 ;
  sunrealtype arg2 ;
  N_Vector arg3 = (N_Vector) 0 ;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (sunrealtype)(*farg2);
  arg3 = (N_Vector)(farg3);
  N_VAddConst_MPIManyVector(arg1,arg2,arg3);
}


SWIGEXPORT double _wrap_FN_VDotProd_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (sunrealtype)N_VDotProd_MPIManyVector(arg1,arg2);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VMaxNorm_MPIManyVector(N_Vector farg1) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  result = (sunrealtype)N_VMaxNorm_MPIManyVector(arg1);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VWrmsNorm_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (sunrealtype)N_VWrmsNorm_MPIManyVector(arg1,arg2);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VWrmsNormMask_MPIManyVector(N_Vector farg1, N_Vector farg2, N_Vector farg3) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  result = (sunrealtype)N_VWrmsNormMask_MPIManyVector(arg1,arg2,arg3);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VMin_MPIManyVector(N_Vector farg1) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  result = (sunrealtype)N_VMin_MPIManyVector(arg1);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VWL2Norm_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (sunrealtype)N_VWL2Norm_MPIManyVector(arg1,arg2);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VL1Norm_MPIManyVector(N_Vector farg1) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  result = (sunrealtype)N_VL1Norm_MPIManyVector(arg1);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT void _wrap_FN_VCompare_MPIManyVector(double const *farg1, N_Vector farg2, N_Vector farg3) {
  sunrealtype arg1 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  
  arg1 = (sunrealtype)(*farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  N_VCompare_MPIManyVector(arg1,arg2,arg3);
}


SWIGEXPORT int _wrap_FN_VInvTest_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  int result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (int)N_VInvTest_MPIManyVector(arg1,arg2);
  fresult = (int)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VConstrMask_MPIManyVector(N_Vector farg1, N_Vector farg2, N_Vector farg3) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  int result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  result = (int)N_VConstrMask_MPIManyVector(arg1,arg2,arg3);
  fresult = (int)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VMinQuotient_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (sunrealtype)N_VMinQuotient_MPIManyVector(arg1,arg2);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VLinearCombination_MPIManyVector(int const *farg1, double *farg2, void *farg3, N_Vector farg4) {
  int fresult ;
  int arg1 ;
  sunrealtype *arg2 = (sunrealtype *) 0 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  N_Vector arg4 = (N_Vector) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (sunrealtype *)(farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (N_Vector)(farg4);
  result = (SUNErrCode)N_VLinearCombination_MPIManyVector(arg1,arg2,arg3,arg4);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VScaleAddMulti_MPIManyVector(int const *farg1, double *farg2, N_Vector farg3, void *farg4, void *farg5) {
  int fresult ;
  int arg1 ;
  sunrealtype *arg2 = (sunrealtype *) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  N_Vector *arg4 = (N_Vector *) 0 ;
  N_Vector *arg5 = (N_Vector *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (sunrealtype *)(farg2);
  arg3 = (N_Vector)(farg3);
  arg4 = (N_Vector *)(farg4);
  arg5 = (N_Vector *)(farg5);
  result = (SUNErrCode)N_VScaleAddMulti_MPIManyVector(arg1,arg2,arg3,arg4,arg5);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VDotProdMulti_MPIManyVector(int const *farg1, N_Vector farg2, void *farg3, double *farg4) {
  int fresult ;
  int arg1 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  sunrealtype *arg4 = (sunrealtype *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (sunrealtype *)(farg4);
  result = (SUNErrCode)N_VDotProdMulti_MPIManyVector(arg1,arg2,arg3,arg4);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VDotProdMultiLocal_MPIManyVector(int const *farg1, N_Vector farg2, void *farg3, double *farg4) {
  int fresult ;
  int arg1 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  sunrealtype *arg4 = (sunrealtype *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (sunrealtype *)(farg4);
  result = (SUNErrCode)N_VDotProdMultiLocal_MPIManyVector(arg1,arg2,arg3,arg4);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VDotProdMultiAllReduce_MPIManyVector(int const *farg1, N_Vector farg2, double *farg3) {
  int fresult ;
  int arg1 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype *arg3 = (sunrealtype *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (sunrealtype *)(farg3);
  result = (SUNErrCode)N_VDotProdMultiAllReduce_MPIManyVector(arg1,arg2,arg3);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VLinearSumVectorArray_MPIManyVector(int const *farg1, double const *farg2, void *farg3, double const *farg4, void *farg5, void *farg6) {
  int fresult ;
  int arg1 ;
  sunrealtype arg2 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  sunrealtype arg4 ;
  N_Vector *arg5 = (N_Vector *) 0 ;
  N_Vector *arg6 = (N_Vector *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (sunrealtype)(*farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (sunrealtype)(*farg4);
  arg5 = (N_Vector *)(farg5);
  arg6 = (N_Vector *)(farg6);
  result = (SUNErrCode)N_VLinearSumVectorArray_MPIManyVector(arg1,arg2,arg3,arg4,arg5,arg6);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VScaleVectorArray_MPIManyVector(int const *farg1, double *farg2, void *farg3, void *farg4) {
  int fresult ;
  int arg1 ;
  sunrealtype *arg2 = (sunrealtype *) 0 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  N_Vector *arg4 = (N_Vector *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (sunrealtype *)(farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (N_Vector *)(farg4);
  result = (SUNErrCode)N_VScaleVectorArray_MPIManyVector(arg1,arg2,arg3,arg4);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VConstVectorArray_MPIManyVector(int const *farg1, double const *farg2, void *farg3) {
  int fresult ;
  int arg1 ;
  sunrealtype arg2 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (sunrealtype)(*farg2);
  arg3 = (N_Vector *)(farg3);
  result = (SUNErrCode)N_VConstVectorArray_MPIManyVector(arg1,arg2,arg3);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VWrmsNormVectorArray_MPIManyVector(int const *farg1, void *farg2, void *farg3, double *farg4) {
  int fresult ;
  int arg1 ;
  N_Vector *arg2 = (N_Vector *) 0 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  sunrealtype *arg4 = (sunrealtype *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (N_Vector *)(farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (sunrealtype *)(farg4);
  result = (SUNErrCode)N_VWrmsNormVectorArray_MPIManyVector(arg1,arg2,arg3,arg4);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VWrmsNormMaskVectorArray_MPIManyVector(int const *farg1, void *farg2, void *farg3, N_Vector farg4, double *farg5) {
  int fresult ;
  int arg1 ;
  N_Vector *arg2 = (N_Vector *) 0 ;
  N_Vector *arg3 = (N_Vector *) 0 ;
  N_Vector arg4 = (N_Vector) 0 ;
  sunrealtype *arg5 = (sunrealtype *) 0 ;
  SUNErrCode result;
  
  arg1 = (int)(*farg1);
  arg2 = (N_Vector *)(farg2);
  arg3 = (N_Vector *)(farg3);
  arg4 = (N_Vector)(farg4);
  arg5 = (sunrealtype *)(farg5);
  result = (SUNErrCode)N_VWrmsNormMaskVectorArray_MPIManyVector(arg1,arg2,arg3,arg4,arg5);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VDotProdLocal_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (sunrealtype)N_VDotProdLocal_MPIManyVector(arg1,arg2);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VMaxNormLocal_MPIManyVector(N_Vector farg1) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  result = (sunrealtype)N_VMaxNormLocal_MPIManyVector(arg1);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VMinLocal_MPIManyVector(N_Vector farg1) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  result = (sunrealtype)N_VMinLocal_MPIManyVector(arg1);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VL1NormLocal_MPIManyVector(N_Vector farg1) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  result = (sunrealtype)N_VL1NormLocal_MPIManyVector(arg1);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VWSqrSumLocal_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (sunrealtype)N_VWSqrSumLocal_MPIManyVector(arg1,arg2);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VWSqrSumMaskLocal_MPIManyVector(N_Vector farg1, N_Vector farg2, N_Vector farg3) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  result = (sunrealtype)N_VWSqrSumMaskLocal_MPIManyVector(arg1,arg2,arg3);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VInvTestLocal_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  int result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (int)N_VInvTestLocal_MPIManyVector(arg1,arg2);
  fresult = (int)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VConstrMaskLocal_MPIManyVector(N_Vector farg1, N_Vector farg2, N_Vector farg3) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  N_Vector arg3 = (N_Vector) 0 ;
  int result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  arg3 = (N_Vector)(farg3);
  result = (int)N_VConstrMaskLocal_MPIManyVector(arg1,arg2,arg3);
  fresult = (int)(result);
  return fresult;
}


SWIGEXPORT double _wrap_FN_VMinQuotientLocal_MPIManyVector(N_Vector farg1, N_Vector farg2) {
  double fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  N_Vector arg2 = (N_Vector) 0 ;
  sunrealtype result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (N_Vector)(farg2);
  result = (sunrealtype)N_VMinQuotientLocal_MPIManyVector(arg1,arg2);
  fresult = (sunrealtype)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VBufSize_MPIManyVector(N_Vector farg1, int32_t *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype *arg2 = (sunindextype *) 0 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (sunindextype *)(farg2);
  result = (SUNErrCode)N_VBufSize_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VBufPack_MPIManyVector(N_Vector farg1, void *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  void *arg2 = (void *) 0 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (void *)(farg2);
  result = (SUNErrCode)N_VBufPack_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VBufUnpack_MPIManyVector(N_Vector farg1, void *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  void *arg2 = (void *) 0 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (void *)(farg2);
  result = (SUNErrCode)N_VBufUnpack_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableFusedOps_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableFusedOps_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableLinearCombination_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableLinearCombination_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableScaleAddMulti_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableScaleAddMulti_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableDotProdMulti_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableDotProdMulti_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableLinearSumVectorArray_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableLinearSumVectorArray_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableScaleVectorArray_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableScaleVectorArray_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableConstVectorArray_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableConstVectorArray_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableWrmsNormVectorArray_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableWrmsNormVectorArray_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableWrmsNormMaskVectorArray_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableWrmsNormMaskVectorArray_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}


SWIGEXPORT int _wrap_FN_VEnableDotProdMultiLocal_MPIManyVector(N_Vector farg1, int const *farg2) {
  int fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  int arg2 ;
  SUNErrCode result;
  
  arg1 = (N_Vector)(farg1);
  arg2 = (int)(*farg2);
  result = (SUNErrCode)N_VEnableDotProdMultiLocal_MPIManyVector(arg1,arg2);
  fresult = (SUNErrCode)(result);
  return fresult;
}



#ifdef SUNDIALS_INT32_T
SWIGEXPORT double * _wrap_FN_VGetSubvectorArrayPointer_MPIManyVector(N_Vector farg1, int32_t const *farg2) {
  double * fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype arg2 ;
  sunrealtype *result = 0 ;

  arg1 = (N_Vector)(farg1);
  arg2 = (sunindextype)(*farg2);
  result = (sunrealtype *)N_VGetSubvectorArrayPointer_MPIManyVector(arg1,arg2);
  fresult = result;
  return fresult;
}
#else
SWIGEXPORT double * _wrap_FN_VGetSubvectorArrayPointer_MPIManyVector(N_Vector farg1, int64_t const *farg2) {
  double * fresult ;
  N_Vector arg1 = (N_Vector) 0 ;
  sunindextype arg2 ;
  sunrealtype *result = 0 ;

  arg1 = (N_Vector)(farg1);
  arg2 = (sunindextype)(*farg2);
  result = (sunrealtype *)N_VGetSubvectorArrayPointer_MPIManyVector(arg1,arg2);
  fresult = result;
  return fresult;
}
#endif


