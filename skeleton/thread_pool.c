#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "thread_pool.h"

#define MAX_THREADS 20
#define QUEUE_SIZE 20

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  Feel free to make any modifications you want to the function prototypes and structs
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

typedef struct {
    void (*function)(void *);
    void *argument;
} threadpool_task_t;


struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
  threadpool_task_t *queue;
  int thread_count;
  int task_queue_size_limit;
};

/**
 * @function void *threadpool_work(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *thread_do_work(void *threadpool);


/*
 * Create a threadpool, initialize variables, etc
 *
 */
threadpool_t *threadpool_create(int thread_count, int queue_size)
{
    threadpool_t* pool = malloc(sizeof(threadpool_t));
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * MAX_THREADS);
    pool->queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t)* QUEUE_SIZE);

    //Initialize Mutex
    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_initi(&(pool->notify), NULL);

    for(int i=0; i<MAX_THREADS; i++){
	pthread_create(&(pool->threads[i]), NULL, threadpool_do_work, (void*)pool);
	pool->thread_count++;
    }

    return pool;
}


/*
 * Add a task to the threadpool
 *
 */
int threadpool_add_task(threadpool_t *pool, void (*function)(void *), void *argument)
{
    int err = 0;
    /* Get the lock */
    /* Add task to queue */
        
    /* pthread_cond_broadcast and unlock */
    
    return err;
}



/*
 * Destroy the threadpool, free all memory, destroy treads, etc
 *
 */
int threadpool_destroy(threadpool_t *pool)
{
    int err = 0;

    
    /* Wake up all worker threads */
    

    /* Join all worker thread */
        

    /* Only if everything went well do we deallocate the pool */
    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *threadpool)
{ 

    while(1) {
        /* Lock must be taken to wait on conditional variable */
        

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), do some task. */
        
        
        /* Grab our task from the queue */
        

        /* Unlock mutex for others */


        /* Start the task */

    }

    pthread_exit(NULL);
    return(NULL);
}
