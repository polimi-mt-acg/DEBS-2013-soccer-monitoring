#!/bin/sh

set -e

ROOT=`git rev-parse --show-toplevel`

echo "==> Bootstrapping..."
echo "  Checking if submodules are set up..."
# Check if git submodules are initialized. If not do it.
if ! test "$(ls -A "$ROOT/external/fmt")"; then
    echo "==> Initializing git submodules..."
    git submodule init
fi

echo "==> Updating git submodules..."
git submodule update --recursive --remote
