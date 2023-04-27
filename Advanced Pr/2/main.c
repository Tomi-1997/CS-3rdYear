#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include<unistd.h>

#include "codec.h"
#include "queue.h"

#define DATA_SIZE 1024
#define debug 0


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  queue_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t  work_cond = PTHREAD_COND_INITIALIZER;


Queue* work;
int key;
int thread_num;
int FIN_WORK;


struct data_block_
{
	char block[DATA_SIZE];
	int order_num;
	bool finished;	
} typedef data_block;


struct worker_
{
	data_block* current_work;
	pthread_cond_t wcond;
} typedef worker;


void* dispatcher(void* workers_)
{
	worker* workers = (worker*)(workers_);
	int current_output = 0;
	while (true)
	{
		/* Sleep until a worker wakes you up */
		pthread_mutex_lock( &lock );

		if (debug)
			puts("Dispatcher went to sleep");

		while (FIN_WORK == 0)
			pthread_cond_wait( &work_cond, &lock );

		if (debug)
			puts("Dispatcher woke up");
		
		/* Find which worker finished */
		for (int i = 0; i < thread_num; i++)
		{
			worker* w = &workers[i];
			data_block* cw = w->current_work;
			if (cw == NULL)
				continue;

			/* If a worker finished, and the next output is his work, 
			print his work, free it and wake up him to keep working. */
			if (cw->finished && cw->order_num == current_output)
				{
					if (debug)
						printf("<%d>\n", current_output);
					current_output++;
					printf("%s\n", cw->block);
					// fflush(stdout); \n already flushes
					w->current_work = NULL;
					free(cw);
					FIN_WORK--;
					pthread_cond_signal(&(w->wcond));
				}
		}
		pthread_mutex_unlock( &lock );
	}
	return NULL;
}


void* worker_func(void* worker_)
{
	worker* w = (worker*)(worker_);
	while(true)
	{
		/* Sleep, awake when work queue is not empty*/
		while(work->size == 0)
		{
			pthread_mutex_lock( &lock );
			if (debug)
				puts("worker sleeping");
			pthread_cond_wait( &queue_cond, &lock );
			if (debug)
				puts("worker awaken");
			pthread_mutex_unlock( &lock );
		}

		pthread_mutex_lock( &lock );
		void* ret = deQ(&work);
		pthread_mutex_unlock( &lock );

		/* Check if another worker already took job */
		if (!w) continue;
		w->current_work = (data_block*) (ret);

		/* Is it a dummy work? then we are finished */
		if (w->current_work->order_num == -1)
			return 0;
			
		// Do task
		w->current_work->finished = false;
		encrypt(w->current_work->block, key);
		w->current_work->finished = true;

		// Shout to manager that work is done, sleep until he notices
		pthread_mutex_lock( &lock );

		if (debug)
			puts("work done");
			
		pthread_cond_signal( &work_cond );
		FIN_WORK++;
		pthread_mutex_unlock( &lock );

		while (w->current_work != NULL)
		{
			pthread_mutex_lock( &lock );
			pthread_cond_wait( &(w->wcond), &lock );

			pthread_mutex_unlock( &lock );
		}
	}
	return NULL;
}


data_block* arr_to_work(char* data, int order)
{
	data_block* w = malloc(sizeof(data_block));
	if (w == NULL)
	{
		printf("Data block malloc failure at main thread.");
		return NULL;
	}

	strcpy(w->block, data);
	w->order_num = order;
	w->finished = false;
	return w;
}


int main(int argc, char *argv[])
{
	if (0 && argc != 2)
	{
	    printf("usage: key < file \n");
	    printf("!! data more than 1024 char will be ignored !!\n");
	    return 0;
	}

	key = atoi(argv[1]);

	if (debug)
		printf("key is %i \n",key);

	pthread_t* threadpool;
	pthread_t disp;
	worker* workers;
	work = createQ();
	
	char c;
	int counter = 0;
	char data[DATA_SIZE];
	int input_order = 0;
	FIN_WORK = 0;
	// setbuf(stdout, NULL);

	// CPU Num minus one
	thread_num = sysconf(_SC_NPROCESSORS_ONLN) - 1;
	threadpool = malloc( sizeof(pthread_t) * thread_num );
	
	if (threadpool == NULL)
	{
		printf("Malloc threadpool err");
		return 1;
	}

	workers = malloc( sizeof(worker) * thread_num );

	if (workers == NULL)
	{
		printf("Malloc threadpool err");
		return 1;
	}
	
	for (int i = 0; i < thread_num; i++)
	{
		pthread_create( & (threadpool[i]), NULL, &worker_func, (void*)(&workers[i]));
		pthread_cond_init(&workers[i].wcond, NULL);
	}
	
	pthread_create( &disp, NULL, &dispatcher, (void*)(workers) );

	while ((c = getchar()) != EOF)
	{
	  data[counter] = c;
	  counter++;

	  if (counter == DATA_SIZE - 1)
	  {
		data[counter] = '\0';
		/* Create work instance */
		data_block* current_work = arr_to_work(data, input_order);

		/* Add work to overall workload */
		pthread_mutex_lock( &lock );
		enQ(&work, (void*)current_work);

		/* Let the workers know new data is added */
		pthread_cond_signal( &queue_cond );
		if (debug)
			puts("Sending work");
		pthread_mutex_unlock( &lock );

		counter = 0;
		input_order++;
	  }
	}

	if (debug)
		puts("fin");

	if (counter > 0)
	{
		char lastData[counter];
		// lastData[0] = '\0';
		strncat(lastData, data, counter);
		lastData[counter] = '\0';

		/* Create work instance */
		data_block* current_work = arr_to_work(lastData, input_order);

		/* Add work to overall workload */
		pthread_mutex_lock( &lock );
		enQ(&work, (void*)current_work);
		pthread_cond_signal( &queue_cond );
		if (debug)
			puts("Sending work");
		pthread_mutex_unlock( &lock );

		
	}

	// Mark the end of jobs, send dummy work to everyone.
	data_block dummy;
	dummy.block[0] = '\0';
	dummy.order_num = -1;
	dummy.finished = true;
	for (int i = 0; i < thread_num; i++)
	{
		pthread_mutex_lock( &lock );
		enQ(&work, (void*)&dummy);
		pthread_cond_signal( &queue_cond );
		if (debug)
			puts("Sending dummy");
		pthread_mutex_unlock( &lock );
	}

	for (int i = 0; i < thread_num; i++)
	{
		pthread_join(threadpool[i], NULL);
	}
	return 0;
}
