/* receive.c - receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receiveq(void)
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

	
	//msg = prptr->prmsg;		/* retrieve message		*/
	//prptr->prhasmsg = FALSE;	/* reset message flag		*/

	msg = deaqueueMessageToSend(currpid);
	
	if(prptr -> totalNumberOfMessagesToSend == 0){
		prptr-> prhasmsg = FALSE;	
	}

	
	if(!isempty(prptr-> messagesQueue) && prptr-> totalNumberOfMessagesToSend < MSGQ_SIZE){
		struct procent *processPtr;
		pid32 senderPid;
	
		int isValid = 0;
		while (isValid == 0){
			pid32 pidToSend;

			if(isempty(prptr-> messagesQueue)){
				restore(mask);
				return msg;		
			}
			else{
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

			/*if(!isempty(prptr-> messagesQueue)){
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
			}*/
		}

		enqueueMessageToSend(processPtr->sndmsg, currpid);
		prptr-> prhasmsg = TRUE;
		processPtr -> sndflag = FALSE;

		ready(senderPid, RESCHED_YES);	
	}
	restore(mask);
	return msg;
}
