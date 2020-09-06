#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t mutex[3];
int ready = 0;

void print_message(void* str){
    int k = 1;

    if(ready == 0){
        if(pthread_mutex_lock(&mutex[1]) != 0){
            printf("Error to block mutex#1\n");
        } else{
            printf("Block mutex#1 by child\n");
        }
        ready = 1;
        k = 0;
    }

    for(int i = 0; i < 10 * 3; i++){
        if(pthread_mutex_lock(&mutex[k]) != 0){
            printf("Error to block mutex#%d\n", k);
        } else{
            printf("Block mutex#%d by child\n", k);
        }

        k = (k + 1) % 3;
        if(pthread_mutex_unlock(&mutex[k]) != 0){
            printf("Error to unlock mutex#%d\n", k);
        }
        else{
            printf("Unlock mutex#%d by child\n", k);
        }

        if(k == 2){
            printf("%s's string\n", (char*)str);
        }

        k = (k + 1) % 3;
    }
    if(pthread_mutex_unlock(&mutex[2]) != 0){
        printf("Error to unlock mutex#2\n");
    } else{
        printf("Unlock mutex#2 by child\n");
    }
}

int main(int argc, char** argv){
    pthread_t pthread;
    pthread_mutexattr_t mutexattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);

    for(int i = 0; i < 3; i++){
        pthread_mutex_init(&mutex[i], &mutexattr);
    }

    if(pthread_mutex_lock(&mutex[2]) != 0){
        printf("Error to block mutex#2\n");
    }
    else{
        printf("Block mutex#2 by parent\n");
    }

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
