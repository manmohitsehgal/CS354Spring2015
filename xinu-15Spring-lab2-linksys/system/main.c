#include <xinu.h>
/* Lock variables */
int lck;
#ifndef NLOCK
#define NLOCKS 50
#define NLOCK NLOCKS
#endif
#ifndef READ 
#define READ 0
#define READL READ
#endif
#ifndef WRITE 
#define WRITE 1
#define WRITEL WRITE
#endif

int lcks[NLOCK];

void reader1( int , int,int );
void writer1( int, int,int );
void loop(int);

void test0(void);
void test1(void);
void test2(void);
void test3(void);

void test9(void);

int main(void) {

	kprintf("15 spring CS354 Lab \n\r");

	kprintf("Running test 0\n\r");
	test0();
	kprintf("Running test 1\n\r");
	test1();
	kprintf("Running test 2\n\r");
	test2();
	kprintf("Running test 3\n\r");
	test3();
	kprintf("Running test 9\n\r");
	test9();
	return 0;
}

void reader1( int lck,int num, int prio ) {
    int a;
    a = lock( lck, READ, prio );
    if( a != OK )
    {
        kprintf(" Reader%d: lock failed %d ..\n\r", num, a );
        return;
    }
    kprintf(" Reader%d: Lock ..\n\r", num );
    sleep(3);
    kprintf(" Reader%d: Releasing ..\n\r", num );
    
    a = releaseall( 1,lck );
    if( a != OK )
        kprintf(" Reader%d: Lock release failed %d ..\n\r", num, a );
    
}

void writer1( int lck ,int num, int prio ) {
    int a;
	//kprintf("\r starting writer %d\n",num);
    a = lock( lck, WRITE,prio );
	//kprintf("\r acquired lock%d with num%d\n",lck,num);
    if( a != OK ) {
        kprintf(" Writer%d: lock failed %d ..\n\r", num, a);
        return;
    }
    
    kprintf(" Writer%d: Lock ..\n\r", num );
    sleep(3);
    kprintf(" Writer%d: Releasing ..\n\r", num );
    //kprintf("num%d, lck%d, prio%d\n", num, lck, prio);
    a = releaseall(1, lck);
    if( a != OK )
        kprintf(" Writer%d: Lock release failed %d ..\n\r", num, a);
	//else
		//kprintf("\r release the lock %d\n",lck);
}

void loop(int timetoloop) {
    //kprintf("pid %d starting loop \n\r", getpid());
    unsigned int start_time =  clktime;
    while((clktime - start_time) <= timetoloop);
    //kprintf("pid %d exiting from loop \n\r",getpid());
}

/*Test0	- Regression testing
 *Expected Output:
 *	TEST0 
 *	TEST0	DONE
 */

void test0() {
	int i, j, a;
	for( j = 0; j < 4; j++ ) {
		for( i = 0; i < NLOCK; i++ ) {
			lcks[i] = lcreate();	
			if( lcks[i] == SYSERR ) {
				kprintf(" TEST0: lcreate failed ..DONE\n\r");
				return;
			}
		}
		for( i = 0; i < NLOCK; i++ ) {
			a = lock( lcks[i], READ, 0 );
			if( a == SYSERR ) {
				kprintf(" TEST0: lock failed ..DONE\n\r");
				return;
			}
		}
		for( i = 0; i < NLOCK; i++ ) {
			a = releaseall( 1,lcks[i]);
			if( a == SYSERR ) {
				kprintf(" TEST0: release failed ..DONE\n\r");
				return;
			}
		}
		for( i = 0; i < NLOCK; i++ ) {
			a = ldelete( lcks[i] );
			if( a == SYSERR ) {
				kprintf(" TEST0: release failed ..DONE\n\r");
				return;
			}
		}
	}
	kprintf(" TEST0	DONE \n\r");
}

/* Test 1 - Test for basic lcreate, lock, release ( read lock )
 *
 * Expected Output : 
 *	TEST1
 * 
 *
 *	Reader1: lock .. 
 *	Reader2: lock .. 
 *	Reader3: lock .. 
 *
 *	Reader1: Releasing .. 
 *	Reader2: Releasing .. 
 *	Reader3: Releasing .. 
 * 	( The order doesnt matter . The last 3  
 *	  statements will be printed after approx 3 seconds )
 *	TEST1: Completed
 *	       
 */

void test1() {
	int  a;
	pid32 lok;
	lck = lcreate();
	if( lck == SYSERR ) {
	  kprintf(" TEST1: error in lcreate()..DONE\n\r");
	  return;
	}

	 lok=create( reader1, 2000, 30, "reader", 3, lck,1, 0 ); 	
	resume(lok);
	resume( create( reader1, 2000, 30, "reader", 3, lck,2, 0 ));
	resume( create( reader1, 2000, 30, "reader", 3, lck,3, 0 ));

	
	sleep( 15 );
	a = ldelete( lck );
	if( a != OK )
	{
		kprintf(" TEST1: error in ldelete()..DONE\n\r");
		return;
	}

	kprintf(" TEST1: DONE \n\r");
}

/* Test2 - Test for basic lcreate, lock, release ( for a write lock )
 *
 * Expected output:
 *	Writer1: Lock ..
 *	Writer1: Releasing  ..
 *     	(after 3 seconds )
 *	Writer2: Lock ..
 *	Writer2: Releasing ..
 *     	(after 3 seconds )
 *	Writer3: Lock ..
 *	Writer3: Releasing ..
 *     The order is not important. What is important is that the lock/release 
 * for a process should not be interspersed with the lock/release for another process
 */

void test2( ) {
	int a; 

	kprintf("	TEST2	\n\r\n\r");
	lck = lcreate();

	if( lck == SYSERR )
	{
	  kprintf(" TEST2: error in lcreate()..DONE\n\r");
	  return;
    }
	
	resume( create( writer1, 2000, 30, "writer", 3, lck, 1, 0 ));
	resume( create( writer1, 2000, 30, "writer", 3, lck, 2, 0 ));
    resume( create( writer1, 2000, 30, "newWriter", 3, lck, 3, 0 ));
	/* Wait for the writers to complete and then delete the lock */
	
	//kprintf("\r now sleeping lock %d test 2 \n", lck);
    sleep(20);
	//kprintf("\r ok till here \n");
	kprintf("\r a%d\n", a);
	a = ldelete( lck );
	//kprintf("\r passed this step\n");
	if( a != OK )
        kprintf(" TEST2: error in ldelete()..\n\r");
    kprintf("	TEST2	DONE\n\r");
}

/* Test3: Testing for SYSERRs in case of erroneous inputs
 * Expected output - SYSERRs in all cases 
 */

void test3() {
	/* Locking without creating */
	
	lck = 2;
	kprintf(" lock(2,READ) without lcreate() : %d \n\r", lock( lck, READ,0 ) ); 

	/* Locking with invalid id */

	lck = -2;
	kprintf(" lock(-2,READ)  : %d \n\r", lock( lck, READ,0 ) ); 


	/* Locking with a invalid mode */

	lck = lcreate();
	if( lck == SYSERR ) 
		kprintf(" TEST3: lcreate() failed .. DONE\n\r");

	kprintf(" lock(lck,INVALID) : %d \n\r", lock( lck, -9,0 ) ); 
	ldelete(lck);

	/* Deleting without creating  */

	lck = 10;
	kprintf(" ldelete(lck) without lcreate() : %d \n\r", ldelete( lck ) ); 


	/* Deleting with an invalid ID */

	lck = -1;
	kprintf(" ldelete(-1)  : %d \n\r", ldelete( lck ) ); 
}

/* Test 9
 * Testing priority inheritance on a single lock acquired by multiple processes  * Expected output: 
 * Reader1: Lock ..
 * Reader2: Lock..
 * Reader3: Lock..
 * pid of medium priority process = x
 * Reader1: Releasing
 * Reader2 : Releasing
 * Reader3: Releasing
 * Writer1 :Lock
 * Writer 1: Releasing
 * pid x exiting from loop
 * (The order of the readers acquiring / releasing the locks does not matter )
*/

void test9() {
  int lk = lcreate();
  int mpid;

  resume( create( reader1, 2000, 30, "reader", 3,lk, 1, 0 ));
  resume( create( reader1, 2000, 30, "reader", 3,lk, 2, 1 ));
  resume( create( reader1, 2000, 30, "reader", 3,lk, 3, 2 ));
  /* The 3 readers have acquired the lock */
  chprio(getpid(),60);
  resume(mpid = create(loop,2000,40,"loop",1,10));
  kprintf("pid of medium priority process = %d\n\r",mpid);
  resume(create(writer1,2000,50,"writer",3,lk,1,0));
  chprio(getpid(),20);
  yield();
  if(ldelete(lk)!=OK) 
    kprintf("Test9 : Error while deleting lock\n\r");
  kprintf("Test 9 done..\n\r");
}
