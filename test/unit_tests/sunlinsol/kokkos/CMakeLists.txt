# ------------------------------------------------------------------------------
# Programmer(s): David J. Gardner @ LLNL
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
# ------------------------------------------------------------------------------

# Example lists are tuples "name\;args\;type" where the type is 'develop' for
# examples excluded from 'make test' in releases
set(examples_list
    "test_sunlinsol_kokkosdense.cpp\;10 1 0\;"
    "test_sunlinsol_kokkosdense.cpp\;100 1 0\;"
    "test_sunlinsol_kokkosdense.cpp\;10 1000 0\;"
    "test_sunlinsol_kokkosdense.cpp\;100 50 0\;")

# Add source directory to include directories
include_directories(..)

# Add the build and install targets for each example
foreach(example_tuple ${examples_list})
  foreach(backend ${KOKKOS_EXAMPLES_BACKENDS})

    # parse the example tuple
    list(GET example_tuple 0 example)
    list(GET example_tuple 1 example_args)
    list(GET example_tuple 2 example_type)

    # extract the file name without extension
    get_filename_component(example_target ${example} NAME_WE)
    set(example_target "${example_target}.${backend}")

    if(NOT TARGET ${example_target})
      # example source files
      sundials_add_executable(${example_target} ${example} ../test_sunlinsol.c
                              ../test_sunlinsol.h)

      # folder for IDEs
      set_target_properties(${example_target} PROPERTIES FOLDER "Examples")

      # which backend to use
      target_compile_definitions(${example_target} PRIVATE USE_${backend})

      # libraries to link against
      target_link_libraries(
        ${example_target}
        PRIVATE sundials_nveckokkos sundials_sunmatrixkokkosdense
                sundials_sunlinsolkokkosdense ${EXE_EXTRA_LINK_LIBS})
    endif()

    # check if example args are provided and set the test name
    if("${example_args}" STREQUAL "")
      set(test_name ${example_target})
    else()
      string(REGEX REPLACE " " "_" test_name ${example_target}_${example_args})
    endif()

    # add example to regression tests
    sundials_add_test(
      ${test_name} ${example_target}
      TEST_ARGS ${example_args}
      EXAMPLE_TYPE ${example_type}
      NODIFF)

  endforeach()
endforeach()

if(EXAMPLES_INSTALL)

  sundials_install_examples(
    sunlinsol_kokkosdense examples_list
    EXAMPLES_DEPENDENCIES test_sunlinsol.c
    EXTRA_FILES
      ${SUNDIALS_SOURCE_DIR}/test/unit_tests/sunlinsol/test_sunlinsol.c
      ${SUNDIALS_SOURCE_DIR}/test/unit_tests/sunlinsol/test_sunlinsol.h
    CMAKE_TEMPLATE cmakelists_CXX_ex.in
    SUNDIALS_COMPONENTS nveckokkos sunmatrixkokkosdense sunlinsolkokkosdense
    SUNDIALS_TARGETS generic
    OTHER_TARGETS Kokkos::kokkos Kokkos::kokkoskernels
    DESTINATION sunlinsol/kokkos)

endif()
