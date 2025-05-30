# ---------------------------------------------------------------
# Programmer(s): Radu Serban @ LLNL
#                David J. Gardner @ LLNL
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
# CMakeLists.txt file for CVODES serial examples
# ---------------------------------------------------------------

# Example lists are tuples "name\;args\;type" where the type is 'develop' for
# examples excluded from 'make test' in releases

# Examples using SUNDIALS linear solvers
set(CVODES_examples
    "cvsAdvDiff_ASAi_bnd\;\;develop"
    "cvsAdvDiff_FSA_non\;-sensi sim t\;exclude-single"
    "cvsAdvDiff_FSA_non\;-sensi stg t\;exclude-single"
    "cvsAdvDiff_bnd\;\;develop"
    "cvsAnalytic_mels\;\;develop"
    "cvsDirectDemo_ls\;\;develop"
    "cvsDiurnal_FSA_kry\;-sensi sim t\;exclude-single"
    "cvsDiurnal_FSA_kry\;-sensi stg t\;exclude-single"
    "cvsDiurnal_kry\;\;develop"
    "cvsDiurnal_kry_bp\;\;develop"
    "cvsFoodWeb_ASAi_kry\;\;develop"
    "cvsFoodWeb_ASAp_kry\;\;develop"
    "cvsHessian_ASA_FSA\;\;exclude-single"
    "cvsKrylovDemo_ls\;\;develop"
    "cvsKrylovDemo_ls\;1\;develop"
    "cvsKrylovDemo_ls\;2\;develop"
    "cvsKrylovDemo_prec\;\;develop"
    "cvsLotkaVolterra_ASA\;\;develop"
    "cvsParticle_dns\;\;develop"
    "cvsPendulum_dns\;\;exclude-single"
    "cvsRoberts_ASAi_dns\;\;exclude-single"
    "cvsRoberts_ASAi_dns_constraints\;\;develop"
    "cvsRoberts_FSA_dns\;-sensi sim t\;exclude-single"
    "cvsRoberts_FSA_dns\;-sensi stg1 t\;exclude-single"
    "cvsRoberts_FSA_dns_Switch\;\;exclude-single"
    "cvsRoberts_FSA_dns_constraints\;-sensi stg1 t\;exclude-single"
    "cvsRoberts_dns\;\;"
    "cvsRoberts_dns_constraints\;\;develop"
    "cvsRoberts_dns_uw\;\;develop")

if(SUNDIALS_BUILD_WITH_MONITORING)
  list(APPEND CVODE_examples "cvsKrylovDemo_ls\;0 1\;develop")
endif()

# Examples using LAPACK linear solvers
set(CVODES_examples_BL "cvsAdvDiff_bndL\;\;exclude-single"
                       "cvsRoberts_dnsL\;\;exclude-single")

# Examples using KLU linear solver
set(CVODES_examples_KLU
    "cvsRoberts_ASAi_klu\;\;develop"
    "cvsRoberts_FSA_klu\;-sensi stg1 t\;develop" "cvsRoberts_klu\;\;develop")

# Examples using SuperLU_MT linear solver
set(CVODES_examples_SUPERLUMT
    "cvsRoberts_ASAi_sps\;\;exclude-single"
    "cvsRoberts_FSA_sps\;-sensi stg1 t\;exclude-single"
    "cvsRoberts_sps\;\;develop")

# Auxiliary files to install
set(CVODES_extras
    plot_cvsParticle.py
    plot_cvsPendulum.py
    cvsRoberts_ASAi_dns_bkw1_stats.csv
    cvsRoberts_ASAi_dns_bkw2_stats.csv
    cvsRoberts_ASAi_dns_fwd_stats.csv
    cvsRoberts_dns_stats.csv
    cvsRoberts_FSA_dns_stats_-sensi_sim_t.csv
    cvsRoberts_FSA_dns_stats_-sensi_stg1_t.csv)

# Specify libraries to link against
set(CVODES_LIB sundials_cvodes)
set(NVECS_LIB sundials_nvecserial)

# Set-up linker flags and link libraries
set(SUNDIALS_LIBS ${CVODES_LIB} ${NVECS_LIB} ${EXE_EXTRA_LINK_LIBS})

# Add the build and install targets for each example
foreach(example_tuple ${CVODES_examples})

  # parse the example tuple
  list(GET example_tuple 0 example)
  list(GET example_tuple 1 example_args)
  list(GET example_tuple 2 example_type)

  # check if this example has already been added, only need to add example
  # source files once for testing with different inputs
  if(NOT TARGET ${example})
    # example source files
    sundials_add_executable(${example} ${example}.c)

    # folder to organize targets in an IDE
    set_target_properties(${example} PROPERTIES FOLDER "Examples")

    # libraries to link against
    target_link_libraries(${example} ${SUNDIALS_LIBS})
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
    ANSWER_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    ANSWER_FILE ${test_name}.out
    EXAMPLE_TYPE ${example_type})

  # find all .out files for this example
  file(GLOB example_out ${example}*.out)

  # install example source and .out files
  if(EXAMPLES_INSTALL)
    install(FILES ${example}.c ${example_out}
            DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial)
  endif()

endforeach(example_tuple ${CVODES_examples})

# Add the build and install targets for each LAPACK example (if needed)
if(BUILD_SUNLINSOL_LAPACKBAND AND BUILD_SUNLINSOL_LAPACKDENSE)

  # Sundials LAPACK linear solver modules
  set(SUNLINSOLLAPACK_LIBS sundials_sunlinsollapackband
                           sundials_sunlinsollapackdense)

  foreach(example_tuple ${CVODES_examples_BL})

    # parse the example tuple
    list(GET example_tuple 0 example)
    list(GET example_tuple 1 example_args)
    list(GET example_tuple 2 example_type)

    # check if this example has already been added, only need to add example
    # source files once for testing with different inputs
    if(NOT TARGET ${example})
      # example source files
      sundials_add_executable(${example} ${example}.c)

      # folder to organize targets in an IDE
      set_target_properties(${example} PROPERTIES FOLDER "Examples")

      # libraries to link against
      target_link_libraries(${example} ${SUNDIALS_LIBS} ${SUNLINSOLLAPACK_LIBS})
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
      ANSWER_DIR ${CMAKE_CURRENT_SOURCE_DIR}
      ANSWER_FILE ${test_name}.out
      EXAMPLE_TYPE ${example_type})

    # find all .out files for this example
    file(GLOB example_out ${example}*.out)

    # install example source and .out files
    if(EXAMPLES_INSTALL)
      install(FILES ${example}.c ${example_out}
              DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial)
    endif()

  endforeach(example_tuple ${CVODES_examples_BL})

endif()

# Add the build and install targets for each KLU example (if needed)
if(BUILD_SUNLINSOL_KLU)

  # Sundials KLU linear solver module
  set(SUNLINSOLKLU_LIBS sundials_sunlinsolklu)

  # KLU libraries
  list(APPEND SUNLINSOLKLU_LIBS)

  foreach(example_tuple ${CVODES_examples_KLU})

    # parse the example tuple
    list(GET example_tuple 0 example)
    list(GET example_tuple 1 example_args)
    list(GET example_tuple 2 example_type)

    # check if this example has already been added, only need to add example
    # source files once for testing with different inputs
    if(NOT TARGET ${example})
      # add example source files
      sundials_add_executable(${example} ${example}.c)

      # folder to organize targets in an IDE
      set_target_properties(${example} PROPERTIES FOLDER "Examples")

      # libraries to link against
      target_link_libraries(${example} ${SUNDIALS_LIBS} ${SUNLINSOLKLU_LIBS})
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
      ANSWER_DIR ${CMAKE_CURRENT_SOURCE_DIR}
      ANSWER_FILE ${test_name}.out
      EXAMPLE_TYPE ${example_type})

    # find all .out files for this example
    file(GLOB example_out ${example}*.out)

    # install example source and .out files
    if(EXAMPLES_INSTALL)
      install(FILES ${example}.c ${example_out}
              DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial)
    endif()

  endforeach(example_tuple ${CVODES_examples_KLU})

endif()

# Add the build and install targets for each SuperLU_MT example (if needed)
if(BUILD_SUNLINSOL_SUPERLUMT)

  # Sundials SuperLU_MT linear solver module
  set(SUNLINSOLSLUMT_LIBS sundials_sunlinsolsuperlumt)

  # SuperLU_MT libraries
  list(APPEND SUNLINSOLSLUMT_LIBS ${SUPERLUMT_LIBRARIES})

  foreach(example_tuple ${CVODES_examples_SUPERLUMT})

    # parse the example tuple
    list(GET example_tuple 0 example)
    list(GET example_tuple 1 example_args)
    list(GET example_tuple 2 example_type)

    # check if this example has already been added, only need to add example
    # source files once for testing with different inputs
    if(NOT TARGET ${example})
      # add example source files
      sundials_add_executable(${example} ${example}.c)

      # folder to organize targets in an IDE
      set_target_properties(${example} PROPERTIES FOLDER "Examples")

      # libraries to link against
      target_link_libraries(${example} ${SUNDIALS_LIBS} ${SUNLINSOLSLUMT_LIBS})
    endif()

    # check if example args are provided and set the test name
    if("${example_args}" STREQUAL "")
      set(test_name ${example})
    else()
      string(REGEX REPLACE " " "_" test_name ${example}_${example_args})
    endif()

    # Do not include SuperLUMT examples in testing when the indextype is
    # int32_t. Answer files were generated with int64_t and minor differences in
    # output occur causing a false positive when testing. These tests can be
    # re-enabled when type specific answer files are added.
    if(SUNDIALS_INDEX_SIZE MATCHES "64")
      # add example to regression tests
      sundials_add_test(
        ${test_name} ${example}
        TEST_ARGS ${example_args}
        ANSWER_DIR ${CMAKE_CURRENT_SOURCE_DIR}
        ANSWER_FILE ${test_name}.out
        EXAMPLE_TYPE ${example_type})
    endif()

    # find all .out files for this example
    file(GLOB example_out ${example}*.out)

    # install example source and .out files
    if(EXAMPLES_INSTALL)
      install(FILES ${example}.c ${example_out}
              DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial)
    endif()

  endforeach(example_tuple ${CVODES_examples_SUPERLUMT})

endif()

# create Makefile and CMakeLists.txt for examples
if(EXAMPLES_INSTALL)

  # Install the README file
  install(FILES README DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial)

  # Install the extra files
  foreach(extrafile ${CVODES_extras})
    install(FILES ${extrafile}
            DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial)
  endforeach()

  # Prepare substitution variables for Makefile and/or CMakeLists templates
  set(SOLVER "CVODES")
  set(SOLVER_LIB "sundials_cvodes")

  examples2string(CVODES_examples EXAMPLES)

  if(BUILD_SUNLINSOL_LAPACKBAND AND BUILD_SUNLINSOL_LAPACKDENSE)
    examples2string(CVODES_examples_BL EXAMPLES_BL)
  else()
    set(EXAMPLES_BL "")
  endif()

  if(BUILD_SUNLINSOL_KLU)
    examples2string(CVODES_examples_KLU EXAMPLES_KLU)
  else()
    set(EXAMPLES_KLU "")
  endif()

  if(BUILD_SUNLINSOL_SUPERLUMT)
    examples2string(CVODES_examples_SUPERLUMT EXAMPLES_SLUMT)
    if(SUNDIALS_SUPERLUMT_THREAD_TYPE STREQUAL "PTHREAD")
      set(THREAD_LIBRARY_SLUMT ${CMAKE_THREAD_LIBS_INIT})
    else()
      set(THREAD_LIBRARY_SLUMT "")
    endif()
  else()
    set(EXAMPLES_SLUMT "")
    set(THREAD_LIBRARY_SLUMT "")
  endif()

  # Regardless of the platform we're on, we will generate and install
  # CMakeLists.txt file for building the examples. This file  can then be used
  # as a template for the user's own programs.

  # generate CMakelists.txt in the binary directory
  configure_file(
    ${PROJECT_SOURCE_DIR}/examples/templates/cmakelists_serial_C_ex.in
    ${PROJECT_BINARY_DIR}/examples/cvodes/serial/CMakeLists.txt @ONLY)

  # install CMakelists.txt
  install(FILES ${PROJECT_BINARY_DIR}/examples/cvodes/serial/CMakeLists.txt
          DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial)

  # On UNIX-type platforms, we also  generate and install a makefile for
  # building the examples. This makefile can then be used as a template for the
  # user's own programs.

  if(UNIX)
    # generate Makefile and place it in the binary dir
    configure_file(
      ${PROJECT_SOURCE_DIR}/examples/templates/makefile_serial_C_ex.in
      ${PROJECT_BINARY_DIR}/examples/cvodes/serial/Makefile_ex @ONLY)
    # install the configured Makefile_ex as Makefile
    install(
      FILES ${PROJECT_BINARY_DIR}/examples/cvodes/serial/Makefile_ex
      DESTINATION ${EXAMPLES_INSTALL_PATH}/cvodes/serial
      RENAME Makefile)
  endif()

  # add test_install target
  sundials_add_test_install(cvodes serial EXECUTABLE cvsRoberts_dns)

endif()
