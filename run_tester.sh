#!/bin/bash

set -e

if [ -z "$1" ]; then
	echo "Usage: $0 <target_program> [args...]"
	exit 1
fi

echo "Warning: make sure the targeted project is compiled with gcc and -rdynamic -g"

echo "[script] Compiling malloc_tester.so..."
gcc -fPIC -shared -o malloc_tester.so malloc_tester.c -ldl -g

if [[ "$1" == "gdb" || "$1" == "pwndbg" ]]; then
	if [ -z "$2" ]; then
		echo "[error] You must provide a target when using gdb or pwndbg."
		exit 1
	fi
	TARGET="$2"
else
	TARGET="$1"
fi

echo "[script] Running malloc tester with $1"
LD_PRELOAD=./malloc_tester.so TARGET_BIN="$TARGET" "$@"
