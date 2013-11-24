/*
 * NAME, etc.
 *
 * sync.h
 */

#ifndef _STHREAD_SYNC_H_
#define _STHREAD_SYNC_H_

#define MAX_Q_SIZE 25

struct q_member{
    sthread_t ptr_thread;
    int count;
};

struct queue{
    struct q_member member[MAX_Q_SIZE];
    int size;
    int head;
    int tail;
};

struct sthread_mutex_struct {
  /* FILL ME IN! */
    unsigned long M;
    struct queue Q;
};

typedef struct sthread_mutex_struct sthread_mutex_t;

int sthread_mutex_init(sthread_mutex_t *mutex);
int sthread_mutex_destroy(sthread_mutex_t *mutex);
int sthread_mutex_lock(sthread_mutex_t *mutex);
int sthread_mutex_trylock(sthread_mutex_t *mutex);
int sthread_mutex_unlock(sthread_mutex_t *mutex);

#endif
