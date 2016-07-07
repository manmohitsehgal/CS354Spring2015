#include <xinu.h>


int generateNewLock(void);

int lcreate(void){

	intmask mask;
	int lock;
	mask = disable();
	
	if ((lock=generateNewLock())==SYSERR ) {
		restore(mask);
		return(SYSERR);

	}	

	restore(mask);
	return locktab[lock].lockDesp;
}
	


/*  Goes through the locks to check if any of the lock is in LFREE
	State. If the lock in in LFREE, it changes the state to a used state */


int generateNewLock(void){

	int i;
	int nextLock;

	for(i = 0; i <NLOCKS; i++){
		nextLock = nextInLine -- ;
		if(nextInLine < 0){
			nextInLine = NLOCKS - 1;		
		}
		// Check the state of the lock
		if(locktab[nextLock].lockState == LFREE){
			locktab[nextLock].lockState = LUSED;
	
			for(i = 0; i<NLOCKS; i++){
				locktab[nextLock].processList[i] = 0;
			}
			
			if(locktab[nextLock].lockType == 0){
				locktab[nextLock].lockDesp = nextLock;			
			}
			return (nextLock);
		}
	}
	return(SYSERR);
}

