#ifndef LAB23_24_MUTEXSERVICE_H
#define LAB23_24_MUTEXSERVICE_H

#include <pthread.h>

void mutexTryInit(pthread_mutex_t *mutex);
void mutexTryLock(pthread_mutex_t *mutex);
void mutexTryUnlock(pthread_mutex_t *mutex);
void mutexTryDestroy(pthread_mutex_t *mutex);

#endif //LAB23_24_MUTEXSERVICE_H
