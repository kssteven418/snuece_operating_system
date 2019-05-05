/********************************************************
 * Filename: core/sync.c
 * 
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>

extern int32u_t _eflags;

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
	/* initialization */
	sem->count = initial_count;
	sem->wait_queue = NULL;
	sem->queue_type = queue_type;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
	_eflags = 0;
	
	//if semaphor is available
	if(sem->count > 0){
		sem->count--;
		_eflags = 1;
		return 1;
	}
	
	//if semaphor is NOT availble
	else{
		//negative timeout : fail
		if(timeout<0){
			_eflags = 1;
			return 0;
		}
		
		// zero timeout : sleep at semaphor wait queue
		else if (timeout == 0){
			eos_tcb_t *task = eos_get_current_task();
			
			//remove task from ready queue
			eos_suspend_task(task);
			
			//add task at semaphor waiting queue
			if(sem->queue_type == 0){ // FIFO
				_os_add_node_tail(&(sem->wait_queue), task->node); 
			}
			else if (sem->queue_type == 1){ // priority
				_os_add_node_priority(&(sem->wait_queue), task->node);
			}

			while(1){
				_eflags = 1;
			
				//reschedule
				eos_schedule();

				/*resume point after wake-up*/
					
				_eflags = 0;	
				
				if(sem->count > 0){
					sem->count--;
					_eflags = 1;
					return 1;
				}
				else{
					//not available, then some other tasks are using the semaphor..
					//put into the semaphor waiting queue again
					
					//remove task from ready queue
					eos_suspend_task(task);
					
					if(sem->queue_type == 0){ // FIFO
						//push at the tail
						_os_add_node_tail(&(sem->wait_queue), task->node); 
					}
					else if (sem->queue_type == 1){ // priority
						//push according to the priority
						_os_add_node_priority(&(sem->wait_queue), task->node);
					}
				}
			}
		}

		else{ // timeout > 0

			while(1){	
				
				_eflags = 1;
				eos_sleep(timeout); // suspend + append at timer waiting queue
				
				/*resume point after wake-up*/
				
				_eflags = 0;
				if(sem->count > 0){
					sem->count--;
					_eflags = 1;
					return 1;
				}
		
			}
		}
	}		
}

void eos_release_semaphore(eos_semaphore_t *sem) {
	
	_eflags = 0;
	sem->count ++;

	//waking logic
	if(sem->wait_queue != NULL){ // Not empty
		//remove from semaphor waiting queue
		_os_node_t* wake_node = sem->wait_queue;
		_os_remove_node(&(sem->wait_queue), wake_node);
		
		//append the task at ready_queue
		eos_resume_task((eos_tcb_t*)(wake_node->ptr_data));
	}
	_eflags = 1;

}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
	/* initialization */
	cond->wait_queue = NULL;
	cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
	/* release acquired semaphore */
	eos_release_semaphore(mutex);
	/* wait on condition's wait_queue */
	_os_wait(&cond->wait_queue);
	/* acquire semaphore before return */
	eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
	/* select a task that is waiting on this wait_queue */
	_os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
