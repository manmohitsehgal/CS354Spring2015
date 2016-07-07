/* REGISTERCB.C
 * the registered callback function is executed 
 * which handles the chore of processing the 
 * received message (in our case just printing).
 */

#include <xinu.h>

syscall registercb( umsg32 *abuf, 
					int (* func) (void) ){

	struct procent * prcptr;

	prcptr = &proctab[currpid];

    if(abuf == NULL){
        ;
    }

    if(func == NULL){
        ;
    }
	
	if((abuf != NULL) && (func != NULL)){
			//	kprintf("%d\n values:\n",abuf);
				addr_buf = abuf; addr_func = func;	
	}

	return OK;
}
