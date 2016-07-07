#include <xinu.h>

/*------------------------------------------------------------------------
 * ldelete  --  Delete a lock by releasing its table entry
 *------------------------------------------------------------------------
 */

int ldelete(int lockdescriptor){

	intmask mask2; 

	int pid; // id of the process holding the lock
	int lockId; 

	struct lentry *lptr;

	mask2 = disable();
	
	lockId = lockdescriptor%50;
	lptr = &locktab[lockId];
	
	if(isbadlock(lockId) || lptr->lockState == LFREE || lptr->lockDesp != lockdescriptor){
		restore(mask2);
		return(SYSERR);	
	}
	lptr->lockState = LFREE;
	lptr->lockType++;
	locktab[lockId].lockDesp = (locktab[lockId].lockType*100) + lockId;
	lptr->lockFlag = 0;
	// make sure the everything is empty

	if(nonempty(lptr ->lockHead)){
		while((pid = getfirst(lptr->lockHead)) != EMPTY){
			proctab[pid].processWaitReturn = DELETED;
			ready(pid,RESCHED_NO);
		}
		resched();
	}
	/*
	if(nonempty(lptr ->lockHead)){
		while((pid = getlast(lptr->lockHead)) != EMPTY){
			proctab[pid].processWaitReturn = DELETED;
			proctab[pid].processLockId = -1;
			ready(pid,RESCHED_NO);
		}
		resched();
	}*/
	restore(mask2);
	return(OK);
}

