#include <arch/x86/include/asm/unistd_64.h>
#include <stdio.h>
#include <errno.h> 

int main(int argc, char **argv) { 
    int a;
    //buffer = malloc(sizeof(long));
    if(argc < 2)
        return 0;
    int pid = atoi(argv[1]);
    int priority;
    char temp_buffer[21];
    printf("Calling ...\n");
    syscall(__NR_my_get_proc_name_by_id, pid, temp_buffer);
    memcpy(&priority, temp_buffer+17, 4);
    printf("name: %s %d\n", temp_buffer, priority);
}
