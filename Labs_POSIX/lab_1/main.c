#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void* func();

int main(){
        pthread_t pthread;
        int i;
        if(pthread_create(&pthread, NULL, func, NULL) != 0){
                printf("ERROR, cannot create pthread!\n");
                return 1;
        }

        for(i = 0; i < 10; i++){
                printf("%d)Parent string\n", i);
        }

        pthread_exit(0);
}

void* func(void* ptr){
    int i;
    for(i = 0; i < 10; i++){
        printf("%d)Child str\n", i);
    }
}

