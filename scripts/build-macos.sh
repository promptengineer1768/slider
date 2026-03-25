#!/usr/bin/env bash
# scripts/build-macos.sh
# Build script for macOS (clang)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_TYPE="${1:-debug}"
PRESET="macos-clang-${BUILD_TYPE}"

check_ci_contract() {
    local workflow_file expected_token
    if [[ "${BUILD_TYPE}" == "release" ]]; then
        workflow_file="${PROJECT_ROOT}/.github/workflows/release.yml"
    else
        workflow_file="${PROJECT_ROOT}/.github/workflows/ci.yml"
    fi
    expected_token="CI_CONTRACT: ${PRESET}@1"

    if [[ ! -f "${workflow_file}" ]]; then
        echo "WARNING: CI contract check skipped; workflow file not found: ${workflow_file}" >&2
        return
    fi
    if ! grep -Fq "${expected_token}" "${workflow_file}"; then
        echo "WARNING: CI contract mismatch for ${PRESET}. Expected '${expected_token}' in ${workflow_file}" >&2
    fi
}

check_ci_contract

cd "$PROJECT_ROOT"

echo "=== Configuring ${PRESET} ==="
cmake --preset "${PRESET}"

echo "=== Building ${PRESET} ==="
cmake --build --preset "build-${PRESET}"

echo "=== Running tests ==="
ctest --preset "test-${PRESET}" --output-on-failure

echo "=== Build complete ==="
