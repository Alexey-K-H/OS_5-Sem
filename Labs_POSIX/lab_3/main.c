#include <stdio.h>
#include <pthread.h>

void* func();

int main(){
    pthread_t  pthread[4];
    int i;
    char *str1[2] = {"A", "1"};
    char *str2[2] = {"B", "2"};
    char *str3[2] = {"C", "3"};
    char *str4[2] = {"D", "4"};

    if(pthread_create(&pthread[0], NULL, func, str1) != 0){
        return 1;
    }

    if(pthread_create(&pthread[1], NULL, func, str2) != 0){
        return 1;
    }

    if(pthread_create(&pthread[2], NULL, func, str3) != 0){
        return 1;
    }

    if(pthread_create(&pthread[3], NULL, func, str4) != 0){
        return 1;
    }

    for(i = 0; i < 4; i++){
        pthread_join(pthread[i], NULL);
    }

    return 0;
}

void* func(void* ptr){
    char** str;
    int i;
    str = (char **)ptr;

    for(i = 0; i < 2; i++){
        printf("Thread#%s's string:{%s};\n", str[1], str[i]);
    }
}

