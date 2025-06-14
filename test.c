#include <stdlib.h>
#include <stdio.h>

void caller_1(void)
{
	void *ptr;

	ptr = malloc(10);
	if (ptr)
		free(ptr);
}

void caller_2(void)
{
	void *ptr;

	ptr = malloc(200);
	if (ptr)
		free(ptr);
}

void caller_3(void)
{
	void *ptr;

	ptr = malloc(3000);
	if (ptr)
		free(ptr);
}

int main(int argc, char const *argv[])
{
	printf("=== test start ===\n");
	caller_1();
	caller_2();
	caller_3();
	printf("===  test end  ===\n");
	return 0;
}

