# ------------------------------------------------------------------------------
# Programmer(s): Cody J. Balos @ LLNL
# ------------------------------------------------------------------------------
# SUNDIALS Copyright Start
# Copyright (c) 2002-2025, Lawrence Livermore National Security
# and Southern Methodist University.
# All rights reserved.
#
# See the top-level LICENSE and NOTICE files for details.
#
# SPDX-License-Identifier: BSD-3-Clause
# SUNDIALS Copyright End
# -----------------------------------------------------------------------------
# CMakeLists.txt file for the superlu-dist SUNLinearSolver examples
# -----------------------------------------------------------------------------

# Example lists are tuples "name\;args\;nodes\;type" where the 'develop' for
# examples excluded from 'make test' in releases

# Examples using the SuperLU_MT linear solver
set(sunlinsol_superludist_examples
    "test_sunlinsol_superludist.cpp\;100 1 1 0\;1\;"
    "test_sunlinsol_superludist.cpp\;400 1 2 0\;2\;"
    "test_sunlinsol_superludist.cpp\;900 3 1 0\;3\;"
    "test_sunlinsol_superludist.cpp\;1000 2 2 0\;4\;")

# Dependencies for nvector examples
set(sunlinsol_superludist_dependencies test_sunlinsol)

if(SUPERLUDIST_CUDA)
  set(_ex_lang CUDA)
elseif(SUPERLUDIST_ROCM)
  set(_ex_lang HIP)
else()
  set(_ex_lang CXX)
endif()

# Add source directory to include directories
include_directories(. ..)

# Add the build and install targets for each example
foreach(example_tuple ${sunlinsol_superludist_examples})

  # parse the example tuple
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 example_args)
  list(GET example_tuple 2 number_of_nodes)
  list(GET example_tuple 3 example_type)

  # extract the file name without extension
  get_filename_component(example_target ${example} NAME_WE)

  # check if this example has already been added, only need to add example
  # source files once for testing with different inputs
  if(NOT TARGET ${example_target})

    # example source files
    sundials_add_executable(${example_target} ${example} ../test_sunlinsol.c)

    # folder to organize targets in an IDE
    set_target_properties(${example_target} PROPERTIES FOLDER "Examples")

    # libraries to link against
    target_link_libraries(
      ${example_target}
      PRIVATE sundials_sunlinsolsuperludist sundials_sunmatrixslunrloc
              sundials_sunmatrixdense sundials_nvecparallel sundials_nvecserial
              MPI::MPI_CXX)
  endif()

  # check if example args are provided and set the test name
  if("${example_args}" STREQUAL "")
    if("${number_of_nodes}" STREQUAL "")
      set(test_name ${example_target})
    else()
      string(REGEX REPLACE " " "_" test_name
                           ${example_target}_${number_of_nodes})
    endif()
  else()
    if("${number_of_nodes}" STREQUAL "")
      string(REGEX REPLACE " " "_" test_name ${example_target}_${example_args})
    else()
      string(REGEX REPLACE " " "_" test_name
                           ${example_target}_${number_of_nodes}_${example_args})
    endif()
  endif()

  # add example to regression tests
  sundials_add_test(
    ${test_name} ${example_target}
    TEST_ARGS ${example_args}
    MPI_NPROCS ${number_of_nodes}
    EXAMPLE_TYPE ${example_type}
    NODIFF)

endforeach(example_tuple ${sunlinsol_superludist_examples})

if(EXAMPLES_INSTALL)

  sundials_install_examples(
    sunlinsol_superludist sunlinsol_superludist_examples
    EXAMPLES_DEPENDENCIES test_sunlinsol.c
    EXTRA_FILES
      ${SUNDIALS_SOURCE_DIR}/test/unit_tests/sunlinsol/test_sunlinsol.c
      ${SUNDIALS_SOURCE_DIR}/test/unit_tests/sunlinsol/test_sunlinsol.h
    CMAKE_TEMPLATE cmakelists_${_ex_lang}_MPI_ex.in
    SUNDIALS_TARGETS nvecserial nvecparallel sunlinsoldense sunlinsolsuperludist
    DESTINATION sunlinsol/superludist)

endif()
