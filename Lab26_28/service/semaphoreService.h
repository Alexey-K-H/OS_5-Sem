#ifndef LAB23_24_SEMAPHORESERVICE_H
#define LAB23_24_SEMAPHORESERVICE_H

#include <semaphore.h>

void semTryInit(sem_t *sem, int shared, int initialValue);
void semTryPost(sem_t *sem);
void semTryWait(sem_t *sem);
void semTryDestroy(sem_t *sem);

#endif //LAB23_24_SEMAPHORESERVICE_H
