//
// Created by ninetail on 12/2/20.
//

#ifndef LAB26_28_COND_H
#define LAB26_28_COND_H

#include <pthread.h>

void cond_try_init(pthread_cond_t *cond);
void cond_try_signal(pthread_cond_t *cond);
void cond_try_broadcast(pthread_cond_t *cond);
void cond_try_wait(pthread_cond_t *cond, pthread_mutex_t* mutex);
void cond_try_destroy(pthread_cond_t *cond);

#endif //LAB26_28_COND_H
