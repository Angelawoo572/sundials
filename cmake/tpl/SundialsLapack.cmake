# -----------------------------------------------------------------------------
# Programmer(s): Radu Serban and Cody J. Balos @ LLNL
# -----------------------------------------------------------------------------
# SUNDIALS Copyright Start
# Copyright (c) 2002-2024, Lawrence Livermore National Security
# and Southern Methodist University.
# All rights reserved.
#
# See the top-level LICENSE and NOTICE files for details.
#
# SPDX-License-Identifier: BSD-3-Clause
# SUNDIALS Copyright End
# -----------------------------------------------------------------------------
# Module to find and setup LAPACK/BLAS correctly.
# Created from the SundialsTPL.cmake template.
# All SUNDIALS modules that find and setup a TPL must:
#
# 1. Check to make sure the SUNDIALS configuration and the TPL is compatible.
# 2. Find the TPL.
# 3. Check if the TPL works with SUNDIALS, UNLESS the override option
# <TPL>_WORKS is TRUE - in this case the tests should not be performed and it
# should be assumed that the TPL works with SUNDIALS.
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Section 1: Include guard
# -----------------------------------------------------------------------------

include_guard(GLOBAL)

# -----------------------------------------------------------------------------
# Section 2: Check to make sure options are compatible
# -----------------------------------------------------------------------------

# LAPACK does not support extended precision
if(ENABLE_LAPACK AND SUNDIALS_PRECISION MATCHES "EXTENDED")
  message(
    FATAL_ERROR "LAPACK is not compatible with ${SUNDIALS_PRECISION} precision")
endif()

# -----------------------------------------------------------------------------
# Section 3: Find the TPL
# -----------------------------------------------------------------------------

find_package(LAPACK REQUIRED)

# get path to LAPACK library to use in generated makefiles for examples, if
# LAPACK_LIBRARIES contains multiple items only use the path of the first entry
list(GET LAPACK_LIBRARIES 0 TMP_LAPACK_LIBRARIES)
get_filename_component(LAPACK_LIBRARY_DIR ${TMP_LAPACK_LIBRARIES} PATH)

# -----------------------------------------------------------------------------
# Section 4: Test the TPL
# -----------------------------------------------------------------------------

# Macro to test different name mangling options
macro(test_lapack_name_mangling)

  # Both the name mangling case and number of underscores must be set
  if((NOT SUNDIALS_LAPACK_FUNC_CASE AND SUNDIALS_LAPACK_FUNC_UNDERSCORES) OR
      (SUNDIALS_LAPACK_FUNC_CASE AND NOT SUNDIALS_LAPACK_FUNC_UNDERSCORES))
    print_error("Both SUNDIALS_LAPACK_FUNC_CASE and SUNDIALS_LAPACK_FUNC_UNDERSCORES must be set.")
  endif()

  # Create the FortranTest directory
  set(FortranTest_DIR ${PROJECT_BINARY_DIR}/FortranTest)
  file(MAKE_DIRECTORY ${FortranTest_DIR})

  # Create a CMakeLists.txt file which will generate the "flib" library and an
  # executable "ftest"
  file(
    WRITE ${FortranTest_DIR}/CMakeLists.txt
    "CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_VERSION})\n"
    "PROJECT(ftest Fortran)\n"
    "SET(CMAKE_VERBOSE_MAKEFILE ON)\n"
    "SET(CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\")\n"
    "SET(CMAKE_Fortran_COMPILER \"${CMAKE_Fortran_COMPILER}\")\n"
    "SET(CMAKE_Fortran_FLAGS \"${CMAKE_Fortran_FLAGS}\")\n"
    "SET(CMAKE_Fortran_FLAGS_RELEASE \"${CMAKE_Fortran_FLAGS_RELEASE}\")\n"
    "SET(CMAKE_Fortran_FLAGS_DEBUG \"${CMAKE_Fortran_FLAGS_DEBUG}\")\n"
    "SET(CMAKE_Fortran_FLAGS_RELWITHDEBUGINFO \"${CMAKE_Fortran_FLAGS_RELWITHDEBUGINFO}\")\n"
    "SET(CMAKE_Fortran_FLAGS_MINSIZE \"${CMAKE_Fortran_FLAGS_MINSIZE}\")\n"
    "ADD_LIBRARY(flib flib.f)\n"
    "ADD_EXECUTABLE(ftest ftest.f)\n"
    "TARGET_LINK_LIBRARIES(ftest flib)\n")

  # Create the Fortran source flib.f which defines two subroutines, "mysub" and
  # "my_sub"
  file(WRITE ${FortranTest_DIR}/flib.f
       "        SUBROUTINE mysub\n" "        RETURN\n" "        END\n"
       "        SUBROUTINE my_sub\n" "        RETURN\n" "        END\n")

  # Create the Fortran source ftest.f which calls "mysub" and "my_sub"
  file(WRITE ${FortranTest_DIR}/ftest.f
       "        PROGRAM ftest\n" "        CALL mysub()\n"
       "        CALL my_sub()\n" "        END\n")

  # Use TRY_COMPILE to make the targets "flib" and "ftest"
  try_compile(
    FTEST_OK ${FortranTest_DIR}
    ${FortranTest_DIR} ftest
    OUTPUT_VARIABLE MY_OUTPUT)

  # Test timers with a simple program
  set(LAPACK_TEST_DIR ${PROJECT_BINARY_DIR}/LapackTest)
  file(MAKE_DIRECTORY ${LAPACK_TEST_DIR})

  # Proceed based on test results
  if(FTEST_OK)

    # Infer Fortran name-mangling scheme for symbols WITHOUT underscores.
    # Overwrite CMakeLists.txt with one which will generate the "ctest1"
    # executable
    file(
      WRITE ${FortranTest_DIR}/CMakeLists.txt
      "CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_VERSION})\n"
      "PROJECT(ctest1 C)\n"
      "SET(CMAKE_VERBOSE_MAKEFILE ON)\n"
      "SET(CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\")\n"
      "SET(CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\")\n"
      "SET(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS}\")\n"
      "SET(CMAKE_C_FLAGS_RELEASE \"${CMAKE_C_FLAGS_RELEASE}\")\n"
      "SET(CMAKE_C_FLAGS_DEBUG \"${CMAKE_C_FLAGS_DEBUG}\")\n"
      "SET(CMAKE_C_FLAGS_RELWITHDEBUGINFO \"${CMAKE_C_FLAGS_RELWITHDEBUGINFO}\")\n"
      "SET(CMAKE_C_FLAGS_MINSIZE \"${CMAKE_C_FLAGS_MINSIZE}\")\n"
      "ADD_EXECUTABLE(ctest1 ctest1.c)\n"
      "FIND_LIBRARY(FLIB flib \"${FortranTest_DIR}\")\n"
      "TARGET_LINK_LIBRARIES(ctest1 \${FLIB})\n")

    # Define the list "options" of all possible schemes that we want to consider
    # Get its length and initialize the counter "iopt" to zero
    set(options mysub mysub_ mysub__ MYSUB MYSUB_ MYSUB__)
    list(LENGTH options imax)
    set(iopt 0)

    # We will attempt to successfully generate the "ctest1" executable as long
    # as there still are entries in the "options" list
    while(${iopt} LESS ${imax})
      # Get the current list entry (current scheme)
      list(GET options ${iopt} opt)
      # Generate C source which calls the "mysub" function using the current
      # scheme
      file(WRITE ${FortranTest_DIR}/ctest1.c
           "extern void ${opt}();\n" "int main(void){${opt}();return(0);}\n")
      # Use TRY_COMPILE to make the "ctest1" executable from the current C
      # source and linking to the previously created "flib" library.
      try_compile(
        CTEST_OK ${FortranTest_DIR}
        ${FortranTest_DIR} ctest1
        OUTPUT_VARIABLE MY_OUTPUT)
      # Write output compiling the test code
      file(WRITE ${FortranTest_DIR}/ctest1_${opt}.out "${MY_OUTPUT}")
      # To ensure we do not use stuff from the previous attempts, we must remove
      # the CMakeFiles directory.
      file(REMOVE_RECURSE ${FortranTest_DIR}/CMakeFiles)
      # Test if we successfully created the "ctest" executable. If yes, save the
      # current scheme, and set the counter "iopt" to "imax" so that we exit the
      # while loop. Otherwise, increment the counter "iopt" and go back in the
      # while loop.
      if(CTEST_OK)
        set(CMAKE_Fortran_SCHEME_NO_UNDERSCORES ${opt})
        set(iopt ${imax})
      else(CTEST_OK)
        math(EXPR iopt ${iopt}+1)
      endif()
    endwhile(${iopt} LESS ${imax})

    # Infer Fortran name-mangling scheme for symbols WITH underscores.
    # Practically a duplicate of the previous steps.
    file(
      WRITE ${FortranTest_DIR}/CMakeLists.txt
      "CMAKE_MINIMUM_REQUIRED(VERSION ${CMAKE_VERSION})\n"
      "PROJECT(ctest2 C)\n"
      "SET(CMAKE_VERBOSE_MAKEFILE ON)\n"
      "SET(CMAKE_BUILD_TYPE \"${CMAKE_BUILD_TYPE}\")\n"
      "SET(CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\")\n"
      "SET(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS}\")\n"
      "SET(CMAKE_C_FLAGS_RELEASE \"${CMAKE_C_FLAGS_RELEASE}\")\n"
      "SET(CMAKE_C_FLAGS_DEBUG \"${CMAKE_C_FLAGS_DEBUG}\")\n"
      "SET(CMAKE_C_FLAGS_RELWITHDEBUGINFO \"${CMAKE_C_FLAGS_RELWITHDEBUGINFO}\")\n"
      "SET(CMAKE_C_FLAGS_MINSIZE \"${CMAKE_C_FLAGS_MINSIZE}\")\n"
      "ADD_EXECUTABLE(ctest2 ctest2.c)\n"
      "FIND_LIBRARY(FLIB flib \"${FortranTest_DIR}\")\n"
      "TARGET_LINK_LIBRARIES(ctest2 \${FLIB})\n")

    set(options my_sub my_sub_ my_sub__ MY_SUB MY_SUB_ MY_SUB__)
    list(LENGTH options imax)
    set(iopt 0)
    while(${iopt} LESS ${imax})
      list(GET options ${iopt} opt)
      file(WRITE ${FortranTest_DIR}/ctest2.c
           "extern void ${opt}();\n" "int main(void){${opt}();return(0);}\n")
      try_compile(
        CTEST_OK ${FortranTest_DIR}
        ${FortranTest_DIR} ctest2
        OUTPUT_VARIABLE MY_OUTPUT)
      file(WRITE ${FortranTest_DIR}/ctest2_${opt}.out "${MY_OUTPUT}")
      file(REMOVE_RECURSE ${FortranTest_DIR}/CMakeFiles)
      if(CTEST_OK)
        set(CMAKE_Fortran_SCHEME_WITH_UNDERSCORES ${opt})
        set(iopt ${imax})
      else(CTEST_OK)
        math(EXPR iopt ${iopt}+1)
      endif()
    endwhile(${iopt} LESS ${imax})

    # If a name-mangling scheme was found set the C preprocessor macros to use
    # that scheme. Otherwise default to lower case with one underscore.
    if(CMAKE_Fortran_SCHEME_NO_UNDERSCORES
       AND CMAKE_Fortran_SCHEME_WITH_UNDERSCORES)
      message(STATUS "Determining Fortran name-mangling scheme... OK")
    else()
      message(STATUS "Determining Fortran name-mangling scheme... DEFAULT")
      set(CMAKE_Fortran_SCHEME_NO_UNDERSCORES "mysub_")
      set(CMAKE_Fortran_SCHEME_WITH_UNDERSCORES "my_sub_")
    endif()

  else()

    message(STATUS "Checking if LAPACK works with SUNDIALS... FAILED")
    set(LAPACK_WORKS FALSE CACHE BOOL "LAPACK does not work with SUNDIALS as configured" FORCE)
    print_error("SUNDIALS interface to LAPACK is not functional.")

  endif()

else()

  message(STATUS "Skipped LAPACK tests, assuming LAPACK works with SUNDIALS. Set LAPACK_WORKS=FALSE to (re)run compatibility test.")

endif()
