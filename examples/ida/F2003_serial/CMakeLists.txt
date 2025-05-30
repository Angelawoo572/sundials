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
# CMakeLists.txt file for the F2003 IDA serial examples
# ---------------------------------------------------------------

# Example lists are tuples "name\;type" where the type is 'develop' for examples
# excluded from 'make test' in releases

# Examples using SUNDIALS linear solvers
set(FIDA_examples "idaRoberts_dns_f2003\;develop")

if(SUNDIALS_INDEX_SIZE MATCHES "64")
  # Examples using SUNDIALS linear solvers
  list(APPEND FIDA_examples "idaHeat2D_kry_f2003\;develop")
endif()

# Specify libraries to link against
set(IDA_LIB sundials_fida_mod)

# Set-up linker flags and link libraries
set(SUNDIALS_LIBS ${IDA_LIB} ${EXE_EXTRA_LINK_LIBS})

# Add the build and install targets for each example
foreach(example_tuple ${FIDA_examples})
  # parse the example tuple
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 example_type)

  # Install fortran modules to a unique directory to avoid naming collisions
  set(CMAKE_Fortran_MODULE_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${example}.dir)

  # example source files
  sundials_add_executable(${example} ${example}.f90)

  set_target_properties(${example} PROPERTIES FOLDER "Examples")

  # add example to regression tests
  sundials_add_test(
    ${example} ${example}
    ANSWER_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    ANSWER_FILE ${example}.out
    EXAMPLE_TYPE ${example_type})

  # libraries to link against
  target_link_libraries(${example} ${SUNDIALS_LIBS})

  # install example source and out files
  if(EXAMPLES_INSTALL)
    install(FILES ${example}.f90 ${example}.out
            DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/F2003_serial)
  endif()
endforeach(example_tuple ${FIDA_examples})

# create Makefile and CMakeLists.txt for examples
if(EXAMPLES_INSTALL)

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  set(SOLVER "IDA")
  set(SOLVER_LIB "sundials_ida")
  set(SOLVER_FLIB "sundials_fida_mod")

  examples2string(FIDA_examples EXAMPLES)

  # Regardless of the platform we're on, we will generate and install
  # CMakeLists.txt file for building the examples. This file  can then be used
  # as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  configure_file(
    ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_serial_F2003_ex.in
    ${PROJECT_BINARY_DIR}/examples/ida/F2003_serial/CMakeLists.txt @ONLY)

  # install CMakelists.txt
  install(FILES ${PROJECT_BINARY_DIR}/examples/ida/F2003_serial/CMakeLists.txt
          DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/F2003_serial)

  # On UNIX-type platforms, we also  generate and install a makefile for
  # building the examples. This makefile can then be used as a template for the
  # user's own programs.

  if(UNIX)
    # generate Makefile and place it in the binary dir
    configure_file(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_serial_F2003_ex.in
      ${PROJECT_BINARY_DIR}/examples/ida/F2003_serial/Makefile_ex @ONLY)
    # install the configured Makefile_ex as Makefile
    install(
      FILES ${PROJECT_BINARY_DIR}/examples/ida/F2003_serial/Makefile_ex
      DESTINATION ${EXAMPLES_INSTALL_PATH}/ida/F2003_serial
      RENAME Makefile)
  endif()

  # add test_install target
  sundials_add_test_install(ida F2003_serial)

endif()
