#include <xinu.h>

void iointensive(void){
	
	int j = 0;

	for(j =0; j<10; j++){			
		sleepms(5);
		kprintf("\rI/O Intensive - CPU pid: %d + count: %d + Prio: %d + TS: %d Clock Counter: %u\n\r", currpid, j, proctab[currpid].prprio, preempt, proctab[currpid].prclockCount);  	
	}
}
