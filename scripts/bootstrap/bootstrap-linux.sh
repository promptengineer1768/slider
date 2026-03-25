#!/bin/bash
# scripts/bootstrap/bootstrap-linux.sh
# Linux bootstrap for Ubuntu/Debian.

set -e

sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  libwxgtk3.2-dev \
  libgtk-3-dev \
  libgtest-dev \
  clang \
  clang-format-19 \
  git \
  rpm \
  file \
  fuse

echo ">>> Linux build dependencies installed."
