#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

/**
 * ┌───────────────────────────────────────────────────────────────────────────┐
 * │                              HOW TO USE                                   │
 * └───────────────────────────────────────────────────────────────────────────┘
 *
 * REQUIREMENTS:
 * For best results, compile the target program using `gcc` with debug symbols:
 *     gcc -rdynamic -g target.c -o target_program
 *
 * RECOMMENDED:
 * Use the provided script to simplify setup and execution:
 *     ./run_tester.sh target_program [args...]
 *     ./run_tester.sh gdb target_program [args...]
 *
 * Compile the tester:
 *     gcc -fPIC -shared -o malloc_tester.so malloc_tester.c -ldl -g
 *
 * Run the tester:
 *     LD_PRELOAD=./malloc_tester.so TARGET_BIN=./target_program ./target_program
 *
 * Run in GDB:
 *     LD_PRELOAD=./malloc_tester.so TARGET_BIN=./target_program gdb ./target_program
 *
 * Configuration can be modified live during GDB execution:
 *     set malloc_cfg.max_calls = <int>
 *     set malloc_cfg.fail_percent = <int>
 *     set malloc_cfg.max_memory = <int>
 *
 * Configuration options
 * 	   max_calls        - maximum number of allocations (-1 = unlimited)
 *     max_memory       - maximum available bytes for allocations (-1 = unlimited)
 *     fail_percent     - fail probability (0–100)
 *     rejected_symbols - ignored function calls
 */

struct malloc_cfg {
	int max_calls;
	int max_memory;
	int fail_percent;
};

struct malloc_cfg malloc_cfg = {
	.max_calls = -1,
	.max_memory = -1,
	.fail_percent = 0
};

// If a function caller name contains any of these strings it will be ignored
const char *rejected_symbols[] = {
	"mlx",
	"MLX",
	"_IO_file_doallocate",
	NULL
};

static void print_alloc_status(
	int count,
	size_t size,
	size_t total,
	const char *status,
	const char *caller,
	void *caller_address
)
{
	fprintf(stderr,
		"[malloc] call #%-3d | size: %-5zu | total: %-6zu | from: (%p) %-20s | %s\n",
		count, size, total, caller_address, caller ? caller : "unknown", status);
}

static bool is_injection_allowed(Dl_info info)
{
	const char *target_binary;
	char caller_path[PATH_MAX];
	char target_path[PATH_MAX];
	int  i;

	target_binary = getenv("TARGET_BIN");
	if (!target_binary)
		return false;
	if (!realpath(target_binary, target_path))
		return false;
	if (!realpath("/proc/self/exe", caller_path))
		return false;
	// fprintf(stderr, "caller=%s target=%s\n", caller_path, target_path);

	// Reject if the caller is not from the target binary
	if (strcmp(caller_path, target_path) != 0)
		return false;

	// Reject based on symbol name
	if (info.dli_sname)
	{
		i = 0;
		while(rejected_symbols[i]){
			if (strstr(info.dli_sname, rejected_symbols[i]))
			return false;
			i++;
		}
	}

	return true;
}

void *malloc(size_t size)
{
	static void    *(*real_malloc)(size_t) = NULL;
	static size_t   used_memory = 0;
	static int      count = 0;
	void 			*caller;
	Dl_info 		info;

	if (!real_malloc) {
		real_malloc = dlsym(RTLD_NEXT, "malloc");
		srand((unsigned int)(size_t)real_malloc);
	}

	caller = __builtin_return_address(0);
	if (!caller)
		return real_malloc(size);
	if (!dladdr(caller, &info))
		return real_malloc(size);
	if (!is_injection_allowed(info))
		return real_malloc(size);

	// fprintf(stderr, "address=%p bin=%s name=%s\n", caller, info.dli_fname, info.dli_sname);

	count++;

	if (malloc_cfg.max_calls >= 0 && count > malloc_cfg.max_calls) {
		print_alloc_status(count, size, used_memory, "DENIED (max alloc)", info.dli_sname, caller);
		return NULL;
	}
	if (malloc_cfg.max_memory >= 0 && used_memory > malloc_cfg.max_memory) {
		print_alloc_status(count, size, used_memory, "DENIED (max memory)", info.dli_sname, caller);
		return NULL;
	}
	if (malloc_cfg.fail_percent >= 0 && (rand() % 100) < malloc_cfg.fail_percent) {
		print_alloc_status(count, size, used_memory, "DENIED (random failure)", info.dli_sname, caller);
		return NULL;
	}

	used_memory += size;
	print_alloc_status(count, size, used_memory, "ALLOWED", info.dli_sname, caller);
	return real_malloc(size);
}
