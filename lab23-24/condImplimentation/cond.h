#ifndef LAB23_24_COND_H
#define LAB23_24_COND_H

#include <pthread.h>

void condTryInit(pthread_cond_t *cond);
void condTrySignal(pthread_cond_t *cond);
void condTryBroadcast(pthread_cond_t *cond);
void condTryWait(pthread_cond_t *cond, pthread_mutex_t *mutex);
void condTryDestroy(pthread_cond_t *cond);

#endif //LAB23_24_COND_H
