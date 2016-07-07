#include <xinu.h>

void cpuintensive(void){
	int i = 0;
	int j = 0;
	int counter = 0;
	for(i =0; i<10 ; i++){
		for(j =0; j<10000000; j++){
			//kprintf("in loop of cpu intensive\n");
			counter = i+j*2;
			//kprintf("\rcounter%d\n\r", counter);
		}
			//kprintf("\rc-counter %u\n\r", clkcount());
		kprintf("\rCPU-Intensive - CPU pid: %d \t count: %d \t Prio: %d \t TS: %d  \tClock Counter: %u\n \r", currpid, i, proctab[currpid].prprio, preempt, proctab[currpid].prclockCount);  	
	}
}
