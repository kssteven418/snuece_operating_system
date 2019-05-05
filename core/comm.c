/********************************************************
 * Filename: core/comm.c
 *  
 * Author: jtlim, RTOSLab. SNU.
 * 
 * Description: message queue management. 
 ********************************************************/
#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {
	mq->queue_start = queue_start;
	mq->queue_size = queue_size;
	mq->msg_size = msg_size;
	mq->front = queue_start;
	mq->rear = queue_start;
	mq->queue_type = queue_type;
	
	eos_init_semaphore(&(mq->putsem), queue_size, queue_type);
	eos_init_semaphore(&(mq->getsem), 0, queue_type);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	int i = 0;

	if(!eos_acquire_semaphore(&(mq->putsem), timeout)) return 0;
	
	for(i=0; i<mq->msg_size; i++){
			
		//add new byte at the rear
		*((int8u_t*)(mq->rear)) = ((int8u_t*)(message))[i];

		//update rear
		mq->rear++;
		
		if(mq->rear == mq->queue_start + mq->queue_size * mq->msg_size)
			mq->rear = mq->queue_start;
	}

	eos_release_semaphore(&(mq->getsem));
	return 1;
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	int i = 0;
	
	if(!eos_acquire_semaphore(&(mq->getsem), timeout)) return 0;
		
		for(i=0; i<mq->msg_size; i++){
			
			//add new byte at the rear
			((int8u_t*)(message))[i] = *((int8u_t*)(mq->front)) ;

			//update front
			mq->front++;
			
			if(mq->front == mq->queue_start + mq->queue_size * mq->msg_size)
				mq->front = mq->queue_start;
		}

	eos_release_semaphore(&(mq->putsem));
	return 1;
}
