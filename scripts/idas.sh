#!/bin/bash
# ------------------------------------------------------------------------------
# Programmer(s): Radu Serban, David J. Gardner, Cody J. Balos @ LLNL
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
# Script to add IDAS files to a SUNDIALS tar-file.
# ------------------------------------------------------------------------------

set -e
set -o pipefail

tarfile=$1
distrobase=$2
doc=$3

# all remaining inputs are for tar command
shift 3
tar=$*

echo "   --- Add idas module to $tarfile"

if [ $doc = "T" ]; then
    $tar $tarfile $distrobase/doc/idas/idas_guide.pdf
    $tar $tarfile $distrobase/doc/idas/idas_examples.pdf
fi
$tar $tarfile $distrobase/doc/idas/guide/Makefile
$tar $tarfile $distrobase/doc/idas/guide/source

echo "   --- Add idas include files to $tarfile"
$tar $tarfile $distrobase/include/idas

echo "   --- Add idas source files to $tarfile"
$tar $tarfile $distrobase/src/idas

echo "   --- Add idas examples to $tarfile"
$tar $tarfile $distrobase/examples/idas

echo "   --- Add idas unit tests to $tarfile"
$tar $tarfile $distrobase/test/unit_tests/idas
