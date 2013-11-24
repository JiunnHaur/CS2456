/*
 * NAME, etc.
 *
 * sync.c
 *
 * Synchronization routines for SThread
 */

#define _REENTRANT

#include "sthread.h"

#include <memory.h>

#include <stdlib.h>
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
    return pQ->size == 0; 
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
    if(pQ->size >= MAX_Q_SIZE)
        //Queue alread full
        return -1;
    else
    {
        pQ->size++;
        pQ->tail = ((pQ->tail + 1) % MAX_Q_SIZE);
        struct q_member * p_cnt_mem = Q_last(pQ);
        p_cnt_mem->ptr_thread = ptr_thread;
        p_cnt_mem->count = 0;
        return 0;
    }
}

struct q_member * Q_pop(struct queue *pQ)
{
    if(Q_empty(pQ))
        //instant fail
        return NULL;
    else
    {
        pQ->size--;
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
    int i=0;
    //lock should be available
    //to be acquired initially
    mutex->M = 0;
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
            Q_first(pQ)->count++; //Trying to acquire the lock own by myself
                                 //take down this attempt and keep myself in
                                 //criticle section
            mutex->M = 0; //I am done with lock; others can now check / modify lock.
        }
        else
        {
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
    //fighting for the right to modify Q
    while(test_and_set(&mutex->M) == 1){}
    //I am the only one checking mutex
    //can modify Q
    struct queue *pQ = &mutex->Q;
    if(Q_first(pQ)->count == 1)
    {
        Q_pop(pQ);
        if(Q_empty(pQ) == 0)
        {
            //Queue is not empty
            //there are other threads sleeping
            //int the Q
            sthread_wake(Q_first(pQ)->ptr_thread);
        }
        else
        {
            //I am the last one int the Q
            //Do nothing
        }
        return 0;
    }
    else if( Q_first(pQ)->count > 1)
    {
        //I have acquired the lock owned by me 
        //more than once need to unlock 
        Q_first(pQ)->count--;
    }
    else
    {
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

