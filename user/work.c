#include <core/eos.h>

static eos_tcb_t tcb1;
static eos_tcb_t tcb2;
static eos_tcb_t tcb3;
static int8u_t stack1[8096];
static int8u_t stack2[8096];
static int8u_t stack3[8096];
static int8u_t queue1[10];
static int8u_t queue2[10];
eos_mqueue_t mq1;
eos_mqueue_t mq2;

static void sender_task(void *arg) {
	int8u_t *data = "xy";
	while (1) {
		PRINT("send message to mq1\n");
		eos_send_message(&mq1, data, 0) ;
		PRINT("send message to mq2\n");
		eos_send_message(&mq2, data, 0) ;
		eos_sleep(0);
	}
}

static void receiver_task1(void *arg) {
	int8u_t data[2];
	while (1) {
		PRINT("receive message from mq1\n");
		eos_receive_message(&mq1, data, 0);
		PRINT("received message: %s\n", data);
		eos_sleep(0);
	}
}

static void receiver_task2(void *arg) {
	int8u_t data[2];
	while (1) {
		PRINT("receive message from mq2\n");
		eos_receive_message(&mq2, data, 0);
		PRINT("received message: %s\n", data);
		eos_sleep(0);
	}
}

void eos_user_main() {
	eos_create_task(&tcb1, (addr_t)stack1, 8096, sender_task, NULL, 50);
	eos_create_task(&tcb2, (addr_t)stack2, 8096, receiver_task1, NULL, 10);
	eos_create_task(&tcb3, (addr_t)stack3, 8096, receiver_task2, NULL, 10);
	eos_set_period(&tcb1, 2);
	eos_set_period(&tcb2, 4);
	eos_set_period(&tcb3, 5);
	eos_init_mqueue(&mq1, queue1, 5, 2, FIFO);
	eos_init_mqueue(&mq2, queue2, 5, 2, FIFO);
}

/*
#include <core/eos.h>

static eos_tcb_t tcb1;
static eos_tcb_t tcb2;
static eos_tcb_t tcb3;
static eos_tcb_t tcb4;

static int8u_t stack1[8096];
static int8u_t stack2[8096];
static int8u_t stack3[8096];
static int8u_t stack4[8096];


static void task1() {
	while(1){
		PRINT("A\n");
		eos_sleep(0);
	}
}

static void task2() {
	while(1){
		PRINT("B\n");
		eos_sleep(0);
	}
}

static void task3() {
	while(1){
		PRINT("C\n");
		eos_sleep(0);
	}
}

static void task4() {
	while(1){
		PRINT("D\n");
		eos_sleep(0);
	}
}
void eos_user_main() {
	eos_create_task(&tcb1, (addr_t)stack1, 8096, task1, NULL, 1);
	eos_set_period(&tcb1, 2);
	eos_create_task(&tcb2, (addr_t)stack2, 8096, task2, NULL, 10);
	eos_set_period(&tcb2, 4);
	eos_create_task(&tcb3, (addr_t)stack3, 8096, task3, NULL, 50);
	eos_set_period(&tcb3, 8);

	eos_create_task(&tcb4, (addr_t)stack4, 8096, task4, NULL, 1);
	eos_set_period(&tcb4, 3);
	//PRINT("PRINT_NUMBER %x\n", print_number);
	//PRINT("PRINT_ALPHAB %x\n", print_alphabet);
	
	//stack_top = (addr_t)stack1 + 8096;
}

*/
