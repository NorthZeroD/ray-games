#!/usr/bin/env bash

printf "========== clang++ ==========\n"; clang++ --version | head -n 1; \
    printf "========== g++ ==========\n"; g++ --version | head -n 1; \
    printf "========== cmake ==========\n"; cmake --version | head -n 1; \
    printf "========== ninja ==========\n"; ninja --version
