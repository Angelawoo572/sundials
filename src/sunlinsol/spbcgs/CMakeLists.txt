# ---------------------------------------------------------------
# Programmer(s): Daniel R. Reynolds @ SMU,
#                and Cody J. Balos @ LLNL
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
# CMakeLists.txt file for the SPBCGS SUNLinearSolver library
# ---------------------------------------------------------------

install(CODE "MESSAGE(\"\nInstall SUNLINSOL_SPBCGS\n\")")

# Add the sunlinsol_spbcgs library
sundials_add_library(
  sundials_sunlinsolspbcgs
  SOURCES sunlinsol_spbcgs.c
  HEADERS ${SUNDIALS_SOURCE_DIR}/include/sunlinsol/sunlinsol_spbcgs.h
  INCLUDE_SUBDIR sunlinsol
  LINK_LIBRARIES PUBLIC sundials_core
  OBJECT_LIBRARIES
  OUTPUT_NAME sundials_sunlinsolspbcgs
  VERSION ${sunlinsollib_VERSION}
  SOVERSION ${sunlinsollib_SOVERSION})

message(STATUS "Added SUNLINSOL_SPBCGS module")

# Add F90 module if F2003 interface is enabled
if(BUILD_FORTRAN_MODULE_INTERFACE)
  add_subdirectory("fmod_int${SUNDIALS_INDEX_SIZE}")
endif()
