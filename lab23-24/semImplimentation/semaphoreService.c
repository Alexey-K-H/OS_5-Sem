#include "../ConsoleColors.h"
#include <semaphore.h>

void semTryInit(sem_t *sem, int shared, int initialValue){
    if(sem_init(sem, shared, initialValue)){
        throwAndExit("sem_init()");
    }
}
void semTryPost(sem_t *sem){
    if(sem_post(sem)){
        throwAndExit("sem_post()");
    }
}
void semTryWait(sem_t *sem){
    if(sem_wait(sem)){
        throwAndExit("sem_wait()");
    }
}
void semTryDestroy(sem_t *sem){
    if(sem_destroy(sem)){
        throwAndExit("sem_close()");
    }
}

