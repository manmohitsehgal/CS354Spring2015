#include <xinu.h>

void hybridprocess(void){
	
	int j = 0;
    int i = 0;
	int z = 0; 
   // int count = 0;
    for(i = 0; i<100; i++){
     	
	    for(j =0; j<700000; j++){
          //  count = i+j;
		//	count = i*j;
		 z = 34665 + 45646 ;
        }
		sleepms(1);        
		kprintf("\rHybrid-Intensive - CPU pid: %d \t count: %d \t Prio: %d \t TS: %d  \tClock Counter: %u \n \r", currpid, j, proctab[currpid].prprio, preempt, proctab[currpid].prclockCount);  	
		//sleepms(1);
	} 
}
