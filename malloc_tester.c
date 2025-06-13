#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

/**
 * [HOW TO USE]
 *
 * compile command
 * gcc -fPIC -shared -o malloc_tester.so malloc_tester.c -ldl
 *
 * execution command
 * LD_PRELOAD=./malloc_tester.so ./program_to_test
 *
 * gdb usage
 * add ```setenv("MALLOC_TESTER_ENABLE", "1", 1);``` at the beginning of main
 * and run LD_PRELOAD=./malloc_tester.so gdb ./program_to_test
 *
 *
 * [OPTIONS]
 * MALLOC_SKIP_COUNT   — skip first N allocations
 * MALLOC_MAX_CALLS    — fail all calls after N total allocations
 * MALLOC_MAX_MEMORY   — fail all calls after N total allocated bytes
 * MALLOC_FAIL_PERCENT — fail each call with N% probability
 * MALLOC_PRINT        — print log in stderr
 */

#define SKIP_COUNT_DEFAULT 0
#define MAX_CALLS_DEFAULT -1
#define MAX_MEMORY_DEFAULT -1
#define FAIL_PERCENT_DEFAULT 20
#define PRINT_BOOL_DEFAULT 1

static int malloc_active = 0;

static int	parse_env(const char *name, int default_value)
{
	char *value;

	value = getenv(name);
	if (value)
		return atoi(value);
	return default_value;
}

static void	print_alloc_status(int count, size_t size, size_t total, const char *status)
{
	fprintf(stderr, "[malloc] call #%d | size: %zu | total: %zu | %s\n",
		count, size, total, status);
}

void *malloc(size_t size)
{
	static void *(*real_malloc)(size_t);
	static size_t used_memory = 0;
	static int count = 0;

	if (!real_malloc)
	{
		real_malloc = dlsym(RTLD_NEXT, "malloc");
		srand((unsigned int)(size_t)real_malloc);
	}

	if (!getenv("MALLOC_TESTER_ENABLE"))
		return real_malloc(size);

	int skip_count = parse_env("MALLOC_SKIP_COUNT", SKIP_COUNT_DEFAULT);
	int max_calls = parse_env("MALLOC_MAX_CALLS", MAX_CALLS_DEFAULT);
	int max_memory = parse_env("MALLOC_MAX_MEMORY", MAX_MEMORY_DEFAULT);
	int fail_percent = parse_env("MALLOC_FAIL_PERCENT", FAIL_PERCENT_DEFAULT);
	int print_alloc = parse_env("MALLOC_PRINT_BOOL", PRINT_BOOL_DEFAULT);

	count++;
	int effective_count = count - skip_count;

	if (skip_count > 0 && count <= skip_count)
		return real_malloc(size);

	if (max_calls >= 0 && effective_count > max_calls)
	{
		if (print_alloc)
			print_alloc_status(count, size, used_memory, "DENIED (max alloc)");
		return NULL;
	}

	if (max_memory >= 0 && used_memory > max_memory)
	{
		if (print_alloc)
			print_alloc_status(count, size, used_memory, "DENIED (max memory)");
		return NULL;
	}

	if (fail_percent >= 0 && (rand() % 100) < fail_percent)
	{
		if (print_alloc)
			print_alloc_status(count, size, used_memory, "DENIED (random failure)");
		return NULL;
	}

	used_memory += size;

	if (print_alloc)
		print_alloc_status(count, size, used_memory, "ALLOWED");

	return real_malloc(size);
}
