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
  pthread_t* threads;
  threadpool_task_t* queue;
  int queueItems;
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
    pool->queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t) * QUEUE_SIZE);
    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);
    pool->queueItems = 0;
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
    /* Get the lock */
    /* Add task to queue */
    if (pool->queueItems <= QUEUE_SIZE)
    {
        threadpool_task_t* newTask = &(pool->queue[pool->queueItems]);
        newTask->function = function;
        newTask->argument = argument;
        pool->queueItems++;
    }
    //queue size full - return error
    else
    {
        return 1;
    }
        
    /* pthread_cond_broadcast and unlock */
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));	    

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
    err = err & pthread_cond_broadcast(&(pool->notify));
    err = err & pthread_mutex_unlock(&(pool->lock));

    /* Join all worker thread */
    int i;
    for (i = 0; i < MAX_THREADS; i++)
    {
        err = err & pthread_join(pool->threads[i], NULL);
    }    

    /* Only if everything went well do we deallocate the pool */
    if (!err)
    {
        free(pool);
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
        threadpool_task_t currentTask = pool->queue[0];

        /* Unlock mutex for others */
	pthread_mutex_unlock(&(pool->lock));

        /* Start the task */
        (*(currentTask.function))(currentTask.argument);
    }

    pthread_exit(NULL);
    return(NULL);
}
