#!/bin/bash

set -e

# Check if target program is provided
if [ -z "$1" ]; then
	echo "Usage: $0 <target_program> [args...]"
	exit 1
fi

echo "Warning: make sure the targeted project is compiled with -rdynamic -g"

# Build the malloc tester
echo "[script] Compiling malloc_tester.so..."
gcc -fPIC -shared -o malloc_tester.so malloc_tester.c -ldl -g

# Run the target program with LD_PRELOAD
echo "[script] Running malloc tester with $1"
LD_PRELOAD=./malloc_tester.so "$@"
