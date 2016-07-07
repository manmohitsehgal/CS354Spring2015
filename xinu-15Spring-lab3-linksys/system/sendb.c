/* send.c - send */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  sendb  -  pass a message to the process using BLOCKING.
 *  
 *  If(msgBuffer == empty): same as send
 *  else if (msgBuffer != empty): 
 * 	- blocks until the receiver receives the message and receiveb
 *	  unblocks sender
 *------------------------------------------------------------------------
 */

syscall	sendb(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* contents of message		*/
	)
{
	intmask	mask;			/* saved interrupt mask		*/
	struct	procent *prptr;		/* ptr to process' table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prstate == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

		/* another pointer to process table */



    /* CONDITIONS:
     * if process has a message
     * - sendflag = true
     * - state changes to pr send
     * else
     * - state of prhasmsg is true
     */

	if( prptr -> prhasmsg){
		struct procent *sendMessage;
		sendMessage = &proctab[currpid];

		sendMessage -> prstate	= PR_SND; 	/* setting the message state to PR_SEND */
		sendMessage -> sndmsg  	= msg;
		sendMessage -> sndflag 	= TRUE;		/* message to send */
	
		enqueue(currpid, prptr -> messagesQueue);
		resched();

	}

	prptr->prmsg = msg;		/* deliver message		*/
	prptr->prhasmsg = TRUE;		/* indicate message is waiting	*/

	if (prptr->prstate == PR_RECV) { ready(pid, RESCHED_YES );
	} 
    else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid, RESCHED_YES);
	}
	restore(mask);		/* restore interrupts */
	return OK;
}
