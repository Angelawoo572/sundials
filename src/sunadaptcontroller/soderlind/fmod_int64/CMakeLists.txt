# ---------------------------------------------------------------
# Programmer(s): Daniel R. Reynolds @ SMU
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

# Disable warnings from unused SWIG macros which we can't control
if(ENABLE_ALL_WARNINGS)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-macros")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-macros")
endif()

sundials_add_f2003_library(
  sundials_fsunadaptcontrollersoderlind_mod
  SOURCES fsunadaptcontroller_soderlind_mod.f90
          fsunadaptcontroller_soderlind_mod.c
  LINK_LIBRARIES PUBLIC sundials_fcore_mod
  OUTPUT_NAME sundials_fsunadaptcontrollersoderlind_mod OBJECT_LIB_ONLY)

message(STATUS "Added SUNAdaptController_Soderlind F2003 interface")
