/*******************************************************
 * Filename: core/task.c
 * 
 * Author: parkjy, RTOSLab. SNU.
 * 
 * Description: task management.
 ********************************************************/
#include <core/eos.h>
#include <stdlib.h>

#define READY		1
#define RUNNING		2
#define WAITING		3

/*
 * Queue (list) of tasks that are ready to run.
 */
static _os_node_t *_os_ready_queue[LOWEST_PRIORITY + 1];

//maybe, data element can be eos_tcb_t address!


/*
 * Pointer to TCB of running task
 */
static eos_tcb_t *_os_current_task;

int32u_t eos_create_task(eos_tcb_t *task, addr_t sblock_start, size_t sblock_size,
		void (*entry)(void *arg), void *arg, int32u_t priority) {
	
	addr_t stack_base = sblock_start + sblock_size; // stack starts from high address
	
	PRINT("task: 0x%x, priority: %d\n", (int32u_t)task, priority);
	
	//Create Context
	addr_t stack_init = _os_create_context(stack_base, sblock_size, entry, arg);
	
	//Initiate TCB non-pointer entry
	task->state = READY;
	task->priority = priority;
	task->sp = stack_init;
	task->period = 0;

	//make node and append at _os_ready_queue
	task->node = (_os_node_t*)malloc(sizeof(_os_node_t));
	task->node->priority = priority;
	task->node->ptr_data = task;
	task->node->previous = task->node->next = NULL;
	
	//make alarm and set TCB alarm pointer
	task->alarm = (eos_alarm_t*)malloc(sizeof(eos_alarm_t));
	task->alarm->alarm_queue_node = *(task->node);

	//scheduler
	_os_add_node_tail(&_os_ready_queue[priority], task->node);
	_os_set_ready(priority);
	PRINT("priority in %d, get_priority : %d\n", priority, _os_get_highest_priority());
}

int32u_t eos_destroy_task(eos_tcb_t *task) {

}

void eos_schedule() {
	_os_node_t *n;

	int index;
	addr_t esp;

	//SELECT NEXT PROCESS
	
	index = _os_get_highest_priority();
	n = _os_ready_queue[index];

	if(n==NULL)	return;
	
    //state change
	//new task : to RUNNING
	((eos_tcb_t*)n->ptr_data)->state = RUNNING;
	_os_remove_node(&_os_ready_queue[index], n);
	
	//if no process in the given priority queue, turn off the ready_bit
	if(_os_ready_queue[index]==NULL){
		_os_unset_ready(index);
	}

	//if the old task is the idle task : to READY
	if(_os_current_task!=NULL && _os_current_task->period == 0){
		_os_current_task->state = READY;
		_os_add_node_tail(&_os_ready_queue[_os_current_task->priority], _os_current_task->node);
		_os_set_ready(_os_current_task->priority);
	}   

	/* CONTEXT SWITCHING */

	if(_os_current_task==NULL){
		_os_current_task = (eos_tcb_t*)(n->ptr_data);
		esp =  ((eos_tcb_t*)(n->ptr_data))->sp;
		_os_restore_context(esp);
	}

	else{
		esp = _os_save_context();
		/* must not touch this area*/
		_os_current_task->sp = esp;
		_os_current_task = (eos_tcb_t*)(n->ptr_data);
		esp =  ((eos_tcb_t*)(n->ptr_data))->sp;
		_os_restore_context(esp);
	}
}

eos_tcb_t *eos_get_current_task() {
	return _os_current_task;
}

void eos_change_priority(eos_tcb_t *task, int32u_t priority) {
	task->priority = priority;
}

int32u_t eos_get_priority(eos_tcb_t *task) {
	return task->priority;
}

void eos_set_period(eos_tcb_t *task, int32u_t period){
	task->period = period;
}

int32u_t eos_get_period(eos_tcb_t *task) {
	return task->period;
}

int32u_t eos_suspend_task(eos_tcb_t *task) {

	int32u_t index = task->priority;
	task->state = WAITING;
	_os_remove_node(&_os_ready_queue[index], task->node);
	
	//if no process in the given priority queue, turn off the ready_bit
	if(_os_ready_queue[index]==NULL){
		_os_unset_ready(index);
	}

}

int32u_t eos_resume_task(eos_tcb_t *task) {
	
	int32u_t priority = task->priority;
	task->state = READY;
	_os_add_node_tail(&_os_ready_queue[priority], task->node);
	_os_set_ready(priority);
}

void eos_sleep(int32u_t tick) {
	
	_os_current_task->state = WAITING;
	
	//time to sleep
	int32u_t t = _os_current_task->period;
	
	//set alarm
	eos_set_alarm(eos_get_system_timer(), _os_current_task->alarm, 
			eos_get_system_timer()->tick+t, 
			_os_wakeup_sleeping_task, _os_current_task);			
	
	//reschedule
	eos_schedule();
}

void _os_init_task() {
	PRINT("initializing task module.\n");

	/* init current_task */
	_os_current_task = NULL;

	/* init multi-level ready_queue */
	int32u_t i;
	for (i = 0; i < LOWEST_PRIORITY+1; i++) {
		_os_ready_queue[i] = NULL;
	}
}

void _os_wait(_os_node_t **wait_queue) {

}

void _os_wakeup_single(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_all(_os_node_t **wait_queue, int32u_t queue_type) {
}

void _os_wakeup_sleeping_task(void *arg) {
	//arg : tcb
	eos_tcb_t* tcb = (eos_tcb_t*)arg;
	int32u_t priority = tcb->priority;
	_os_add_node_tail(&_os_ready_queue[priority], tcb->node);
	_os_set_ready(priority);
	tcb->state = READY;
}
