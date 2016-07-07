#include <linux/kernel.h>
#include <linux/unistd.h>

void sys_my_hello_call(){
    printk(KERN_ALERT "Hello world!!!\n");
    return;
}
