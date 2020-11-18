#include "semImplimentation/messageQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "ConsoleColors.h"

#define MESSAGES_LIMIT 10

MessageQueue queue;
pthread_t* threads;
int nProducers;
int nConsumers;

int getThreadId(){
    pthread_t self = pthread_self();
    for(int i = 0; i < nProducers + nConsumers; i++){
        if(pthread_equal(self, threads[i])){
            return i < nProducers ? i : i - nProducers;
        }
    }
    return -1;
}

void *producer(){
    int id = getThreadId();
    char messageText[STRING_LENGTH_LIMIT + 1];
    int messageNumber= 1;

    while (1){
        sprintf(messageText, "Hello from PRODUCER#%d [%d]", id, messageNumber++);
        if(!mymsgput(&queue, messageText)){
            pthread_exit((void *)0);
        }
        printf("%s[PRODUCER#%d] Put: %s\n", GREEN_COLOR, id, messageText);
    }
}

void *consumer(){
    int id = getThreadId();
    char messageText[STRING_LENGTH_LIMIT + 1];

    while (1){
        if(!mymsgget(&queue, messageText, STRING_LENGTH_LIMIT + 1)){
            pthread_exit((void *)0);
        }
        else{
            printf("%s[CONSUMER#%d] Get: %s\n", BLUE_COLOR, id, messageText);
        }
    }
}

int pleaseQuit = 0;

void sigintHandler(int sig) {
    pleaseQuit = 1;
    signal(sig, sigintHandler);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "%s\nUsage: %s nProducers nConsumers\n", ERROR_COLOR, argv[0]);
        pthread_exit((void *)0);
    }
    nProducers = atoi(argv[1]);
    nConsumers = atoi(argv[2]);

    threads = (pthread_t*)malloc(sizeof(pthread_t)*(nConsumers + nProducers));

    signal(SIGINT, sigintHandler);

    mymsginit(&queue, MESSAGES_LIMIT);
    printf("%s[PARENT] Init\n", PARENT_COLOR);

    for(int i = 0; i < nProducers; i++){
        if(pthread_create(&threads[i], NULL, producer, NULL)){
            throwAndExit("pthread_create()");
        }
    }

    for(int i = nProducers; i < nProducers + nConsumers; i++){
        if(pthread_create(&threads[i], NULL, consumer, NULL)){
            throwAndExit("pthread_create()");
        }
    }

    while (!pleaseQuit) pause();

    mymsgqdrop(&queue);
    printf("%s[PARENT] Drop\n", PARENT_COLOR);
    for(int i = 0; i < nProducers + nConsumers; i++){
        if(pthread_join(threads[i], NULL)){
            throwAndExit("pthread_join()");
        }
    }
    mymsgdestroy(&queue);
    printf("%s[PARENT] Destroy\n", PARENT_COLOR);

    free(threads);
    pthread_exit((void*)0);
}
