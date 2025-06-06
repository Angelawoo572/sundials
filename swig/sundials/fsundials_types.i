// ---------------------------------------------------------------
// Programmer: Seth R. Johnson @ ORNL
//             Cody J. Balos @ LLNL
// ---------------------------------------------------------------
// SUNDIALS Copyright Start
// Copyright (c) 2002-2025, Lawrence Livermore National Security
// and Southern Methodist University.
// All rights reserved.
//
// See the top-level LICENSE and NOTICE files for details.
//
// SPDX-License-Identifier: BSD-3-Clause
// SUNDIALS Copyright End
// ---------------------------------------------------------------
// Swig interface file
// ---------------------------------------------------------------

// By default, wrap all constants as native fortran PARAMETERs
%fortranconst;

%include <stdint.i>

#ifdef GENERATE_INT32
// Inform SWIG of the configure-provided types
#define SUNDIALS_INT32_T
#define SUNDIALS_INDEX_TYPE int32_t
#else
#define SUNDIALS_INT64_T
#define SUNDIALS_INDEX_TYPE int64_t
#endif
#define SUNDIALS_DOUBLE_PRECISION
#define SUNDIALS_COUNTER_TYPE long int

%ignore SUN_FORMAT_E;
%ignore SUN_FORMAT_G;
%ignore SUN_FORMAT_SG;

// Handle MPI_Comm and SUNComm
%include <typemaps.i>

%apply int { MPI_Comm };

%typemap(ftype) MPI_Comm
   "integer"
%typemap(fin, noblock=1) MPI_Comm {
    $1 = int($input, C_INT)
}
%typemap(fout, noblock=1) MPI_Comm {
    $result = int($1)
}

%typemap(in, noblock=1) MPI_Comm {
%#if SUNDIALS_MPI_ENABLED
    int flag = 0;
    MPI_Initialized(&flag);
    if(flag) {
      $1 = MPI_Comm_f2c(%static_cast(*$input, MPI_Fint));
    } else {
      $1 = SUN_COMM_NULL;
    }
%#else
    $1 = *$input;
%#endif
}
%typemap(out, noblock=1) MPI_Comm {
%#if SUNDIALS_MPI_ENABLED
    int flag = 0;
    MPI_Initialized(&flag);
    if(flag) {
      $result = %static_cast(MPI_Comm_c2f($1), int);
    } else {
      $result = 0;
    }
%#else
    $result = $1;
%#endif
}

%apply MPI_Comm { SUNComm };

// Insert code into the C wrapper to check that the sizes match
%{
#include "sundials/sundials_types.h"

#ifndef SUNDIALS_DOUBLE_PRECISION
#error "The Fortran bindings are only targeted at double-precision"
#endif
%}

// We insert the binding code for SUN_COMM_NULL ourselves because
// (1) SWIG expands SUN_COMM_NULL to its value
// (2) We need it to be equivalent to MPI_COMM_NULL when MPI is enabled

%insert("fdecl") %{
#if SUNDIALS_MPI_ENABLED
 include "mpif.h"
 integer(C_INT), protected, public :: SUN_COMM_NULL = MPI_COMM_NULL
#else
 integer(C_INT), parameter, public :: SUN_COMM_NULL = 0_C_INT
#endif
%}

// Insert SUNDIALS copyright into generated C files.
%insert(begin)
%{
/* ---------------------------------------------------------------
 * Programmer(s): Auto-generated by swig.
 * ---------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2025, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * -------------------------------------------------------------*/
%}
// Insert SUNDIALS copyright into generated Fortran files
%insert(fbegin)
%{
! ---------------------------------------------------------------
! Programmer(s): Auto-generated by swig.
! ---------------------------------------------------------------
! SUNDIALS Copyright Start
! Copyright (c) 2002-2025, Lawrence Livermore National Security
! and Southern Methodist University.
! All rights reserved.
!
! See the top-level LICENSE and NOTICE files for details.
!
! SPDX-License-Identifier: BSD-3-Clause
! SUNDIALS Copyright End
! ---------------------------------------------------------------
%}

// Process and wrap functions in the following files
%include "sundials/sundials_types.h"
