/* receiveb.c - receiveb */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receiveb  -  wait for a message and return the message to the caller
 *  	      -  Checks for any blocked messages
 *  	      -  Unblocks sending process that has been waiting the longest
 *  	      -  Uses FIFO implementation
 *------------------------------------------------------------------------
 */
umsg32	receiveb(void)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/
	umsg32	msg;			/* message to return		*/

	mask = disable();
	prptr = &proctab[currpid];

	if (prptr->prhasmsg == FALSE) {
		prptr->prstate = PR_RECV;
		resched();		/* block until message arrives	*/
	}

	msg = prptr->prmsg;		/* retrieve message		*/
	prptr-> prhasmsg = FALSE;
//	prptr->sndflag = FALSE;	/* reset message flag		*/

	if(!isempty(prptr->messagesQueue)){
		struct procent *processPtr;
		pid32 senderPid;
	
		int isValid = 0;
		while (isValid == 0){
			pid32 pidToSend;

			if(!isempty(prptr-> messagesQueue)){
				pidToSend = dequeue(prptr-> messagesQueue);
				processPtr = &proctab[pidToSend];

				if(processPtr -> sndflag == FALSE){	
					isValid = 0;				
				}
				else{
					senderPid = pidToSend;
					isValid = 1;				
				}

			}
			else{
				restore(mask);
				return msg;			
			}
		}

		prptr-> prmsg = processPtr -> sndmsg;
		prptr-> prhasmsg = TRUE;
		processPtr -> sndflag = FALSE;

		ready(senderPid, RESCHED_YES);		
	}
	
	restore(mask);
	return msg;
}
