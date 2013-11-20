#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "thread_pool.h"

#define MAX_THREADS 50
#define QUEUE_SIZE 50

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
  pthread_t* threads;
  threadpool_task_t* queue;
  int queueItems;
  int headIndex;
  int tailIndex;
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
threadpool_t *threadpool_create()
{
    threadpool_t* pool = malloc(sizeof(threadpool_t));
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * MAX_THREADS);
    pool->queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t) * QUEUE_SIZE);
    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);
    pool->queueItems = 0;
    pool->headIndex = 0;
    pool->tailIndex = 0;
    int i;
    for (i = 0; i < MAX_THREADS; i++)
    {
        pthread_create(&(pool->threads[i]), NULL, thread_do_work, (void*) pool);
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
    int nextIndex;
    /* Get the lock */
    err = err | pthread_mutex_lock(&(pool->lock));
    /* Add task to queue */
    if (pool->queueItems <= QUEUE_SIZE)
    {
        nextIndex = pool->tailIndex + 1;
        //if index of tail is equal to the size of the queue, circle it back to index 0
        if (nextIndex == QUEUE_SIZE)
        {
            nextIndex = 0;
        }
        //add item to tail of queue, increment total queue count
        pool->queue[pool->tailIndex].function = function;
        pool->queue[pool->tailIndex].argument = argument;
        pool->tailIndex = nextIndex;
        pool->queueItems++;
    }
    //queue size full - return error
    else
    {
        printf("%s \n","queue size full");
        return 1;
    }
        
    /* pthread_cond_broadcast and unlock */
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));	    
    if (err) printf("%s \n","add task error");
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
    err = err | pthread_mutex_lock(&(pool->lock));
    err = err | pthread_cond_broadcast(&(pool->notify));
    err = err | pthread_mutex_unlock(&(pool->lock));

    /* Join all worker thread */
    int i;
    for (i = 0; i < MAX_THREADS; i++)
    {
        err = err | pthread_join(pool->threads[i], NULL);
    }    

    /* Only if everything went well do we deallocate the pool */
    if (!err)
    {
        free(pool);
    }
    else
    {
        printf("%s \n","threadpool destroy error");
    }
    return err;
}



/*
 * Work loop for threads. Should be passed into the pthread_create() method.
 *
 */
static void *thread_do_work(void *threadpool)
{
    threadpool_t* pool = (threadpool_t*)threadpool; 

    while(1) {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), do some task. */
        while (pool->queueItems == 0) pthread_cond_wait(&(pool->notify), &(pool->lock));
        
        /* Grab our task from the queue */
        threadpool_task_t currentTask = pool->queue[pool->headIndex];
        //shift head forward, decrement queue count
        pool->headIndex++;
        pool->queueItems--;
        //if index of head is equal to the size of the queue, circle it back to index 0
        if (pool->headIndex == QUEUE_SIZE)
        {
            pool->headIndex = 0;
        }

        /* Unlock mutex for others */
	pthread_mutex_unlock(&(pool->lock));

        /* Start the task */
        (*(currentTask.function))(currentTask.argument);
    }

    pthread_exit(NULL);
    return(NULL);
}
