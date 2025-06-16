#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define RED     "\001\033[0;31m\002"
#define GREEN   "\001\033[0;32m\002"
#define RESET   "\001\033[0m\002"

struct malloc_cfg {
	int  max_calls;
	int  max_memory;
	int  fail_percent;
	bool ignore_anonymous_functions;
	bool print_log;
};

struct malloc_cfg malloc_cfg = {
	.max_calls = -1,
	.max_memory = -1,
	.fail_percent = 10,
	.ignore_anonymous_functions = false,
	.print_log = true
};

// If a function caller name contains any of these strings it will be ignored
const char *ignored_function_names[] = {
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
		"[malloc] call #%-3d | size: %-5zu | total: %-5zu | from: (%p) %-20s | %s\n",
		count, size, total, caller_address, caller ? caller : "unknown", status);
}

static bool is_injection_allowed(Dl_info info)
{
	const char *target_binary;
	char self_exe_path[PATH_MAX];
	char fname_path[PATH_MAX];
	char target_path[PATH_MAX];
	int  i;

	target_binary = getenv("TARGET_BIN");
	if (!target_binary)
		return false;
	if (!realpath(target_binary, target_path))
		return false;
	if (!realpath(info.dli_fname, fname_path))
		return false;
	if (!realpath("/proc/self/exe", self_exe_path))
		return false;

	// Reject if the caller is not from the target binary
	if (strcmp(self_exe_path, target_path) != 0)
		return false;
	if (strcmp(fname_path, target_path) != 0)
		return false;

	// fprintf(stderr, "caller: %s | sname: %s | fname: %s\n", self_exe_path, info.dli_sname, info.dli_fname);

	// Reject if anonymous function
	if (malloc_cfg.ignore_anonymous_functions && !info.dli_sname)
		return false;

	// Reject based on function name
	if (info.dli_sname)
	{
		i = 0;
		while(ignored_function_names[i]){
			if (strstr(info.dli_sname, ignored_function_names[i]))
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

	count++;
	if (malloc_cfg.max_calls >= 0 && count > malloc_cfg.max_calls) {
		if (malloc_cfg.print_log)
			print_alloc_status(count, size, used_memory, RED"DENIED"RESET" (max alloc)", info.dli_sname, caller);
		return NULL;
	}
	if (malloc_cfg.max_memory >= 0 && used_memory > malloc_cfg.max_memory) {
		if (malloc_cfg.print_log)
			print_alloc_status(count, size, used_memory, RED"DENIED"RESET" (max memory)", info.dli_sname, caller);
		return NULL;
	}
	if (malloc_cfg.fail_percent >= 0 && (rand() % 100) < malloc_cfg.fail_percent) {
		if (malloc_cfg.print_log)
			print_alloc_status(count, size, used_memory, RED"DENIED"RESET" (random failure)", info.dli_sname, caller);
		return NULL;
	}

	used_memory += size;
	if (malloc_cfg.print_log)
		print_alloc_status(count, size, used_memory, GREEN"ALLOWED"RESET, info.dli_sname, caller);
	return real_malloc(size);
}
