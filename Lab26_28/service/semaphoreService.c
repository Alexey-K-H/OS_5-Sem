#include "console_colors.h"
#include <semaphore.h>

void semTryInit(sem_t *sem, int shared, int initialValue){
    if(sem_init(sem, shared, initialValue)){
        throw_and_exit("sem_init()");
    }
}
void semTryPost(sem_t *sem){
    if(sem_post(sem)){
        throw_and_exit("sem_post()");
    }
}
void semTryWait(sem_t *sem){
    if(sem_wait(sem)){
        throw_and_exit("sem_wait()");
    }
}
void semTryDestroy(sem_t *sem){
    if(sem_destroy(sem)){
        throw_and_exit("sem_close()");
    }
}

