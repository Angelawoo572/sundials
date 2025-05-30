/*
 * -----------------------------------------------------------------
 * Programmer(s): Daniel Reynolds, Ashley Crawford @ SMU
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
 * This is the testing routine to check the SUNLinSol Band module
 * implementation.
 * -----------------------------------------------------------------
 */

#include <nvector/nvector_serial.h>
#include <stdio.h>
#include <stdlib.h>
#include <sundials/sundials_math.h>
#include <sundials/sundials_types.h>
#include <sunlinsol/sunlinsol_band.h>
#include <sunmatrix/sunmatrix_band.h>

#include "test_sunlinsol.h"

#if defined(SUNDIALS_EXTENDED_PRECISION)
#define GSYM "Lg"
#else
#define GSYM "g"
#endif

/* ----------------------------------------------------------------------
 * SUNLinSol_Band Testing Routine
 * --------------------------------------------------------------------*/
int main(int argc, char* argv[])
{
  int fails = 0;                   /* counter for test failures  */
  sunindextype cols, uband, lband; /* matrix columns, bandwidths */
  SUNLinearSolver LS;              /* solver object              */
  SUNMatrix A, B;                  /* test matrices              */
  N_Vector x, y, b;                /* test vectors               */
  int print_timing;
  sunindextype j, k, kstart, kend;
  sunrealtype *colj, *xdata;
  SUNContext sunctx;

  if (SUNContext_Create(SUN_COMM_NULL, &sunctx))
  {
    printf("ERROR: SUNContext_Create failed\n");
    return (-1);
  }

  /* check input and set matrix dimensions */
  if (argc < 5)
  {
    printf("ERROR: FOUR (4) Inputs required: matrix cols, matrix uband, matrix "
           "lband, print timing \n");
    return (-1);
  }

  cols = (sunindextype)atol(argv[1]);
  if (cols <= 0)
  {
    printf("ERROR: number of matrix columns must be a positive integer \n");
    return (-1);
  }

  uband = (sunindextype)atol(argv[2]);
  if ((uband <= 0) || (uband >= cols))
  {
    printf("ERROR: matrix upper bandwidth must be a positive integer, less "
           "than number of columns \n");
    return (-1);
  }

  lband = (sunindextype)atol(argv[3]);
  if ((lband <= 0) || (lband >= cols))
  {
    printf("ERROR: matrix lower bandwidth must be a positive integer, less "
           "than number of columns \n");
    return (-1);
  }

  print_timing = atoi(argv[4]);
  SetTiming(print_timing);

  printf("\nBand linear solver test: size %ld, bandwidths %ld %ld\n\n",
         (long int)cols, (long int)uband, (long int)lband);

  /* Create matrices and vectors */
  A = SUNBandMatrix(cols, uband, lband, sunctx);
  B = SUNBandMatrix(cols, uband, lband, sunctx);
  x = N_VNew_Serial(cols, sunctx);
  y = N_VNew_Serial(cols, sunctx);
  b = N_VNew_Serial(cols, sunctx);

  /* Fill matrix and x vector with uniform random data in [0,1] */
  xdata = N_VGetArrayPointer(x);
  for (j = 0; j < cols; j++)
  {
    /* A matrix column */
    colj   = SUNBandMatrix_Column(A, j);
    kstart = (j < uband) ? -j : -uband;
    kend   = (j > cols - 1 - lband) ? cols - 1 - j : lband;
    for (k = kstart; k <= kend; k++)
    {
      colj[k] = (sunrealtype)rand() / (sunrealtype)RAND_MAX;
    }

    /* x entry */
    xdata[j] = (sunrealtype)rand() / (sunrealtype)RAND_MAX;
  }

  /* Scale/shift matrix to ensure diagonal dominance */
  fails += SUNMatScaleAddI(ONE / (uband + lband + 1), A);
  if (fails)
  {
    printf("FAIL: SUNLinSol SUNMatScaleAddI failure\n");

    /* Free matrices and vectors */
    SUNMatDestroy(A);
    SUNMatDestroy(B);
    N_VDestroy(x);
    N_VDestroy(y);
    N_VDestroy(b);

    return (1);
  }

  /* copy A and x into B and y to print in case of solver failure */
  SUNMatCopy(A, B);
  N_VScale(ONE, x, y);

  /* create right-hand side vector for linear solve */
  fails = SUNMatMatvec(A, x, b);
  if (fails)
  {
    printf("FAIL: SUNLinSol SUNMatMatvec failure\n");

    /* Free matrices and vectors */
    SUNMatDestroy(A);
    SUNMatDestroy(B);
    N_VDestroy(x);
    N_VDestroy(y);
    N_VDestroy(b);

    return (1);
  }

  /* Create banded linear solver */
  LS = SUNLinSol_Band(x, A, sunctx);
  if (LS == NULL)
  {
    printf("FAIL: SUNLinSol_Band returned NULL\n");

    /* Free matrices and vectors */
    SUNMatDestroy(A);
    SUNMatDestroy(B);
    N_VDestroy(x);
    N_VDestroy(y);
    N_VDestroy(b);

    return (1);
  }

  /* Run Tests */
  fails += Test_SUNLinSolInitialize(LS, 0);
  fails += Test_SUNLinSolSetup(LS, A, 0);
  fails += Test_SUNLinSolSolve(LS, A, x, b, 100 * SUN_UNIT_ROUNDOFF, SUNTRUE, 0);

  fails += Test_SUNLinSolGetType(LS, SUNLINEARSOLVER_DIRECT, 0);
  fails += Test_SUNLinSolGetID(LS, SUNLINEARSOLVER_BAND, 0);
  fails += Test_SUNLinSolLastFlag(LS, 0);
  fails += Test_SUNLinSolSpace(LS, 0);

  /* Print result */
  if (fails)
  {
    printf("FAIL: SUNLinSol module failed %i tests \n \n", fails);
    printf("\nA (original) =\n");
    SUNBandMatrix_Print(B, stdout);
    printf("\nA (factored) =\n");
    SUNBandMatrix_Print(A, stdout);
    printf("\nx (original) =\n");
    N_VPrint_Serial(y);
    printf("\nx (computed) =\n");
    N_VPrint_Serial(x);
  }
  else { printf("SUCCESS: SUNLinSol module passed all tests \n \n"); }

  /* Free solver, matrix and vectors */
  SUNLinSolFree(LS);
  SUNMatDestroy(A);
  SUNMatDestroy(B);
  N_VDestroy(x);
  N_VDestroy(y);
  N_VDestroy(b);
  SUNContext_Free(&sunctx);

  return (fails);
}

/* ----------------------------------------------------------------------
 * Implementation-specific 'check' routines
 * --------------------------------------------------------------------*/
int check_vector(N_Vector X, N_Vector Y, sunrealtype tol)
{
  int failure = 0;
  sunindextype i, local_length;
  sunrealtype *Xdata, *Ydata, maxerr;

  Xdata        = N_VGetArrayPointer(X);
  Ydata        = N_VGetArrayPointer(Y);
  local_length = N_VGetLength_Serial(X);

  /* check vector data */
  for (i = 0; i < local_length; i++)
  {
    failure += SUNRCompareTol(Xdata[i], Ydata[i], tol);
  }

  if (failure > ZERO)
  {
    maxerr = ZERO;
    for (i = 0; i < local_length; i++)
    {
      maxerr = SUNMAX(SUNRabs(Xdata[i] - Ydata[i]), maxerr);
    }
    printf("check err failure: maxerr = %" GSYM " (tol = %" GSYM ")\n", maxerr,
           tol);
    return (1);
  }
  else { return (0); }
}

void sync_device(void) {}
