#!/bin/bash
# scripts/bootstrap/bootstrap-macos.sh
# macOS bootstrap using Homebrew.

set -e

if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew not found. Please install Homebrew first."
    exit 1
fi

brew update
brew install \
  cmake \
  ninja \
  wxwidgets \
  googletest \
  git \
  clang-format

echo ">>> macOS build dependencies installed via Homebrew."
