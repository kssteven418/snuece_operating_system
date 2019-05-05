/********************************************************
 * Filename: core/timer.c
 *
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: 
 ********************************************************/
#include <core/eos.h>

static eos_counter_t system_timer;

int8u_t eos_init_counter(eos_counter_t *counter, int32u_t init_value) {
	counter->tick = init_value;
	counter->alarm_queue = NULL;
	return 0;
}

void eos_set_alarm(eos_counter_t* counter, eos_alarm_t* alarm, int32u_t timeout, void (*entry)(void *arg), void *arg) {

	_os_remove_node(&(counter->alarm_queue), &(alarm->alarm_queue_node));
	
	if(timeout==0 || entry == NULL) return;

	//initiate alarm 
	alarm->timeout = timeout;
	alarm->handler = entry;
	alarm->arg = arg;
	
	alarm->alarm_queue_node.priority = timeout;
	
	if(timeout > counter->tick)
		_os_add_node_priority(&(counter->alarm_queue), &(alarm->alarm_queue_node));

}

eos_counter_t* eos_get_system_timer() {
	return &system_timer;
}

void eos_trigger_counter(eos_counter_t* counter) {
	
	//increase tick
	counter->tick = counter->tick+1;
	PRINT("tick %d\n", counter->tick);
	
	//check queue
	_os_node_t *n = counter->alarm_queue;
	
	while(true){		
		//queue empty
		if(n==NULL) break;
		
		//if no process is out of time
		if(n->priority != counter->tick) break;
		
		//if process n is out of time, then call call_back function 
		else{
			eos_alarm_t* alarm = ((eos_tcb_t*)(n->ptr_data))->alarm;
			(alarm->handler)(alarm->arg);
			_os_remove_node(&(counter->alarm_queue), n);
			n = counter->alarm_queue;
		}
	}

	//call scheduler
	eos_schedule();
}

/* Timer interrupt handler */
static void timer_interrupt_handler(int8s_t irqnum, void *arg) {
	/* trigger alarms */
	eos_trigger_counter(&system_timer);
}

void _os_init_timer() {
	eos_init_counter(&system_timer, 0);

	/* register timer interrupt handler */
	eos_set_interrupt_handler(IRQ_INTERVAL_TIMER0, timer_interrupt_handler, NULL);
}
