# ---------------------------------------------------------------
# Programmer(s): David J. Gardner and Slaven Peles @ LLNL
# ---------------------------------------------------------------
# SUNDIALS Copyright Start
# Copyright (c) 2002-2025, Lawrence Livermore National Security
# and Southern Methodist University.
# All rights reserved.
#
# See the top-level LICENSE and NOTICE files for details.
#
# SPDX-License-Identifier: BSD-3-Clause
# SUNDIALS Copyright End
# ---------------------------------------------------------------
# examples/idas level CMakeLists.txt for SUNDIALS
# ---------------------------------------------------------------

# C examples
if(EXAMPLES_ENABLE_C)
  add_subdirectory(serial)
  if(ENABLE_OPENMP AND OPENMP_FOUND)
    add_subdirectory(C_openmp)
  endif()
  if(ENABLE_MPI AND MPI_C_FOUND)
    add_subdirectory(parallel)
  endif()
endif()

if(BUILD_FORTRAN_MODULE_INTERFACE AND EXAMPLES_ENABLE_F2003)
  add_subdirectory(F2003_serial)
endif()
