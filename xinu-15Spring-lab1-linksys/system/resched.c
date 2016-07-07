/* resched.c - resched */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* ptr to table entry for old process	*/
	struct procent *ptnew;	/* ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	



/*

	CPU INTENSIVE

*/
	
	ptold->prtotalClockCount = ptold->prtotalClockCount + (clktimeaccu - ptold->prclockCount); // gets the total time count
    //kprintf("\rtotalCount: %u, currpid = %u\r\n",ptold-> prtotalClockCount, currpid);

	if (ptold->prstate == PR_CURR) {  /* process remains running */

        ptold -> prprio = tstab[ptold->prprio].ts_tqexp;

        if (ptold -> prprio > firstkey(readylist)){

								// question wheather getfirst gets the highest priority
            preempt = tstab[ptold->prprio].ts_quantum;

						//ptold -> prclockCount = clktimeaccu; // gets the clock time at the start of the process
						//kprintf("\r\n Clock time is :%u for pid %d \n\r\n",ptold -> prclockCount, currpid);
			//return;
        }

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
    }

	else if(ptold->prstate == PR_SLEEP){

		if( preempt < tstab[ptold->prprio].ts_quantum-15){
			ptold->prprio = tstab[ptold->prprio].ts_tqexp;		
		}
		else{
			ptold -> prprio = tstab[ptold->prprio].ts_slpret;
		}	
	}

	/* NULL PROCESS
	*/
	
	currpid = dequeue(readylist);
    if(currpid == 0 && (!isempty(readylist))){
       insert(currpid, readylist, 0);
        currpid = dequeue(readylist);
    }

	

	
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	//preempt = QUANTUM;		/* reset time slice for process	*/
	preempt = tstab[ptnew->prprio].ts_quantum; // given the priority change the quantum
	//ptnew->myProcessStartTime = tstab[ptnew->prprio].ts_quantum; // initial time
	//ptnew->prclockCount = clktimeaccu;
	ptnew->prclockCount = clkcount();
    ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */

	return;
}
