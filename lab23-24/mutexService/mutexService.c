#include "../ConsoleColors.h"
#include <pthread.h>

void mutexTryInit(pthread_mutex_t *mutex){
    pthread_mutexattr_t attrs;
    if(pthread_mutexattr_init(&attrs)){
        throwAndExit("pthread_mutexattr_init()");
    }
    if(pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK)){
        throwAndExit("pthread_mutexattr_settype()");
    }
    if(pthread_mutex_init(mutex, &attrs)){
        throwAndExit("pthread_mutex_init()");
    }
}
void mutexTryLock(pthread_mutex_t *mutex){
    if(pthread_mutex_lock(mutex)){
        throwAndExit("pthread_mutex_lock()");
    }
}
void mutexTryUnlock(pthread_mutex_t *mutex){
    if(pthread_mutex_unlock(mutex)){
        throwAndExit("pthread_mutex_unlock()");
    }
}
void mutexTryDestroy(pthread_mutex_t *mutex){
    if(pthread_mutex_destroy(mutex)){
        throwAndExit("pthread_mutex_destroy");
    }
}

