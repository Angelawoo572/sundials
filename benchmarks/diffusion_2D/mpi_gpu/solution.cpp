/* -----------------------------------------------------------------------------
 * Programmer(s): David J. Gardner @ LLNL
 * -----------------------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2025, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * -----------------------------------------------------------------------------
 * CUDA/HIP solution and derivative kernels and functions
 * ---------------------------------------------------------------------------*/

#include "diffusion_2D.hpp"

#if defined(USE_HIP)
#include <hip/hip_runtime.h>
#define BLOCK_SIZE   256
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16
#elif defined(USE_CUDA)
#include <cuda_runtime.h>
#define BLOCK_SIZE   256
#define BLOCK_SIZE_X 16
#define BLOCK_SIZE_Y 16
#else
#error Define USE_CUDA or USE_HIP
#endif

// Compute the exact solution
__global__ void solution_kernel(const sunrealtype t, sunrealtype* u,
                                const sunindextype is, const sunindextype ie,
                                const sunindextype js, const sunindextype je,
                                const sunindextype nx, const sunindextype ny,
                                const sunindextype nx_loc,
                                const sunindextype ny_loc, const sunrealtype dx,
                                const sunrealtype dy) // 解 u(x, y, t)
{
  // Thread location in the local grid
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;

  // west, south, east, and north physical boundaries
  if (i < nx_loc && j < ny_loc) // 限制线程在合法范围内运行，防止越界
  {
    if ((is == 0 && i == 0) || (ie == nx - 1 && i == nx_loc - 1) ||
        (js == 0 && j == 0) || (je == ny - 1 && j == ny_loc - 1)) // 判断是否在边界上，是就设置为零
    {
      u[i + j * nx_loc] = ZERO;
    }
    else
    {
      sunrealtype x = (is + i) * dx;
      sunrealtype y = (js + j) * dy;

      sunrealtype cos_sqr_t = cos(PI * t) * cos(PI * t);
      sunrealtype sin_sqr_x = sin(PI * x) * sin(PI * x);
      sunrealtype sin_sqr_y = sin(PI * y) * sin(PI * y);

      u[i + j * nx_loc] = sin_sqr_x * sin_sqr_y * cos_sqr_t;
    }
  }
}

// Compute the exact solution derivative
__global__ void solution_p_kernel(const sunrealtype t, sunrealtype* up,
                                  const sunindextype is, const sunindextype ie,
                                  const sunindextype js, const sunindextype je,
                                  const sunindextype nx, const sunindextype ny,
                                  const sunindextype nx_loc,
                                  const sunindextype ny_loc,
                                  const sunrealtype dx, const sunrealtype dy)
{
  // Thread location in the local grid
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int j = blockIdx.y * blockDim.y + threadIdx.y;

  // west, south, east, and north physical boundaries
  if (i < nx_loc && j < ny_loc)
  {
    if ((is == 0 && i == 0) || (ie == nx - 1 && i == nx_loc - 1) ||
        (js == 0 && j == 0) || (je == ny - 1 && j == ny_loc - 1))
    {
      up[i + j * nx_loc] = ZERO;
    }
    else
    {
      sunrealtype x = (is + i) * dx;
      sunrealtype y = (js + j) * dy;

      sunrealtype cos_sin_t = -TWO * PI * cos(PI * t) * sin(PI * t);
      sunrealtype sin_sqr_x = sin(PI * x) * sin(PI * x);
      sunrealtype sin_sqr_y = sin(PI * y) * sin(PI * y);

      up[i + j * nx_loc] = sin_sqr_x * sin_sqr_y * cos_sin_t;
    }
  }
}

// Compute the exact solution
int Solution(sunrealtype t, N_Vector u, UserData* udata)
{
  // Initialize u to zero (handles boundary conditions)
  N_VConst(ZERO, u);

  // Extract needed constants from user data
  const sunindextype is     = udata->is;
  const sunindextype ie     = udata->ie;
  const sunindextype js     = udata->js;
  const sunindextype je     = udata->je;
  const sunindextype nx     = udata->nx;
  const sunindextype ny     = udata->ny;
  const sunindextype nx_loc = udata->nx_loc;
  const sunindextype ny_loc = udata->ny_loc;
  const sunrealtype dx      = udata->dx;
  const sunrealtype dy      = udata->dy;

  sunrealtype* uarray = N_VGetDeviceArrayPointer(N_VGetLocalVector_MPIPlusX(u));
  if (check_flag((void*)uarray, "N_VGetDeviceArrayPointer", 0)) { return -1; }

  dim3 block(BLOCK_SIZE_X, BLOCK_SIZE_Y);
  dim3 grid(ICEIL(nx_loc, BLOCK_SIZE_X), ICEIL(ny_loc, BLOCK_SIZE_Y));

  solution_kernel<<<grid, block>>>(t, uarray, is, ie, js, je, nx, ny, nx_loc,
                                   ny_loc, dx, dy);

  return 0;
}

// Compute the exact solution derivative
int SolutionDerivative(sunrealtype t, N_Vector up, UserData* udata)
{
  // Initialize u to zero (handles boundary conditions)
  N_VConst(ZERO, up);

  // Extract needed constants from user data
  const sunindextype is     = udata->is;
  const sunindextype ie     = udata->ie;
  const sunindextype js     = udata->js;
  const sunindextype je     = udata->je;
  const sunindextype nx     = udata->nx;
  const sunindextype ny     = udata->ny;
  const sunindextype nx_loc = udata->nx_loc;
  const sunindextype ny_loc = udata->ny_loc;
  const sunrealtype dx      = udata->dx;
  const sunrealtype dy      = udata->dy;

  sunrealtype* uparray = N_VGetDeviceArrayPointer(N_VGetLocalVector_MPIPlusX(up));
  if (check_flag((void*)uparray, "N_VGetDeviceArrayPointer", 0)) { return -1; }

  dim3 block(BLOCK_SIZE_X, BLOCK_SIZE_Y);
  dim3 grid(ICEIL(nx_loc, BLOCK_SIZE_X), ICEIL(ny_loc, BLOCK_SIZE_Y));

  solution_p_kernel<<<grid, block>>>(t, uparray, is, ie, js, je, nx, ny, nx_loc,
                                     ny_loc, dx, dy);

  return 0;
}
