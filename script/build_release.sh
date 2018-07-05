#!/bin/bash
set -e

# Compute paths
WORKING_DIR=`pwd`
APPLICATION_ROOT=`git rev-parse --show-toplevel`

# First clean the build folder (if any)
if [ -d "$APPLICATION_ROOT/build_release" ]; then
    rm -rf $APPLICATION_ROOT/build_release
fi

# Compile application
mkdir -p $APPLICATION_ROOT/build_release
cd $APPLICATION_ROOT/build_release
cmake -DCMAKE_BUILD_TYPE=Release ..
make

