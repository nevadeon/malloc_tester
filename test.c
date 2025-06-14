#include <stdlib.h>
#include <stdio.h>

void function_1(void)
{
	void *ptr;

	ptr = malloc(10);
	if (ptr)
		free(ptr);
}

void function_2(void)
{
	void *ptr;

	ptr = malloc(200);
	if (ptr)
		free(ptr);
}

void function_3(void)
{
	void *ptr;

	ptr = malloc(3000);
	if (ptr)
		free(ptr);
}

int main(int argc, char const *argv[])
{
	printf("=== test start ===\n");
	function_1();
	function_2();
	function_3();
	printf("===  test end  ===\n");
	return 0;
}

