# ---------------------------------------------------------------
# Programmer(s): David J. Gardner @ LLNL
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
# CMakeLists.txt for CVODES unit tests
# ---------------------------------------------------------------

# C unit tests
add_subdirectory(C_serial)

# C++ unit tests
if(CXX_FOUND)
  add_subdirectory(CXX_serial)
  if(SUNDIALS_TEST_ENABLE_GTEST)
    add_subdirectory(gtest)
  endif()
endif()
