#include "messageQueue.h"
#include "semaphoreService.h"
#include <string.h>
#include "../mutexService/mutexService.h"

void mymsginit(MessageQueue *queue, int msgLimit){
    queue->in = NULL;
    queue->out = NULL;
    queue->isDestroyed = 0;
    mutexTryInit(&queue->mutexAccess);
    semTryInit(&queue->semPut, 0, msgLimit);
    semTryInit(&queue->semGet, 0, 0);
}
size_t mymsgput(MessageQueue *queue, char *text){
    semTryWait(&queue->semPut);
    if(queue->isDestroyed){
        semTryPost(&queue->semPut);
        return 0;
    }

    mutexTryLock(&queue->mutexAccess);

    Message *newMessage = malloc(sizeof(Message));
    strncpy(newMessage->text, text, sizeof(newMessage->text));
    newMessage->text[sizeof(newMessage->text) - 1] = '\0';
    newMessage->prev = NULL;
    newMessage->next = queue->in;

    if(queue->in == NULL){
        queue->in = newMessage;
        queue->out = newMessage;
    } else{
        queue->in->prev = newMessage;
        queue->in = newMessage;
    }

    mutexTryUnlock(&queue->mutexAccess);
    semTryPost(&queue->semGet);
    return strlen(newMessage->text) + 1;
}
size_t mymsgget(MessageQueue *queue, char *buffer, size_t bufferSize){
    semTryWait(&queue->semGet);
    if(queue->isDestroyed){
        semTryPost(&queue->semGet);
        return 0;
    }

    mutexTryLock(&queue->mutexAccess);

    Message *message = queue->out;
    if(queue->in == message){
        queue->in = NULL;
        queue->out = NULL;
    } else{
        queue->out = message->prev;
        queue->out->next = NULL;
    }

    strncpy(buffer, message->text, bufferSize);
    buffer[bufferSize - 1] = '\0';
    free(message);

    mutexTryUnlock(&queue->mutexAccess);
    semTryPost(&queue->semPut);

    return strlen(buffer);
}
void mymsgqdrop(MessageQueue *queue){
    queue->isDestroyed = 1;
    semTryPost(&queue->semPut);
    semTryPost(&queue->semGet);

    Message *message = queue->in;
    while (message){
        Message *buf = message->next;
        free(message);
        message = buf;
    }
}
void mymsgdestroy(MessageQueue *queue){
    semTryDestroy(&queue->semPut);
    semTryDestroy(&queue->semGet);
}

