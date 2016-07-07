/*  main.c  - main */
//this test case is revised from weichu's


#include <xinu.h>
#include <stdio.h>

// blocking message send test
#define PRI 30

static int score = 8;       //basic

sid32 sema;
static int test0_success = 0;
static int test0_sender_returned = 0;

static void receiver0(void){
  sleep(1);
  if (proctab[currpid+1].prstate == PR_SND) {
        score+=6;
        kprintf("Send State Test: Pass\r\n");
    } else {
        kprintf("Send State Test: fail: didn't define PR_SND properly\r\n");
    }
  receiveb();
  kprintf("Test0: call receive()\r\n");
  if( test0_sender_returned ){
    test0_success = 1;
  }else{
    kprintf("Test0: fail: timeout, the first sendb() should return immediately.\r\n");
  }
  uint32 msg = receiveb();
  kprintf("Test0: receive msg: %d\r\n", msg);
  if( test0_success ){
    score+= 6;
    kprintf("Test0: Pass\r\n");
  }
}
static void sender0(int pid){
  sendb( pid, 1 ); //shouldn't block
  test0_sender_returned = 1;
  sendb( pid, 2 ); //should block
  kprintf("Test0: sender returned\r\n");
  //signal( sema0 );
}

#define TEST1_MESSAGE 5566
static void _receiver1(void){
  uint32 msg = receiveb();
  if( msg == TEST1_MESSAGE ){
    kprintf("Test1: Pass\r\n");
    score+=6;
  }else{
    kprintf("Test1: fail: received incorrect value from the sender. received %d, expect %d\r\n", msg, TEST1_MESSAGE);
  }
  signal( sema );
}
static void _sender1(int pid){
  sendb( pid, TEST1_MESSAGE );
}

#define TEST2_OFFSET 1024
static void _receiver2(void){
  int n;
  for(n = 0; n< 100; n++ ){
    uint32 msg = receiveb();
    if( msg != n+TEST2_OFFSET ){
      kprintf("Test2: fail: did not receive in sending order at %d th message.\r\n", n);
      signal( sema );
      return;
    }
  }
  kprintf("Test2: pass\r\n");
  score+=6;
  signal( sema );
}
static void _sender2(int pid){
  int n;
  for(n = 0; n< 100; n++ ){
    sendb( pid, n+TEST2_OFFSET );
  }
}

static int test3_result = 0;
static void _receiver3(void){
  int n;
  for(n = 0; n< 5; n++ ){
    uint32 msg = 0;
	msg=receiveb();
    test3_result++;
  }
}
static void _sender3(int pid){
  sendb( pid, 1 );
}

#define TEST4_OFFSET 765
static void receiver4(void){
  int n;
  sleep(6); // sleep several seconds to force all senders to wait
  for(n = 0; n< 5; n++ ){
    uint32 msg = receiveb();
    if( msg != n+TEST4_OFFSET ){
      kprintf("Test4: fail: did not receive in sending order at %d th sender\r\n", n);
      signal( sema );
      return;
    }
  }
  kprintf("Test4: pass\r\n");
  score+=6;
  signal( sema );
}
static void sender4(int pid, int n){
  sendb( pid, n+TEST4_OFFSET );
}

#define TEST5_SENDER 5
static int msgarr[ TEST5_SENDER*100 ] = {0};
static int test5_received_messages = 0;
static int test5_fail = 0;
static int test5_success = 0;
static void receiver5(void){
  int n;
  for(n = 0; n< TEST5_SENDER*100; n++ ){
    uint32 msg = receiveb();
    //kprintf("received %d th message\r\n", n );
    if( msgarr[ msg ] == 1 || msg >= TEST5_SENDER*100 ){
      kprintf("Test5: fail: receive duplicate messages or unsent messages. msg=%d\r\n", msg);
      test5_fail = 1;
      test5_success = 0;
      return;
    }
    msgarr[ msg ] = 1;
    test5_received_messages++;
  }
  kprintf("Test5: pass\r\n");
  score+=6;

  test5_fail = 0;
  test5_success = 1;
}
static void sender5(int pid, int mysenderid){
  int n;
  for(n = 0; n < 100; n++ ){
    sendb( pid, n+ (mysenderid*100) );
  }
}

#define TEST6_OFFSET 47906
int test6_result[2] = {0};
static void receiver6(int receiver_id){
  uint32 msg = 0;
  msg=receiveb();
  if( msg < 0+TEST6_OFFSET || msg >= 2+TEST6_OFFSET ){
    intmask	mask;
    mask = disable();
    kprintf("Test6: fail: incorrect msg received. expect %d <= msg < %d. received %d instead\r\n", TEST6_OFFSET, TEST6_OFFSET+2, msg);
    test6_result[ receiver_id ] = 0;
		restore(mask);
    return;
  }
  
  kprintf("Test6: receiver process %d finished\r\n", currpid );
  test6_result[ receiver_id ] = 1;
}
static void sender6(int pid, int mysenderid){
  sendb( pid, mysenderid+TEST6_OFFSET );
}

int test7_result[2] = {0};
static void receiver7(int receiver_id){
  int n;
  int msgs[2] = {0};
  intmask	mask;
  for(n=0;n<2;n++){
    uint32 msg = receiveb();
    if( msg < 0 || msg >= 4 ){
      mask = disable();
      kprintf("Test7: fail: incorrect msg received. expect 0<=msg<4 received %d instead\r\n", msg);
      restore(mask);
      test7_result[ receiver_id ] = 0;
      signal(sema );
      return;
    }
    if( msgs[msg] == 1 ){
      mask = disable();
      kprintf("Test7: fail: received duplicated message\r\n");
      restore(mask);
      test7_result[ receiver_id ] = 0;
      signal(sema );
      return;
    }
    msgs[msg] = 1;
  }
  test7_result[ receiver_id ] = 1;
  kprintf("Test7: receiver process %d finished\r\n", currpid );
  signal( sema  );
}
static void sender7(int pid, int mysenderid){
  sendb( pid, mysenderid/2 );
}

static int test8_received_message[2] = {0};
static void receiver8(int myid){
  int n;
  for(n=0;n<2*100;n++){
    uint32 msg = 0;
	msg=receiveb();
    test8_received_message[ myid ]++;
  }
  kprintf("Test8: receiver process %d finished\r\n", currpid );
  //signal( sema );
}
static void sender8(int pid, int mysenderid){
  int n;
  for(n=0;n<100;n++){
    sendb( pid, mysenderid );
  }
}

#define TEST9_OFFSET 47906
static int test9_received_message[2] = {0};
static int test9_result = 1;
static void receiver9(int myid){
  int test9_msgs[200] = {0};
  int n;
  intmask	mask;
  for(n=0;n<2*100;n++){
    uint32 msg = receiveb();
    if( msg < 0+TEST9_OFFSET || msg >= 200+TEST9_OFFSET ){
      mask = disable();
      kprintf("Test9: fail: incorrect msg received. expect %d <= msg < %d. received %d instead\r\n", TEST9_OFFSET, TEST9_OFFSET+200, msg);
      restore(mask);
      test9_result = 0;
      return;
    }
    if( test9_msgs[msg-TEST9_OFFSET] == 1 ){
      mask = disable();
      kprintf("Test9: fail: received duplicated message\r\n");
      restore(mask);
      test9_result = 0;
      return;
    }
    test9_msgs[msg-TEST9_OFFSET] = 1;
    test9_received_message[ myid ]++;
  }
  kprintf("Test9: receiver process %d finished\r\n", currpid );
}
static void sender9(int pid, int mysenderid){
  int n;
  for(n=0;n<100;n++){
    sendb( pid, mysenderid*100+n+TEST9_OFFSET );
  }
}

// grading criteria: 
// programming 100 points
//
// (1) every sent messages must arrive
// (2) receiving order conforms sending order
//
// basic: 8 pts     successfully compile
// sendb/receiveb: 66 pts, 6 pts for each test case
// senda: 26 = 6 + 10 + 10
//
void test0(void){
  kprintf("Test 0 -- One sender one receiver. the sender should return immediately. 10 points\r\n");

  //sema0 = semcreate( 0 );
  int pid = create(receiver0, 1024, PRI, "receiver0", 0);
  resume(pid);
  int senderpid = create(sender0, 1024, PRI, "sender0", 1, pid);
  resume(senderpid);
  sleep(3);
  if( kill( pid ) == OK ){
    kprintf("test 0 failed: receiver did not return\r\n");
  }
  kill( senderpid );
  kprintf("Test0 finished\r\n");
  sleep(1);
}
void test1(void){
  kprintf("Test 1 -- One sender one receiver. one message per sender. verify receiver gets the right message. 10 points\r\n");
  int pid = create(_receiver1, 1024, PRI, "receiver1", 0);
  resume(pid);
  
  resume(create(_sender1, 1024, PRI, "sender1", 1, pid));

  wait( sema );
  kprintf("Test 1 finished\r\n");
  sleep(1);
}
void test2(void){
  kprintf("Test 2 -- One sender one receiver. multiple messages per sender. 10 points\r\n");
  int pid = create(_receiver2, 1024, PRI, "receiver2", 0);
  resume(pid);
  
  resume(create(_sender2, 1024, PRI, "sender2", 1, pid));

  wait( sema );
  kprintf("Test 2 finished\r\n");
  sleep(1);
}
void test3(void){
  kprintf("Test 3 -- Multi-sender one receiver. one message per sender. 10 points \r\n");
  int pid = create(_receiver3, 1024, PRI, "receiver3", 0);
  resume(pid);
  
  int n;
  for( n= 0; n< 5;n++){
    resume(create(_sender3, 1024, PRI, "sender3", 1, pid));
  }

  sleep(3);
  if( test3_result == 5 ){
    kprintf("Test3: pass\r\n");
    score+=6;
  }else{
    kprintf("Test 3: fail: timeout waiting for messages\r\n");
  }
  kprintf("Test 3 finished\r\n");
  sleep(1);
}
void test4(void){
  kprintf("Test 4 -- Multi-sender one receiver. one message per sender . is receiving order the same as sending order? 10 points \r\n");
  int pid = create(receiver4, 1024, PRI, "receiver4", 0);
  resume(pid);
  
  int n;
  int senderpid[5];
  for( n= 0; n< 5;n++){
    senderpid[ n ] = create(sender4, 1024, PRI, "sender4", 2, pid, n);
    resume(senderpid[ n ] );
    sleep(1); // sleep 1 second to let the previous sender block sending.
  }
  wait( sema );
  sleep(1); // sleep 1 second to let all processes finish
  for(n=0;n<5;n++){
    kill( senderpid[n] );
  }
  kprintf("Test 4 finished\r\n");
}
void test5(void){
  kprintf("Test 5 -- Multi-sender one-receiver. multiple messages per sender. 10 points\r\n");
  int n;
  int pid = create(receiver5, 1024, PRI, "receiver5", 0);
  resume(pid);
  
  for( n= 0; n< TEST5_SENDER;n++){
    resume(create(sender5, 1024, PRI, "sender5", 2, pid, n));
  }
  int last_received_message = 0;
  for( n=0;n< 50; n++){ // wait for 50 seconds, if at any point receiver does not make progress, abort
    sleep(1);
    if( test5_success ){ break; }
    if( test5_fail ){ break; }
    if( test5_received_messages == last_received_message ){
      kprintf("Test 5 fail: receiver does not receive any more messages after %d th message\r\n", test5_received_messages );
      kill( pid ); // kill receiver
      break;
    }
    last_received_message = test5_received_messages;
  }
  kprintf("Test 5 finished\r\n");
  sleep(1);
}
void test6(void){
  kprintf("Test 6 -- Multi-sender multi-receiver. one sender sends to one receiver. one message per sender - are all messages received? 10 points\r\n");
  int n;
  int receiverpids[ 2 ];
  for( n= 0; n< 2;n++){
    int pid = create(receiver6, 1024, PRI, "receiver6", 1, n);
    receiverpids[ n ] = pid;
    resume(pid);
  }
  
  for( n= 0; n< 2;n++){
    resume(create(sender6, 1024, PRI, "sender6", 2, receiverpids[ n ], n));
  }
  sleep(2);
  if( test6_result[ 0 ] && test6_result[ 1 ]){
    score+= 6;
    kprintf("Test 6 pass\r\n");
  }
  kprintf("Test 6 finished\r\n");
  sleep(1);
}
void test7(void){
  kprintf("Test 7 -- Multi-sender multi-receiver. two senders per receiver. one message per sender - are all messages received? 10 points\r\n");
  int n;
  int receiverpids[ 2 ];
  for( n= 0; n< 2;n++){
    int pid = create(receiver7, 1024, PRI, "receiver7", 1, n);
    receiverpids[ n ] = pid;
    resume(pid);
  }
  
  for( n= 0; n< 4;n++){
    resume(create(sender7, 1024, PRI, "sender7", 2, receiverpids[ n%2 ], n));
  }
  // wait for both receivers to finish
  for( n=0; n< 2; n++){
    wait( sema );
  }
  if( test7_result[ 0 ] && test7_result[ 1 ]){
    kprintf("Test 7 pass\r\n");
    score+= 6;
  }
  sleep(1);
  kprintf("Test 7 finished\r\n");
}
void test8(void){
  kprintf("Test 8 -- Multi-sender multi-receiver. two senders per receiver. Multiple messages per sender - make sure messages are received. 10 points\r\n");
  int n;
  int receiverpids[ 2 ];
  for( n= 0; n< 2;n++){
    int pid = create(receiver8, 1024, PRI, "receiver8", 1, n);
    receiverpids[ n ] = pid;
    resume(pid);
  }
  
  for( n= 0; n< 4;n++){
    resume(create(sender8, 1024, PRI, "sender8", 2, receiverpids[ n%2 ], n%2));
  }
  // wait for both receivers to finish
  /*for( n=0; n< 2; n++){
    wait( sema );
  }*/
  int last_received_message0 = 0;
  int last_received_message1 = 0;
  for( n=0;n< 50; n++){ // wait for 50 seconds, if at any point receiver does not make progress, abort
    sleep(1);
    if( test8_received_message[0] == last_received_message0 ){
      kprintf("Test 8 fail: receiver 0 does not receive any more messages after %d th message\r\n", last_received_message0 );
      kill( receiverpids[0] ); // kill receiver
      break;
    }
    if( test8_received_message[1] == last_received_message1 ){
      kprintf("Test 8 fail: receiver 1 does not receive any more messages after %d th message\r\n", last_received_message1 );
      kill( receiverpids[1] ); // kill receiver
      break;
    }
    last_received_message0 = test8_received_message[0];
    last_received_message1 = test8_received_message[1];

    if( test8_received_message[1] == 200 && test8_received_message[0] == 200 ){
      break;
    }
  }
  if( test8_received_message[0] == 200 && test8_received_message[1] == 200 ){
    kprintf("Test 8 pass\r\n");
    score+= 6;
  }
  sleep(1);
  kprintf("Test 8 finished\r\n");
}
void test9(void){
  kprintf("Test 9 -- Multi-sender multi-receiver. two senders per receiver. Multiple messages per sender. check correctness. 10 points\r\n");
  int n;
  // WC: one of the receiver does not receive at all, and the other pair of sender receiver proceeds normally
  int receiverpids[ 2 ];
  for( n= 0; n< 2;n++){
    int pid = create(receiver9, 10240, PRI, "receiver9", 1, n);
    receiverpids[ n ] = pid;
    resume(pid);
  }
  
  int m;
  for( n= 0; n< 2;n++){
    for(m=0; m< 2;m++){
      resume(create(sender9, 1024, PRI, "sender9", 2, receiverpids[ n ], m ));
    }
  }
  int last_received_message0 = 0;
  int last_received_message1 = 0;
  for( n=0;n< 50;n++){ // wait for 50 seconds
    sleep(1);
    if( test9_received_message[0] == last_received_message0 ){
      kprintf("Test 9 fail: receiver 0 does not receive any more messages after %d th message\r\n", last_received_message0 );
      kill( receiverpids[0] ); // kill receiver
      test9_result = 0;
      break;
    }
    if( test9_received_message[1] == last_received_message1 ){
      kprintf("Test 9 fail: receiver 1 does not receive any more messages after %d th message\r\n", last_received_message1 );
      kill( receiverpids[1] ); // kill receiver
      test9_result = 0;
      break;
    }
    if( test9_result == 0 ){
      break;
    }
    if( test9_received_message[1] == 200 && test9_received_message[0] == 200 ){
      break;
    }
    last_received_message0 = test9_received_message[0];
    last_received_message1 = test9_received_message[1];

  }
  if( test9_result ){
    kprintf("Test 9 pass\r\n");
    score+=6;
  }
  kprintf("Test 9 finished\r\n");
  sleep(1);
}

void blocksending_test(void)
{

  sema = semcreate(0);

  kprintf("======================Start Testing=====================\r\n");
  test0();

  test1();

  test2();

  test3();

  test4();
  
  test5();

  test6();

  test7();

  test8();

  test9();

  kprintf("Total Score: %d\r\n", score );
  kprintf("======================End of Test=====================\r\n");
}

// Asynchronous message receive test
umsg32 recvbuf;
int myrecvhandler(void) {
    kprintf("msg received = %d\r\n", recvbuf);
    return(OK);
}
void test_sender01(pid32 pid, umsg32 msg)	{
	senda(pid, msg);
}
void test_receiver(void)	{
	if (registercb(&recvbuf, myrecvhandler) != OK) {
		kprintf("recv handler registration failed\n");
		return ;
	}
	while (1)
		;
}

void test_sender02(pid32 pid, umsg32 msg)	{
	int i;
	for (i=0; i<10; i++)
		senda(pid, msg+i);
}

void areceive_test01(void)	{
	pid32 pid;
	pid=create(test_receiver, 1024, 20, "receive", 0);
	resume(pid);
	resume(create(test_sender01, 1024, 20, "senda", 2, pid, 10));
	sleep(5);
	kill(pid);
}
void areceive_test02(void)	{
	pid32 pid;
	pid=create(test_receiver, 1024, 20, "receive", 0);
	resume(pid);
	resume(create(test_sender02, 1024, 20, "send1", 2, pid, 10));
	sleep(5);
	kill(pid);
}
void areceive_test03(void)	{
	pid32 pid;
	pid=create(test_receiver, 1024, 20, "receive", 0);
	resume(pid);
	resume(create(test_sender02, 1024, 20, "send1", 2, pid, 10));
	resume(create(test_sender02, 1024, 20, "send2", 2, pid, 20));
	sleep(5);
	kill(pid);
}

void areceive_test(void)	{
	kprintf("Test1:\r\n");
	areceive_test01();	//1 send, 1 receiver
	kprintf("Test2:\r\n");
	areceive_test02();	//1 send, multiple messages, 1 receiver
	kprintf("Test3:\r\n");
	areceive_test03();	//2 send, multiple messages, 1 receiver
}

int main(int argc, char **argv)
{

	blocksending_test();
	areceive_test();

	return OK;
}


