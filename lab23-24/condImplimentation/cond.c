#include "../ConsoleColors.h"
#include <pthread.h>

void condTryInit(pthread_cond_t *cond){
    if(pthread_cond_init(cond, NULL)){
        throwAndExit("pthread_cond_init()");
    }
}
void condTrySignal(pthread_cond_t *cond){
    if(pthread_cond_signal(cond)){
        throwAndExit("pthread_cond_signal()");
    }
}
void condTryBroadcast(pthread_cond_t *cond){
    if(pthread_cond_broadcast(cond)){
        throwAndExit("pthread_cond_broadcast()");
    }
}
void condTryWait(pthread_cond_t *cond, pthread_mutex_t *mutex){
    if(pthread_cond_wait(cond, mutex)){
        throwAndExit("pthread_cond_wait");
    }
}
void condTryDestroy(pthread_cond_t *cond){
    if(pthread_cond_destroy(cond)){
        throwAndExit("pthread_cond_destroy()");
    }
}
