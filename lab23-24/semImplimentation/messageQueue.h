#ifndef LAB23_24_MESSAGEQUEUE_H
#define LAB23_24_MESSAGEQUEUE_H

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#define STRING_LENGTH_LIMIT 80

typedef struct Message{
    struct Message *prev, *next;
    char text[STRING_LENGTH_LIMIT + 1];
}Message;

typedef struct {
    Message *in, *out;
    sem_t semPut, semGet;
    pthread_mutex_t mutexAccess;
    unsigned char isDestroyed;
    unsigned msgLimit;
}MessageQueue;

void mymsginit(MessageQueue *queue, int msgLimit);
size_t mymsgput(MessageQueue *queue, char *text);
size_t mymsgget(MessageQueue *queue, char *buffer, size_t bufferSize);
void mymsgqdrop(MessageQueue *queue);
void mymsgdestroy(MessageQueue *queue);


#endif //LAB23_24_MESSAGEQUEUE_H
