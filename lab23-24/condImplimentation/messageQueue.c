#include "messageQueue.h"
#include "cond.h"
#include "../mutexService/mutexService.h"
#include <stdlib.h>
#include <string.h>

void mymsginit(MessageQueue *queue, int msgLimit){
    queue->in = NULL;
    queue->out = NULL;
    queue->isDestroyed = 0;
    queue->msgLimit = msgLimit;
    queue->msgCount = 0;

    condTryInit(&queue->condAccess);
    mutexTryInit(&queue->mutexAccess);
}
size_t mymsgput(MessageQueue *queue, char *text){
    mutexTryLock(&queue->mutexAccess);
    while (!queue->isDestroyed && queue->msgCount >= queue->msgLimit){
        condTryWait(&queue->condAccess, &queue->mutexAccess);
    }
    if(queue->isDestroyed){
        mutexTryUnlock(&queue->mutexAccess);
        return 0;
    }

    Message *newMessage = malloc(sizeof(Message));
    strncpy(newMessage->text, text, sizeof(newMessage->text));
    newMessage->text[sizeof(newMessage->text) - 1] = '\0';
    newMessage->prev = NULL;
    newMessage->next = queue->in;

    if(queue->in == NULL){
        queue->in = newMessage;
        queue->out = newMessage;
    }else{
        queue->in->prev = newMessage;
        queue->in = newMessage;
    }

    if(!queue->msgCount){
        condTrySignal(&queue->condAccess);
    }
    queue->msgCount++;
    mutexTryUnlock(&queue->mutexAccess);
    return strlen(newMessage->text) + 1;
}
size_t mymsgget(MessageQueue *queue, char *buffer, size_t bufferSize){
    mutexTryLock(&queue->mutexAccess);
    while (!queue->msgCount && !queue->isDestroyed){
        condTryWait(&queue->condAccess, &queue->mutexAccess);
    }

    if(queue->isDestroyed){
        mutexTryUnlock(&queue->mutexAccess);
        return 0;
    }

    Message *message = queue->out;
    if(queue->in == message){
        queue->in = NULL;
        queue->out = NULL;
    }else{
        queue->out = message->prev;
        queue->out->next = NULL;
    }

    if(queue->msgCount == queue->msgLimit){
        condTrySignal(&queue->condAccess);
    }
    queue->msgCount--;
    mutexTryUnlock(&queue->mutexAccess);

    strncpy(buffer, message->text, bufferSize);
    buffer[bufferSize - 1] = '\0';
    free(message);

    return strlen(buffer);
}
void mymsgqdrop(MessageQueue *queue){
    queue->isDestroyed = 1;
    condTryBroadcast(&queue->condAccess);

    mutexTryLock(&queue->mutexAccess);
    Message *message = queue->in;
    while (message){
        Message *buf = message->next;
        free(message);
        message = buf;
    }
    mutexTryUnlock(&queue->mutexAccess);

}
void mymsgdestroy(MessageQueue *queue){
    condTryDestroy(&queue->condAccess);
    mutexTryDestroy(&queue->mutexAccess);
}

