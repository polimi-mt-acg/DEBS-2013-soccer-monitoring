#!/bin/bash
set -e

# Compute paths
WORKING_DIR=`pwd`
APPLICATION_ROOT=`git rev-parse --show-toplevel`

# First clean the build folder (if any)
if [ -d "$APPLICATION_ROOT/cmake-build-profile" ]; then
    rm -rf $APPLICATION_ROOT/cmake-build-profile
fi

# Compile application
mkdir -p $APPLICATION_ROOT/cmake-build-profile
cd $APPLICATION_ROOT/cmake-build-profile
cmake -DPERF_PROFILING=ON -DCMAKE_BUILD_TYPE=Release ..
make soccer-monitoring -j2
