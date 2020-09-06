#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* func();

int main(){
    pthread_t pthread;
    int i;
    if(pthread_create(&pthread, NULL, func, NULL) != 0){
        printf("ERROR, cannot create pthread!\n");
        return 1;
    }

    pthread_join(pthread, NULL);

    for(i = 0; i < 10; i++){
        printf("%d)Parent's str\n", i);
        sleep(2);
    }

    return 0;
}

void* func(void* ptr){
    int i;
    for(i = 0; i < 10; i++){
        printf("%d)Child str\n", i);
        sleep(1);
    }
}

