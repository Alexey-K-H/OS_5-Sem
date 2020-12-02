//
// Created by ninetail on 12/2/20.
//

#ifndef LAB26_28_MUTEX_H
#define LAB26_28_MUTEX_H

#include <pthread.h>

void mutex_try_init(pthread_mutex_t *mutex);
void mutex_try_lock(pthread_mutex_t *mutex);
void mutex_try_unlock(pthread_mutex_t *mutex);
void mutex_try_destroy(pthread_mutex_t *mutex);


#endif //LAB26_28_MUTEX_H
