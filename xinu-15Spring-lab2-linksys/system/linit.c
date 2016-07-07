#include <xinu.h>

//struct lentry locktab[NLOCKS];
int nextInLine;

int linit(){

		struct lentry *lptr;
		nextInLine = NLOCKS - 1;

		int i;
		int j;

		// go through all the lock and initialize them
		
		for(i=0; i <NLOCKS; i++){
			lptr = &locktab[i];

			lptr->lockState 	= LFREE;		
			lptr->lockTail		= 1 + (lptr-> lockHead = newqueue());
			lptr->lockType		= 0;					
			lptr->lockDesp 		= 0;
			lptr->lockFlag		= 0;	
			//lptr->waitingProcCount = 0;		
													
			for(j =0; j<NPROC; j++){
				lptr->processList[j] = -1;										
		}				 		
	}
	return OK;
}

	

		

