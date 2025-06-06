# ---------------------------------------------------------------
# Programmer(s): Cody J. Balos @ LLNL
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
# CMakeLists.txt file for the F2003 SPBCGS SUNLinearSolver object library
# ---------------------------------------------------------------

# Disable warnings from unused SWIG macros which we can't control
if(ENABLE_ALL_WARNINGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-macros")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-macros")
endif()

sundials_add_f2003_library(
  sundials_fsunlinsolspbcgs_mod
  SOURCES fsunlinsol_spbcgs_mod.f90 fsunlinsol_spbcgs_mod.c
  LINK_LIBRARIES PUBLIC sundials_fcore_mod
  OUTPUT_NAME sundials_fsunlinsolspbcgs_mod
  VERSION ${sunlinsollib_VERSION}
  SOVERSION ${sunlinsollib_SOVERSION})

message(STATUS "Added SUNLINSOL_SPBCGS F2003 interface")
