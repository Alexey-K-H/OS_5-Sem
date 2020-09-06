#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t mutex[3];
int ready =  0;

void print_message(void* str){
    int k = 1;

    if(ready == 0){
        pthread_mutex_lock(&mutex[1]);
        ready = 1;
        k = 0;
    }

    for(int i = 0; i < 10 * 3; i++){
        if(pthread_mutex_lock(&mutex[k]) != 0){
            printf("Error to block mutex#%d\n", k);
        }

        k = (k + 1) % 3;
        if(pthread_mutex_unlock(&mutex[k]) != 0){
            printf("Error to unlock mutex#%d", k);
        }

        if(k == 2){
            printf("%s's string\n", (char*)str);
        }

        k = (k + 1) % 3;
    }
    pthread_mutex_unlock(&mutex[2]);
}

int main(int argc, char** argv){
    pthread_t pthread;
    pthread_mutexattr_t mutexattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);

    for(int i = 0; i < 3; i++){
        pthread_mutex_init(&mutex[i], &mutexattr);
    }

    pthread_mutex_lock(&mutex[2]);

    pthread_create(&pthread, NULL, print_message, (void*)"Child thread string");

    while(0 == ready){
        sched_yield();
    }

    print_message("Parent thread string");

    for(int i = 0; i< 3; i++){
        pthread_mutex_destroy(&mutex[i]);
    }

    return 0;
}
