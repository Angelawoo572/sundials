/* -----------------------------------------------------------------
 * Programmer(s): Daniel R. Reynolds @ SMU,
 *      Alan C. Hindmarsh, Radu Serban and Aaron Collier @ LLNL
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
 * This is the header file for the IDABBDPRE module, for a
 * band-block-diagonal preconditioner, i.e. a block-diagonal
 * matrix with banded blocks.
 * -----------------------------------------------------------------*/

#ifndef _IDASBBDPRE_H
#define _IDASBBDPRE_H

#include <sundials/sundials_nvector.h>

#ifdef __cplusplus /* wrapper to enable C++ usage */
extern "C" {
#endif

/*-----------------
  FORWARD PROBLEMS
  -----------------*/

/* User-supplied function Types */

typedef int (*IDABBDLocalFn)(sunindextype Nlocal, sunrealtype tt, N_Vector yy,
                             N_Vector yp, N_Vector gval, void* user_data);

typedef int (*IDABBDCommFn)(sunindextype Nlocal, sunrealtype tt, N_Vector yy,
                            N_Vector yp, void* user_data);

/* Exported Functions */

SUNDIALS_EXPORT int IDABBDPrecInit(void* ida_mem, sunindextype Nlocal,
                                   sunindextype mudq, sunindextype mldq,
                                   sunindextype mukeep, sunindextype mlkeep,
                                   sunrealtype dq_rel_yy, IDABBDLocalFn Gres,
                                   IDABBDCommFn Gcomm);

SUNDIALS_EXPORT int IDABBDPrecReInit(void* ida_mem, sunindextype mudq,
                                     sunindextype mldq, sunrealtype dq_rel_yy);

/* Optional output functions */

SUNDIALS_DEPRECATED_EXPORT_MSG(
  "Work space functions will be removed in version 8.0.0")
int IDABBDPrecGetWorkSpace(void* ida_mem, long int* lenrwBBDP,
                           long int* leniwBBDP);

SUNDIALS_EXPORT int IDABBDPrecGetNumGfnEvals(void* ida_mem,
                                             long int* ngevalsBBDP);

/*------------------
  BACKWARD PROBLEMS
  ------------------*/

/* User-Supplied Function Types */

typedef int (*IDABBDLocalFnB)(sunindextype NlocalB, sunrealtype tt, N_Vector yy,
                              N_Vector yp, N_Vector yyB, N_Vector ypB,
                              N_Vector gvalB, void* user_dataB);

typedef int (*IDABBDCommFnB)(sunindextype NlocalB, sunrealtype tt, N_Vector yy,
                             N_Vector yp, N_Vector yyB, N_Vector ypB,
                             void* user_dataB);

/* Exported Functions */

SUNDIALS_EXPORT int IDABBDPrecInitB(void* ida_mem, int which,
                                    sunindextype NlocalB, sunindextype mudqB,
                                    sunindextype mldqB, sunindextype mukeepB,
                                    sunindextype mlkeepB, sunrealtype dq_rel_yyB,
                                    IDABBDLocalFnB GresB, IDABBDCommFnB GcommB);

SUNDIALS_EXPORT int IDABBDPrecReInitB(void* ida_mem, int which,
                                      sunindextype mudqB, sunindextype mldqB,
                                      sunrealtype dq_rel_yyB);

#ifdef __cplusplus
}
#endif

#endif
