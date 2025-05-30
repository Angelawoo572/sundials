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
# CMakeLists.txt file for sunlinsol PCG examples
# ---------------------------------------------------------------

# Set tolerance for linear solver test based on Sundials precision
if(SUNDIALS_PRECISION MATCHES "SINGLE")
  set(TOL "1e-2")
elseif(SUNDIALS_PRECISION MATCHES "DOUBLE")
  set(TOL "1e-10")
else()
  set(TOL "1e-13")
endif()

# Example lists are tuples "name\;args\;nodes\;tasks\;type" where the type is
# develop for examples excluded from 'make test' in releases

# Examples using the SUNDIALS PCG linear solver
set(sunlinsol_pcg_examples
    "test_sunlinsol_pcg_parallel\;100 500 ${TOL} 0\;1\;4\;")

# Dependencies for nvector examples
set(sunlinsol_pcg_dependencies test_sunlinsol)

# Add source directory to include directories
include_directories(. ../..)

if(MPI_C_COMPILER)
  # use MPI wrapper as the compiler
  set(CMAKE_C_COMPILER ${MPI_C_COMPILER})
else()
  # add MPI_INCLUDE_PATH to include directories
  include_directories(${MPI_INCLUDE_PATH})
endif()

# Add the build and install targets for each example
foreach(example_tuple ${sunlinsol_pcg_examples})

  # parse the example tuple
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 example_args)
  list(GET example_tuple 2 number_of_nodes)
  list(GET example_tuple 3 number_of_tasks)
  list(GET example_tuple 4 example_type)

  # check if this example has already been added, only need to add example
  # source files once for testing with different inputs
  if(NOT TARGET ${example})
    # example source files
    sundials_add_executable(${example} ${example}.c ../../test_sunlinsol.c)

    # folder to organize targets in an IDE
    set_target_properties(${example} PROPERTIES FOLDER "Examples")

    # libraries to link against
    target_link_libraries(${example} sundials_nvecparallel
                          sundials_sunlinsolpcg ${EXE_EXTRA_LINK_LIBS})

    if(NOT MPI_C_COMPILER)
      target_link_libraries(${example} ${MPI_LIBRARIES})
    endif()
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
    MPI_NPROCS ${number_of_tasks}
    EXAMPLE_TYPE ${example_type}
    NODIFF)

  # install example source files
  if(EXAMPLES_INSTALL)
    install(FILES ${example}.c ../../test_sunlinsol.h ../../test_sunlinsol.c
            DESTINATION ${EXAMPLES_INSTALL_PATH}/sunlinsol/pcg/parallel)
  endif()

endforeach(example_tuple ${sunlinsol_pcg_examples})

if(EXAMPLES_INSTALL)

  # Install the README file
  install(FILES DESTINATION ${EXAMPLES_INSTALL_PATH}/sunlinsol/pcg/parallel)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  set(SOLVER_LIB "sundials_sunlinsolpcg")

  examples2string(sunlinsol_pcg_examples EXAMPLES)
  examples2string(sunlinsol_pcg_dependencies EXAMPLES_DEPENDENCIES)

  # Regardless of the platform we're on, we will generate and install
  # CMakeLists.txt file for building the examples. This file  can then be used
  # as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  configure_file(
    ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_parallel_C_ex.in
    ${PROJECT_BINARY_DIR}/test/unit_tests/sunlinsol/pcg/parallel/CMakeLists.txt
    @ONLY)

  # install CMakelists.txt
  install(
    FILES
      ${PROJECT_BINARY_DIR}/test/unit_tests/sunlinsol/pcg/parallel/CMakeLists.txt
    DESTINATION ${EXAMPLES_INSTALL_PATH}/sunlinsol/pcg/parallel)

  # On UNIX-type platforms, we also  generate and install a makefile for
  # building the examples. This makefile can then be used as a template for the
  # user's own programs.

  if(UNIX)
    # generate Makefile and place it in the binary dir
    configure_file(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_parallel_C_ex.in
      ${PROJECT_BINARY_DIR}/test/unit_tests/sunlinsol/pcg/parallel/Makefile_ex
      @ONLY)
    # install the configured Makefile_ex as Makefile
    install(
      FILES
        ${PROJECT_BINARY_DIR}/test/unit_tests/sunlinsol/pcg/parallel/Makefile_ex
      DESTINATION ${EXAMPLES_INSTALL_PATH}/sunlinsol/pcg/parallel
      RENAME Makefile)
  endif()

endif()
