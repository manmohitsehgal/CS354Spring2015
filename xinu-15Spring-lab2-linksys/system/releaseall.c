#include <xinu.h>

void relaseTheLock(int pid, int lockKey, int whereFrom);
void updateOnRelease(int pid);
void changeOnRelease(int pid);
void toNext(int lockId);

int releaseall (int numlocks, int arguments, ...){

	intmask mask4;
	unsigned long *stackAddressPtr;
	
	int i = 0;
	int nLDesp; //ldes;
	int	lockIdToUse;
	
	struct lentry  *lptr;
	//lptr = &locktab[lockIdToUse];
	struct procent *pptr;
	
	//int lockReturn = 0;
	
	int errorOrNot = OK;
	

	mask4 = disable();

	while( i < numlocks){
		stackAddressPtr = (unsigned long *)(&arguments) + i;
		nLDesp = *stackAddressPtr;
	
		lockIdToUse = nLDesp%50;
		lptr = &locktab[lockIdToUse];

		//kprintf("release lock is %d", lockIdToUse);
		
		if(isbadlock(lockIdToUse) || lptr->lockState == LFREE || lptr->lockDesp != nLDesp){
				errorOrNot = SYSERR;
				i = i+1;
				continue;
		}
		
		// reset the lock state here
		lptr-> processList[currpid] = 0;
		
		// check here if any reader has acquired a lock
		
		if(lptr-> lockMode == READ){
			int j;
			int readerFlag = 0; // flag checks if a reader has occupied lock
			
			for(j =0; j<NPROC; j++){
				if(lptr-> processList[j] == 1){
					readerFlag = 1;
					break;		
				}			
			}
			
			if(readerFlag == 1){
				int tail = lptr->lockTail;
				int lproc; // proc
				int pid; 
				while ( ( lproc = queuetab[tail].qprev) < NPROC && lproc != lptr-> lockHead && (pptr = &proctab[lproc])->prHoldingLock[lockIdToUse]!= WRITE){
					pid = getlast(lptr-> lockTail);
					ready(pid, RESCHED_NO);
					lptr->processList[pid] = 1;				
				}			
			}
		
			else if(readerFlag == 0){
				if(isempty(lptr-> lockHead)){
					lptr-> lockFlag = 0;				
				}
				else{
					toNext(lockIdToUse);				
				}			
			}		
		}
		
		// if the else condtion is hit, that means that the lock is free 
		// to be acquired
		else{
			if(isempty(lptr-> lockHead)){
				lptr-> lockFlag = 0;
			}
			else{
				//kprintf("\r LOCK IS FREE, GET IT QUICK \n");
				   toNext(lockIdToUse);		
			}		
		}
		modifyThePrio(lockIdToUse);
		increasePriority(lockIdToUse);
		
		i = i+1;

	}
	changeOnRelease(currpid);
	resched();
	restore(mask4);
	//return (lockReturn);
	return OK;
}


int freeAllLocks(int pid){
	int i;
	int nLFlag = 0;
	struct lentry *lptr;
		
	for(i = 0; i<NLOCKS; i++){
		if(locktab[i].processList[pid] == 1){
			lptr = &locktab[i];
			locktab[i].processList[pid] = 0;
			if(isempty(lptr->lockHead)){
				lptr->lockFlag = 0;			
			}
			else{
				toNext(i);			
			}
			nLFlag = 1;		
		}		
	}
	return nLFlag;
}


void toNext(int lockId){
	struct lentry  *lptr;
	struct procent *pptr;

	lptr = &locktab[lockId];


	// choose a process of higher priority (reader or writer);
	int tail = lptr->lockTail;
	int pid;
	int lproc;
	
	lproc = queuetab[tail].qprev;
	
	if((pptr = &proctab[lproc])-> prHoldingLock[lockId] == READ){
		int prio = queuetab[lproc].qkey;
		tail = 	queuetab[lproc].qprev;

		if(queuetab[lproc].qprev == lptr->lockHead){
		
			pid = getlast(lptr->lockTail);
			pptr-> processLockId = -1;
			
			ready(pid,RESCHED_NO);
			lptr-> processList[pid] = 1; 		
		}
		
		else{	
			tail = lptr->lockTail;
			int nFlag = 0;
			
			while( (lproc = queuetab[tail].qprev) < NPROC && lproc != lptr->lockHead){
				if(prio == queuetab[queuetab[tail].qprev].qkey && nFlag == 0){
					if(proctab[lproc].prHoldingLock[lockId] == WRITE){
						pid = dequeue(lproc);
						pptr-> processLockId = -1;

						ready(pid, RESCHED_NO);
						lptr-> processList[pid] = 1;
						break;					
					}
					else{
						tail = queuetab[tail].qprev;
						if( queuetab[tail].qprev == lptr->lockHead){
							nFlag = 1;
							tail = lptr->lockTail;
							continue;						
						}
						continue;					
					}				
				}
				else{
					// deaqueue all readers with prio > first

					int tail = lptr->lockTail;
					int lproc;
					int pid;
					while ( (lproc = queuetab[tail].qprev) < NPROC && lproc != lptr-> lockHead && (pptr = &proctab[lproc])->prHoldingLock[lockId] != WRITE){
						pid = getlast(lptr-> lockTail);
						pptr->processLockId = -1;
						ready(pid, RESCHED_NO);
						lptr->processList[pid] = 1;					
					}
					break;				
				}			
			} 
		}
	}
	else{

		// first process in queue will be a writer process
		pid = getlast(lptr-> lockHead);
		//kprintf("\r PID in release all is : %d\n",pid);
		pptr-> processLockId = -1;
		ready(pid, RESCHED_NO);
		lptr-> processList[pid] = 1;
	}
}


void modifyThePrio(int lockId){
	struct lentry *lptr;
	lptr = &locktab[lockId];

	int prio = 0;
	int tail = lptr-> lockHead;
	int lproc;

	while( (lproc = queuetab[tail].qnext) < NPROC && lproc != lptr->lockTail){
		if(proctab[lproc].prprio > prio){
			prio = proctab[lproc].prprio;		
		}
		tail = queuetab[tail].qnext;	
	}
	lptr->lockPrio = prio;
}



void changeOnRelease(int pid){
	int i;
	int prio = 0;
	
	for(i =0; i<NLOCKS; i++){
		if(locktab[i].processList[pid] == 1){
			if(locktab[i].lockPrio > prio){
				prio = locktab[i].lockPrio;
			}		
		}	
	}
	if(proctab[pid].prprio < prio){
		proctab[pid].prprio = prio;	
	}
}

/*	
	for(;i<numlocks; i++){
		nextLockDesp = *(&arguments + i);
		key = getDesiredLock(nextLockDesp);
	
		if(isbadlock(key) || locktab[key].lockState == LFREE || locktab[key].processList[currpid] != 1){
			return SYSERR;
		}
		else{
			relaseTheLock(currpid, key, 0);		
		}	
	}
	resched();
	restore(mask);
	return OK;
}


void relaseTheLock(int pid, int lockKey, int whereFrom){
	struct lentry  *lptr;
	struct procent *pptr;
	int nextPidInLine;
	int i;
	int ltemp1;
	int ltemp2;
	
	int nWriter = 0;
	int nReader = 0;
	int taskWriter = 0;
	int currentPriority = 0;
	unsigned long difference = 0;

	pptr = &proctab[pid];
	lptr = &locktab[lockKey];

	if(whereFrom == 1){
		proctab[pid].prprio = proctab[pid].mainPrio;
		updateOnRelease(pid);
		
		lptr-> lockFlag 			= 0;
		lptr-> processList[pid]  	= -1;
		lptr-> lockType				= DELETED;

		pptr-> prHoldingLock[lockKey]= DELETED;
		pptr-> waitingPrio = -1;
		pptr-> lockId = -1;

		currentPriority = pptr-> prprio;
		
		proctab[pid].prprio = proctab[pid].mainPrio;
		updateOnRelease(pid);	
	}
	
	else if(whereFrom == 0){
		lptr-> lockFlag = 0;
		lptr-> processList[pid] = -1;
		lptr-> lockType = DELETED;
		
		pptr-> prHoldingLock[lockKey]	= DELETED;
		pptr-> waitingPrio				= -1;
		pptr-> lockId					= -1;

		currentPriority = pptr-> prprio;
		
		proctab[pid].prprio = proctab[pid].mainPrio;
		updateOnRelease(pid);
		
		if(!isempty(lptr-> lockHead)){
			if(proctab[queuetab[lptr-> lockTail].qprev].prHoldingLock[lockKey] == WRITE){
				for(i =0; i<NPROC; i++){
					if( (proctab[i].prHoldingLock[lockKey] == READ)	&& (proctab[i].waitingPrio >= proctab[queuetab[lptr-> lockTail].qprev].waitingPrio) && locktab[lockKey].processList[i] == 1){
							nReader = 1;
							break;				
					}			
				}
				if(nReader != 1){
					nextPidInLine = getlast(lptr-> lockTail);
					lptr-> lockState = LUSED;
					lptr-> processList[nextPidInLine] = 1;
					lptr-> lockType = WRITE;
					
					proctab[nextPidInLine].totalWaitingTime = clkticks - proctab[nextPidInLine].totalWaitingTime;
					proctab[nextPidInLine].lockId = -1;
					ready(nextPidInLine, RESCHED_NO);				
				}					
			}
		 	else if( proctab[queuetab[lptr->lockTail].qprev].prHoldingLock[lockKey] == READ){
				ltemp1 = lptr-> lockTail;
				while( (queuetab[ltemp1].qprev != lptr-> lockHead) ){
					if(proctab[queuetab[ltemp1].qprev].prHoldingLock[lockKey] == WRITE){
						nWriter = 1;
						taskWriter = queuetab[ltemp1].qprev;
						break;					
					}
					ltemp1 = queuetab[ltemp1].qprev;				
				}
				if(nWriter == 1){
						if( proctab[queuetab[lptr-> lockTail].qprev].waitingPrio == proctab[taskWriter].waitingPrio){
							difference = (proctab[taskWriter].totalWaitingTime - proctab[queuetab[lptr-> lockTail].qprev].totalWaitingTime);
							if(difference < 0){
									difference = -1 * difference;						
								}
							if( difference < 1000){
									nextPidInLine = dequeue(taskWriter);
									lptr-> lockState = LUSED;
									lptr-> lockFlag  = 1;
									lptr-> processList[nextPidInLine] = 1;
									lptr-> lockType = WRITE;
									proctab[nextPidInLine].totalWaitingTime = clkticks - proctab[nextPidInLine].totalWaitingTime;
							
									proctab[nextPidInLine].lockId = -1;
									ready(nextPidInLine, RESCHED_NO);						
								}
							else{
									ltemp2 = lptr-> lockTail;
									while( queuetab[lptr-> lockTail].qprev != taskWriter){
										nextPidInLine = getlast(lptr-> lockTail);
										lptr-> lockState = LUSED;
										lptr-> lockFlag  = 1;
										lptr-> processList[nextPidInLine] = 1;
										lptr-> lockType = READ;
										proctab[nextPidInLine].totalWaitingTime = clkticks - proctab[nextPidInLine].totalWaitingTime;
										proctab[nextPidInLine].lockId = -1;
										ready(nextPidInLine, RESCHED_NO);							
									}
								}					
							}
					else{
							ltemp2 = lptr-> lockTail;
							while( queuetab[lptr-> lockTail].qprev != taskWriter){
								nextPidInLine = getlast(lptr-> lockTail);
								lptr-> lockState = LUSED;
								lptr-> lockFlag = 1;
								lptr-> processList[nextPidInLine] = 1;
								lptr-> lockType = READ;
								proctab[nextPidInLine].totalWaitingTime = clkticks - proctab[nextPidInLine].totalWaitingTime;
								proctab[nextPidInLine].lockId = -1;
								ready(nextPidInLine, RESCHED_NO);						
							}
						}				
					}
				else if(nWriter != 1){
					ltemp1 = lptr-> lockTail;
					while( queuetab[ltemp1].qprev != lptr-> lockHead){
						nextPidInLine = dequeue(queuetab[ltemp1].qprev);
						lptr-> lockState = LUSED;
						lptr-> lockFlag = 1;
						lptr-> processList[nextPidInLine] = 1;
						lptr-> lockType = READ;
						proctab[nextPidInLine].totalWaitingTime = clkticks - proctab[nextPidInLine].totalWaitingTime;
						proctab[nextPidInLine].lockId = -1;
						ready(nextPidInLine, RESCHED_NO);					
					}
									
				}
						
			}	
		}				
	}
}




void updateOnRelease(int pid){
	int i;
	int temp;
	int maximumPrio;
	
	i = getDesiredLock(proctab[pid].lockId);

	if(i != SYSERR){
		temp = locktab[i].lockTail;
		while(queuetab[temp].qprev != locktab[i].lockHead){
			if(maximumPrio < proctab[queuetab[temp].qprev].prprio){
				maximumPrio = proctab[queuetab[temp].qprev].prprio;			
			}
			temp = queuetab[temp].qprev;		
		}
		increasePriority(i, maximumPrio);
	}	
}








*/











































