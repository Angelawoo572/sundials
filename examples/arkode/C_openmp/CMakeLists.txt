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
# CMakeLists.txt file for ARKODE OpenMP examples
# ---------------------------------------------------------------

# Example lists are tuples "name\;args\;type\;float precision\;int precision"
# where the type is 'develop' for examples excluded from 'make test' in releases

# Examples using SUNDIALS linear solvers
set(ARKODE_examples "ark_brusselator1D_omp\;4\;exclude-single\;default\;default"
                    "ark_heat1D_omp\;4\;develop\;default\;10")

# Auxiliary files to install
set(ARKODE_extras_OMP plot_brusselator1D.py)

# Add the build and install targets for each ARKODE example
foreach(example_tuple ${ARKODE_examples})

  # parse the example tuple
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 example_args)
  list(GET example_tuple 2 example_type)
  list(GET example_tuple 3 example_float_precision)
  list(GET example_tuple 4 example_int_precision)

  # example source files
  sundials_add_executable(${example} ${example}.c)

  set_target_properties(${example} PROPERTIES FOLDER "Examples")

  # add to regression tests
  sundials_add_test(
    ${example} ${example}
    TEST_ARGS ${example_args}
    ANSWER_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    ANSWER_FILE ${example}.out
    EXAMPLE_TYPE ${example_type}
    FLOAT_PRECISION ${example_float_precision}
    INTEGER_PRECISION ${example_int_precision})

  # libraries to link against
  target_link_libraries(${example} sundials_arkode sundials_nvecopenmp
                        ${EXE_EXTRA_LINK_LIBS})

  # install example source and out files
  if(EXAMPLES_INSTALL)
    install(FILES ${example}.c ${example}.out
            DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp)
  endif()

endforeach(example_tuple ${ARKODE_examples})

# create Makefile and CMakeLists.txt for examples
if(EXAMPLES_INSTALL)

  # Install the README file
  install(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp)

  # Install the extra files
  foreach(extrafile ${ARKODE_extras_OMP})
    install(FILES ${extrafile}
            DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp)
  endforeach()

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  set(SOLVER "ARKODE")
  set(SOLVER_LIB "sundials_arkode")

  examples2string(ARKODE_examples EXAMPLES)

  # Regardless of the platform we're on, we will generate and install
  # CMakeLists.txt file for building the examples. This file  can then be used
  # as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  configure_file(
    ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_openmp_C_ex.in
    ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/CMakeLists.txt @ONLY)

  # install CMakelists.txt
  install(FILES ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/CMakeLists.txt
          DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp)

  # On UNIX-type platforms, we also  generate and install a makefile for
  # building the examples. This makefile can then be used as a template for the
  # user's own programs.

  if(UNIX)
    # generate Makefile and place it in the binary dir
    configure_file(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_openmp_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/Makefile_ex @ONLY)
    # install the configured Makefile_ex as Makefile
    install(
      FILES ${PROJECT_BINARY_DIR}/examples/arkode/C_openmp/Makefile_ex
      DESTINATION ${EXAMPLES_INSTALL_PATH}/arkode/C_openmp
      RENAME Makefile)
  endif()

  # add test_install target
  sundials_add_test_install(arkode C_openmp)

endif()
