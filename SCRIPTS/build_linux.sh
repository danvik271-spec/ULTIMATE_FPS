#!/usr/bin/env bash
set -euo pipefail
cmake --preset linux-ninja
cmake --build build/linux-debug --parallel
ctest --preset linux-debug-tests
