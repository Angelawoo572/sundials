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
# Script to add shared files to a SUNDIALS tar-file.
# ------------------------------------------------------------------------------

set -e
set -o pipefail

tarfile=$1
distrobase=$2
doc=$3
doc_sundials=$4

echo ">>>>>>"
echo $distrobase
echo ">>>>>>"

# all remaining inputs are for tar command
shift 4
tar=$*

echo "   --- Add top-level files to $tarfile"

# Some tar implementations (e.g., BSD) will not create an archive with the -u
# option so the first tar call uses -c and ignores the input tar options
tar -cvf $tarfile $distrobase/CHANGELOG.md
$tar $tarfile $distrobase/CITATIONS.md
$tar $tarfile $distrobase/CMakeLists.txt
$tar $tarfile $distrobase/CONTRIBUTING.md
$tar $tarfile $distrobase/LICENSE
$tar $tarfile $distrobase/NOTICE
$tar $tarfile $distrobase/README.md
$tar $tarfile $distrobase/.readthedocs.yaml

if [ $doc = "T" ]; then
    $tar $tarfile $distrobase/INSTALL_GUIDE.pdf
fi

if [ $doc_sundials = "T" ]; then
    $tar $tarfile $distrobase/doc/superbuild/Makefile
    $tar $tarfile $distrobase/doc/superbuild/source
fi

$tar $tarfile $distrobase/doc/shared
$tar $tarfile $distrobase/doc/requirements.txt

echo "   --- Add benchmark files to $tarfile"
$tar $tarfile $distrobase/benchmarks

echo "   --- Add configuration files to $tarfile"
$tar $tarfile $distrobase/cmake

echo "   --- Add shared include files to $tarfile"
$tar $tarfile $distrobase/include/nvector
$tar $tarfile $distrobase/include/sundials
$tar $tarfile $distrobase/include/sunlinsol
$tar $tarfile $distrobase/include/sunmatrix
$tar $tarfile $distrobase/include/sunmemory
$tar $tarfile $distrobase/include/sunnonlinsol
$tar $tarfile $distrobase/include/sunadaptcontroller
$tar $tarfile $distrobase/include/sunadjointcheckpointscheme

echo "   --- Add shared source files to $tarfile"
$tar $tarfile $distrobase/src/CMakeLists.txt
$tar $tarfile $distrobase/src/nvector
$tar $tarfile $distrobase/src/sundials
$tar $tarfile $distrobase/src/sunlinsol
$tar $tarfile $distrobase/src/sunmatrix
$tar $tarfile $distrobase/src/sunmemory
$tar $tarfile $distrobase/src/sunnonlinsol
$tar $tarfile $distrobase/src/sunadaptcontroller
$tar $tarfile $distrobase/src/sunadjointcheckpointscheme

echo "   --- Add examples to $tarfile"
$tar $tarfile $distrobase/examples/CMakeLists.txt
$tar $tarfile $distrobase/examples/templates
$tar $tarfile $distrobase/examples/utilities

echo "   --- Add testing files to $tarfile"
$tar $tarfile $distrobase/test/testRunner

echo "   --- Add unit tests files to $tarfile"
$tar $tarfile $distrobase/test/unit_tests/

echo "   --- Add external files to $tarfile"
$tar $tarfile $distrobase/external/

echo "   --- Add tools to $tarfile"
$tar $tarfile $distrobase/tools/
