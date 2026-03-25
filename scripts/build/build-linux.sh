#!/usr/bin/env bash
# scripts/build/build-linux.sh
# Build script for Linux (clang)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$(dirname "$SCRIPT_DIR")")"
BUILD_TYPE="${1:-debug}"
PRESET="linux-clang-${BUILD_TYPE}"

cd "$PROJECT_ROOT"

echo "=== Configuring ${PRESET} ==="
cmake --preset "${PRESET}" \
    -DCMAKE_C_COMPILER=clang-19 \
    -DCMAKE_CXX_COMPILER=clang++-19

echo "=== Building ${PRESET} ==="
cmake --build --preset "build-${PRESET}"

echo "=== Running tests ==="
ctest --preset "test-${PRESET}" --output-on-failure

echo "=== Build complete ==="
