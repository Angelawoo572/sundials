# ---------------------------------------------------------------
# Programmer(s): Steven Smith @ LLNL
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
# CMakeLists.txt file for PThread nvector examples
# ---------------------------------------------------------------

# Example lists are tuples "name\;args\;type" where the type is 'develop' for
# examples excluded from 'make test' in releases

# Examples using SUNDIALS pthread nvector
set(nvector_pthreads_examples
    "test_nvector_pthreads\;1000 1 0\;" "test_nvector_pthreads\;1000 2 0\;"
    "test_nvector_pthreads\;1000 4 0\;" "test_nvector_pthreads\;10000 1 0\;"
    "test_nvector_pthreads\;10000 2 0\;" "test_nvector_pthreads\;10000 4 0\;")

# Dependencies for nvector examples
set(nvector_examples_dependencies test_nvector)

# If building F2003 tests
if(BUILD_FORTRAN_MODULE_INTERFACE)
  set(nvector_pthreads_fortran_examples "test_fnvector_pthreads_mod\;\;")
endif()

# Add source directory to include directories
include_directories(. ..)

# Specify libraries to link against
set(NVECS_LIB sundials_nvecpthreads)
if(BUILD_FORTRAN_MODULE_INTERFACE)
  list(APPEND NVECS_LIB sundials_fnvecpthreads_mod)
endif()

# Set-up linker flags and link libraries
set(SUNDIALS_LIBS ${NVECS_LIB} ${EXE_EXTRA_LINK_LIBS})

# Add the build and install targets for each example
foreach(example_tuple ${nvector_pthreads_examples})

  # parse the example tuple
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 example_args)
  list(GET example_tuple 2 example_type)

  # check if this example has already been added, only need to add example
  # source files once for testing with different inputs
  if(NOT TARGET ${example})
    # example source files
    sundials_add_executable(${example} ${example}.c)

    # link vector test utilities
    target_link_libraries(${example} PRIVATE test_nvector_obj)

    # folder to organize targets in an IDE
    set_target_properties(${example} PROPERTIES FOLDER "Examples")

    # libraries to link against
    target_link_libraries(${example} PRIVATE ${SUNDIALS_LIBS}
                                             ${CMAKE_THREAD_LIBS_INIT})
  endif()

  # check if example args are provided and set the test name
  if("${example_args}" STREQUAL "")
    set(test_name ${example})
  else()
    string(REGEX REPLACE " " "_" test_name ${example}_${example_args})
  endif()

  # add example to regression tests
  sundials_add_test(
    ${test_name} ${example}
    TEST_ARGS ${example_args}
    EXAMPLE_TYPE ${example_type}
    NODIFF)

  # install example source files
  if(EXAMPLES_INSTALL)
    install(FILES ${example}.c ../test_nvector.c ../test_nvector.h
            DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/pthreads)
  endif()

endforeach(example_tuple ${nvector_pthreads_examples})

# Add the build and install targets for each example
foreach(example_tuple ${nvector_pthreads_fortran_examples})

  # parse the example tuple
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 example_args)
  list(GET example_tuple 2 example_type)

  # check if this example has already been added, only need to add example
  # source files once for testing with different inputs
  if(NOT TARGET ${example})
    # build fortran modules into a unique directory to avoid naming collisions
    set(CMAKE_Fortran_MODULE_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${example}.dir)

    # example source files
    sundials_add_executable(
      ${example} ${example}.f90
      ${SUNDIALS_SOURCE_DIR}/test/unit_tests/utilities/test_utilities.f90
      ${SUNDIALS_SOURCE_DIR}/test/unit_tests/nvector/test_nvector.f90)

    # folder to organize targets in an IDE
    set_target_properties(${example} PROPERTIES FOLDER "Examples")

    # libraries to link against
    target_link_libraries(${example} ${SUNDIALS_LIBS} ${CMAKE_THREAD_LIBS_INIT})
  endif()

  # check if example args are provided and set the test name
  if("${example_args}" STREQUAL "")
    set(test_name ${example})
  else()
    string(REGEX REPLACE " " "_" test_name ${example}_${example_args})
  endif()

  # add example to regression tests
  sundials_add_test(
    ${test_name} ${example}
    TEST_ARGS ${example_args}
    EXAMPLE_TYPE ${example_type}
    NODIFF)

endforeach(example_tuple ${nvector_pthreads_fortran_examples})

if(EXAMPLES_INSTALL)

  # Install the README file
  install(FILES DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/pthreads)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  set(SOLVER_LIB "sundials_nvecpthreads")

  examples2string(nvector_pthreads_examples EXAMPLES)
  examples2string(nvector_examples_dependencies EXAMPLES_DEPENDENCIES)

  # Regardless of the platform we're on, we will generate and install
  # CMakeLists.txt file for building the examples. This file  can then be used
  # as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  configure_file(
    ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_pthreads_C_ex.in
    ${PROJECT_BINARY_DIR}/test/unit_tests/nvector/pthreads/CMakeLists.txt @ONLY)

  # install CMakelists.txt
  install(
    FILES ${PROJECT_BINARY_DIR}/test/unit_tests/nvector/pthreads/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/pthreads)

  # On UNIX-type platforms, we also  generate and install a makefile for
  # building the examples. This makefile can then be used as a template for the
  # user's own programs.

  if(UNIX)
    # generate Makefile and place it in the binary dir
    configure_file(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_pthreads_C_ex.in
      ${PROJECT_BINARY_DIR}/test/unit_tests/nvector/pthreads/Makefile_ex @ONLY)
    # install the configured Makefile_ex as Makefile
    install(
      FILES ${PROJECT_BINARY_DIR}/test/unit_tests/nvector/pthreads/Makefile_ex
      DESTINATION ${EXAMPLES_INSTALL_PATH}/nvector/pthreads
      RENAME Makefile)
  endif()

endif()
