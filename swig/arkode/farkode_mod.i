// ---------------------------------------------------------------
// Programmer: Cody J. Balos @ LLNL
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

%module farkode_mod

%include "../sundials/fsundials.i"

%{
#include "arkode/arkode.h"
#include "arkode/arkode_bandpre.h"
#include "arkode/arkode_bbdpre.h"
#include "arkode/arkode_butcher.h"
#include "arkode/arkode_butcher_dirk.h"
#include "arkode/arkode_butcher_erk.h"
#include "arkode/arkode_sprk.h"
#include "arkode/arkode_ls.h"
%}

%import "../sundials/fsundials_core_mod.i"

// Treat ARKodeButcherTable as an opaque pointer
%apply void* { ARKodeButcherTable };

// Treat ARKodeSPRKTable as an opaque pointer
%apply void* { ARKodeSPRKTable };

// Process definitions from these files
%include "arkode/arkode.h"
%include "arkode/arkode_bandpre.h"
%include "arkode/arkode_bbdpre.h"
%include "arkode/arkode_butcher.h"
%include "arkode/arkode_butcher_dirk.h"
%include "arkode/arkode_butcher_erk.h"
%include "arkode/arkode_sprk.h"
%include "arkode/arkode_ls.h"

