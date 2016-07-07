#include <xinu.h>

void increasePriority(int lock);


int lock(int ldes1, int type, int priority){

	// assuming the prioroity is the priority for the process in line / waiting
	
	intmask mask1;

	int lockIdToUse; //lockId
	
	struct lentry  *lptr;
	struct procent *pptr;
	
	int lockReturn = 0;	

	mask1 = disable();
	
	lockIdToUse = ldes1%50;
	lptr = &locktab[lockIdToUse];
	//pptr = &proctab[currpid];

	if(isbadlock(lockIdToUse) || lptr->lockState == LFREE || lptr->lockDesp != ldes1){
		restore(mask1);
		return SYSERR;
	}
	if(lptr->lockFlag == 0){

		
		if(type == READ){
			lptr->lockMode = READ;
			lockReturn = 1;		
		}
		else{
			lptr->lockMode = WRITE;		
		}
	
			
		 lptr->lockFlag = 1;
		 lptr->processList[currpid] = 1;

	} 
	
	else if(lptr->lockMode == WRITE || type == WRITE){
		//kprintf("\r entered the writer state \n");
		//kprintf("\r lock id to use is %d:\n", lockIdToUse);
		pptr = &proctab[currpid];
		pptr->processLockId = lockIdToUse;
		
		//kprintf("\r LOCK PRIO in lock.c %d\n", lptr->lockPrio );
		//kprintf("\r PROCESS PRIO in lock.c %d \n",pptr-> prprio);

		if(pptr-> prprio > lptr-> lockPrio){
			lptr-> lockPrio = pptr->prprio;
			increasePriority(lockIdToUse);
		}
		
		insert(currpid, lptr->lockHead, priority);
		//kprintf("\r current pid in lock.c %d\n", currpid );
		
		pptr-> prstate = PR_WAIT;
		pptr-> prHoldingLock[lockIdToUse] = type;
		resched();
		lptr->processList[currpid] = 1;
		lockReturn = 2;		
	}

	else if(lptr-> lockMode == READ){
		int tail = lptr-> lockTail; 
		int lproc; 
		int nLFlag = 0; 
		
		while( (lproc = queuetab[tail].qprev) < NPROC && lproc != lptr-> lockHead){
			if( ( ( pptr = &proctab[lproc])-> prHoldingLock[lockIdToUse]) == WRITE){
				if( priority >= queuetab[lproc].qkey){
					lptr-> processList[currpid] = 1;
					lockReturn = 3;				
				}
				else{
					pptr = &proctab[currpid];
					pptr-> processLockId = lockIdToUse;
					if(pptr -> prprio > lptr -> lockPrio){
						lptr->lockPrio = pptr-> prprio;
						increasePriority(lockIdToUse);					
					}
					insert(currpid, lptr->lockHead, priority);
				
					pptr->prstate  = PR_WAIT;
					pptr->prHoldingLock[lockIdToUse] = type;
					resched();
					lptr-> processList[currpid] = 1;
					lockReturn = 4;				
				}
				nLFlag = 1;
				break;			
			}
			tail = queuetab[tail].qprev;
			//lockReturn = 7;
		}
		if (nLFlag == 0){
			lptr-> processList[currpid] = 1;
			lockReturn = 5;
		}	
	}
	else{
	}
	restore(mask1);
	return OK;
	
}
































	/*else if(lptr->lockFlag != 0){
		if(lptr->lockType == WRITE){
			pptr-> prstate						= PR_WAIT;
			pptr-> lockId						= ldes1;
			pptr-> prHoldingLock[lockIdToUse] 	= type;
			pptr-> waitingPrio 					= priority;
			pptr-> totalWaitingTime				= clkticks;
			increasePriority(lockIdToUse, pptr-> prprio);
	
			insert(currpid, lptr->lockHead, priority);
			
			resched();
			restore(mask);
			return (OK);
		}
	
		else if(lptr->lockType == READ){
			if(type == READ){
				lockTemp = lptr->lockHead;
				while(queuetab[lockTemp].qnext != lptr-> lockTail){
					if( (proctab[queuetab[lockTemp].qnext].prHoldingLock[lockIdToUse] == WRITE) && (priority <	proctab[queuetab[lockTemp].qnext].waitingPrio)){
						notToUse =1;					
					}
						lockTemp = queuetab[lockTemp].qnext;			
				}
				if(notToUse == 1){
					pptr-> prstate = PR_WAIT;
					pptr-> lockId  = ldes1;
					pptr-> prHoldingLock[lockIdToUse] = type;
					pptr-> waitingPrio = priority;
					increasePriority(lockIdToUse, pptr-> prprio);
					pptr->totalWaitingTime = clkticks;
					insert(currpid, lptr->lockHead, priority);				
				}
				else if( notToUse != 1){
					if(lptr -> lockPrio < pptr -> prprio){
						lptr-> lockPrio	= pptr-> prprio;					
					}
					lptr-> processList[currpid] = 1;
					pptr-> prHoldingLock[lockIdToUse] = type;
					pptr-> waitingPrio = priority;
					pptr-> lockId = -1;

					restore(mask);
					return (OK);				
				}
			}
			
			else if(type == WRITE){
				pptr-> prstate 		= PR_WAIT;
				pptr-> lockId		= ldes1;
				pptr-> prHoldingLock[lockIdToUse] = type;
				pptr-> waitingPrio = priority;
			
				increasePriority(lockIdToUse, pptr-> prprio);
				pptr-> totalWaitingTime = clkticks;
				insert(currpid, lptr->lockHead, priority);
				
				resched();
				restore(mask);
				return(OK);			
			}		
		}	
	}
		

	restore(mask);
	return OK;
}



int getDesiredLock(int desiredLock){
	int i;
	if(desiredLock > 0){
			for(i = 0; i <NLOCKS; i++){
				if(locktab[i].lockDesp == desiredLock){ // the lock has been found
					return i;			
			}	
		}
	}
	return (SYSERR);
}
*/

void increasePriority(int lock){
	int i; 
	//kprintf("\r increasing prio for lock %d\n",lock);
	struct procent *pptr;
	struct lentry *lptr;

	lptr = &locktab[lock];
		
	for( i = 1;i<NPROC; i++){
		if(lptr-> processList[i] == 1){
			int prio = 0;
			pptr = &proctab[i];
			int j = 0;
			while (j < NLOCKS){
				if( locktab[j].processList[i] == 1){
					if (locktab[j].lockPrio  > prio){
						prio = locktab[j].lockPrio;					
					}				
				}
				j++;			
			}
			if(pptr-> prprio < prio){
				//kprintf("\r increasing prio for process %d\n",i);
				pptr->prprio = prio;			
			}
			
			if(pptr-> processLockId != -1){
				increasePriority(pptr-> processLockId);			
			}		
		}
	}
}







/*

		if((proctab[i].prHoldingLock[lock] != DELETED) && (locktab[lock].processList[i] == 1)){
			if(proctab[i].mainPrio < givenCurrentPrio){
				proctab[i].prprio = givenCurrentPrio;
				updateOnRelease(i);			
			}		
		}	
	}
}

*/
