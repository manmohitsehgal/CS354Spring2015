#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sched.h>

void sys_my_get_proc_name_by_id(int pid, char *user_buf){
    struct task_struct *t;
    t = find_task_by_vpid(pid);

    if(t == NULL){
        printk(KERN_ALERT "Error: PID is not valid!\n");
        return;
    }
	
    char buf[21];
	int priority;
	
    get_task_comm(buf,t);
    buf[16] = '\0';
	priority = t->normal_prio; //The spec of task_struct could be found in include/linux/sched.h
    memcpy(buf+17, &priority, 4);
    copy_to_user(user_buf, buf, 21);

    return;
}
