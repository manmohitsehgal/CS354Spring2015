#include <arch/x86/include/asm/unistd_64.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv) {
	printf("Calling Stack Trace.....\n");
	syscall(__NR_my_stack_trace);
	return 0;
}
