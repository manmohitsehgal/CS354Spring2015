/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

/************************************************************************/
/*									*/
/* main - main program for testing Xinu					*/
/*									*/
/************************************************************************/

int main(int argc, char **argv)
{
	kprintf("LAB 3 RESULTS!\n");
	
	resume(create(iointensive,1024,20,"cp-i-1",0));
	resume(create(iointensive,1024,20,"cp-i-2",0));
	resume(create(iointensive,1024,20,"io-i-3",0));
	resume(create(iointensive,1024,20,"io-i-3",0));
	resume(create(iointensive,1024,20,"cp-i-5",0));
	resume(create(iointensive,1024,20,"cp-i-6",0));
	




	//kprintf("passed line 21\n");
	

	

	//kprintf("passed line 29\n");
	
	return OK;
}
