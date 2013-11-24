/*
 * NAME, etc.
 *
 * sync.c
 *
 * Synchronization routines for SThread
 */

#define _REENTRANT

//#define DEBUG 

#include "sthread.h"

#include <memory.h>

#include <stdlib.h>

#include <stdio.h>
/*
 * Queue routines 
 */

int Q_init(struct queue *pQ)
{
    pQ->size = 0;
    pQ->head = -1 + MAX_Q_SIZE;
    pQ->tail = -1 + MAX_Q_SIZE;
    memset(pQ->member,0,sizeof(pQ->member));
    return 0;
}

int Q_empty(struct queue *pQ)
{
    return (pQ->size == 0); 
}

int Q_full(struct queue *pQ)
{
    return pQ->size == MAX_Q_SIZE;
}

struct q_member * Q_first(struct queue *pQ)
{
    if(Q_empty(pQ))
        return NULL;
    else 
        return &pQ->member[(pQ->head + 1) % MAX_Q_SIZE ];
}

struct q_member * Q_last(struct queue *pQ)
{
    if(Q_empty(pQ))
        return NULL;
    else
        return &pQ->member[pQ->tail];
}

int Q_push(struct queue *pQ,  sthread_t ptr_thread)
{
    #ifdef DEBUG
    printf("Q_push\n");
    #endif

    if(pQ->size >= MAX_Q_SIZE)
    {
        //Queue already full
        #ifdef DEBUG
        printf("Queue full\n");
        #endif
        return -1;
    }
    else
    {
        #ifdef DEBUG
        printf("Queue NOT full\n");
        #endif
 
        pQ->size++;
        pQ->tail = ((pQ->tail + 1) % MAX_Q_SIZE);
        struct q_member * p_cnt_mem = Q_last(pQ);
        p_cnt_mem->ptr_thread = ptr_thread;
        p_cnt_mem->count = 1;
        #ifdef DEBUG
        printf("size of Q: %d\n",pQ->size);
        #endif
        return 0;
    }
}

struct q_member * Q_pop(struct queue *pQ)
{
    #ifdef DEBUG
    printf("Q_pop\n");
    #endif

    if(Q_empty(pQ))
    {
        #ifdef DEBUG
        printf("Queue empty\n");
        #endif
        //instant fail
        return NULL;
    }
    else
    {
        pQ->size--;
        #ifdef DEBUG
        printf("size of Q %d\n",pQ->size);
        #endif
        struct q_member * p_first = Q_first(pQ);
        pQ->head = (pQ->head + 1 % MAX_Q_SIZE);
        return p_first;
    }
}

/*
 * Mutex routines
 */

int sthread_mutex_init(sthread_mutex_t *mutex)
{
    //lock should be available
    //to be acquired initially
    mutex->M = 0;
    #ifdef DEBUG
    printf("Initial value for M %lu\n",mutex->M);
    #endif
    //Q should start out being empty
    Q_init(&mutex->Q);
    return 0;
}

int sthread_mutex_destroy(sthread_mutex_t *mutex)
{
    struct queue *pQ = &mutex->Q;
    //fighting for the right to own lock
    while(test_and_set(&mutex->M)){}
    if(Q_empty(pQ))
    {
        //TODO
        free(mutex);
        return 0;
    }
    else
    {
        //destroy mutex error: Q is not empty;
        return -1;
    }
}

int sthread_mutex_lock(sthread_mutex_t *mutex)
{
    //fighting for the right to modify Q
    while(test_and_set(&mutex->M)){}
    //after this I am now the only one
    //to modify the passed in mutex
    //therefore I should go ahead and
    //modify the lock and Q if needed
    sthread_t ptr_this_thread = sthread_self();
    #ifdef DEBUG
    printf("sthread_mutex_lock()\n");
    #endif

    //check if I should go into critical
    //section
    struct queue *pQ = &mutex->Q;
    if(Q_empty(pQ))
    {
        //lock available
        #ifdef DEBUG
        printf("Q_empty\n lock available\n");
        #endif
        Q_push(pQ, ptr_this_thread);
        mutex->M = 0; // I am done with Q; others can come and look / change Q.
        #ifdef DEBUG
        printf("set M to 0\n");
        #endif
        return 0; //this_thread will go into critical section
    }
    else
    {
        //lock not available
        //there are someone or myself before me in the Q
        //if it is myself in criticle section
            //I shoud not wait for myself to wake myself up
            //I will increment counter and keep running in criticle seciton
        //else
            //someone else is before me in the Q 
            //either he is sleeping (he is not the 1st)
            //or he is in the critical section now (he is the 1st and 
            //I am the 2nd in Q)
                //either way I should go to bed and wait for him to wake me up
        if(Q_first(pQ)->ptr_thread == ptr_this_thread)
        {
            #ifdef DEBUG
            printf("Try to acquire the same lock again");
            #endif
            Q_first(pQ)->count++; //Trying to acquire the lock own by myself
                                 //take down this attempt and keep myself in
                                 //criticle section
            mutex->M = 0; //I am done with lock; others can now check / modify lock.
            #ifdef DEBUG
            printf("set M to 0\n");
            #endif
        }
        else
        {
            #ifdef DEBUG
            printf("The first one in Queue is not me\n");
            #endif

            Q_push(pQ, ptr_this_thread);
            mutex->M = 0;
            sthread_suspend();  //Other thread is in criticle section
                        //I should go to bed and sleep 
            return 0;   //the moment I am woken up will
                        //return 0 to caller
        }
        
    }
}

int sthread_mutex_trylock(sthread_mutex_t *mutex)
{
    sthread_t ptr_this_thread = sthread_self();
    //fighting for the right to modify Q
    while(test_and_set(&mutex->M)){}
    //after this I am now the only one
    //to modify the passed in mutex
    //therefore I should go ahead and
    //modify the lock and Q if needed

    //check if I should go into critical
    //section
    struct queue *pQ = &mutex->Q;
    if(Q_empty(pQ))
    {
        //lock available
        Q_push(pQ, ptr_this_thread);
        mutex->M = 0; // I am done with Q; others can come and look / change Q.
        //this_thread will go into critical section
        return 0;
    }
    else
    {
        //lock not available
        if(Q_first(pQ)->ptr_thread == ptr_this_thread)
        {
            Q_first(pQ)->count++;//Trying to acquire the lock own by myself
                                 //take down this attempt and keep myself in
                                 //criticle section
            mutex->M = 0; //I am done with lock; others can now check / modify lock.
        }
        //do not block caller
        //return non-zero indication lock not available
        return -1;
    }

}

int sthread_mutex_unlock(sthread_mutex_t *mutex)
{
    sthread_t ptr_this_thread = sthread_self();
    //fighting for the right to modify Q
    while(test_and_set(&mutex->M) == 1){}
    //I am the only one checking mutex
    //can modify Q
    #ifdef DEBUG
    printf("sthread_mutex_unlock()\n");
    #endif
    struct queue *pQ = &mutex->Q;
    if(Q_first(pQ)->count == 1)
    {
        //This is the outter most unlock
        //should pop myself out of Q
        //if needed
        #ifdef DEBUG
            printf("count is 1\n");
        #endif
        if(ptr_this_thread == Q_first(pQ)->ptr_thread)
        {
            //Should only unlock lock own by myself
            //never unlock others' locks
            Q_pop(pQ);        
        }
        else
        {
            printf("error unlokcing lock own by others");
            return -1;
        }

        if(!Q_empty(pQ))
        {
            //Queue is not empty
            //there are other threads sleeping
            //int the Q
            #ifdef DEBUG
            printf("wake up the first person\n");
            #endif
            sthread_wake(Q_first(pQ)->ptr_thread);
        }
        else
        {
            //I am the last one int the Q
            //Do nothing
            #ifdef DEBUG
            printf("Q empty\n");
            #endif

        }
        mutex->M = 0;
        return 0;
    }
    else if( Q_first(pQ)->count > 1)
    {
        #ifdef DEBUG
            printf("count is more than one\n");
        #endif
        //I have acquired the lock owned by me 
        //more than once need to unlock 
        Q_first(pQ)->count--;
        mutex->M = 0;
    }
    else
    {
        #ifdef DEBUG
            printf("something wrong happened\n");
        #endif
        // printf("Bad!! count is 0 or less\n");
        return -1;
    }
}

//I should implement the Q_push / Q_pop / Q_emtpy fucntion just take in the
//address of the mutex 
//
//instead of take in the exact postion of a Q address
//because it is a little bit waste of calculation
//
//
//with what I should do be done,when I write the operation to Q
//
//I just need to worry about which lock to pass in 
//and do not need to calculate the address of Q to passin every single time. 

