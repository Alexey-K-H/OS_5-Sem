#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

sem_t detailA, detailB, detailC, module;
int finish = 0;

void quit_handler(int sign){
    signal(sign, SIG_IGN);
    if(sign == SIGINT){
        finish = 1;
    }
}

void* createA (void* arg){
    while(!finish){
        sleep(1);
        sem_post(&detailA);
        printf("Detail A created!\n");
    }

    pthread_exit(arg);
}

void* createB (void* arg){
    while(!finish){
        sleep(2);
        sem_post(&detailB);
        printf("Detail B created!\n");
    }

    pthread_exit(arg);
}

void* createC (void* arg){
    while(!finish){
        sleep(3);
        sem_post(&detailC);
        printf("Detail C created!\n");
    }

    pthread_exit(arg);
}

void* createModule (void* arg){
    while(!finish){
        sem_wait(&detailA);
        sem_wait(&detailB);
        sem_post(&module);
        printf("Module created!\n");
    }

    pthread_exit(arg);
}

void createWidget (){
    while(!finish){
        sem_wait(&module);
        sem_wait(&detailC);
        printf("Widget created!\n");
    }
}

int main(){
    pthread_t aThread;
    pthread_t bThread;
    pthread_t cThread;
    pthread_t moduleThread;

    signal(SIGINT, quit_handler);

    sem_init(&detailA, 0, 0);
    sem_init(&detailB, 0, 0);
    sem_init(&detailC, 0, 0);
    sem_init(&module, 0, 0);

    if(pthread_create(&aThread, NULL, createA, NULL)){
        printf("Error to create A thread!\n");
    }
    if(pthread_create(&bThread, NULL, createB, NULL)){
        printf("Error to create B thread!\n");
    }
    if(pthread_create(&cThread, NULL, createC, NULL)){
        printf("Error to create C thread!\n");
    }
    if(pthread_create(&moduleThread, NULL, createModule, NULL)){
        printf("Error to create Module thread!\n");
    }

    createWidget();

    if(pthread_join(aThread, NULL)){
        printf("Error to join thread A!\n");
    }

    if(pthread_join(bThread, NULL)){
        printf("Error to join thread B!\n");
    }

    if(pthread_join(cThread, NULL)){
        printf("Error to join thread C!\n");
    }

    if(pthread_join(moduleThread, NULL)){
        printf("Error to join thread module!\n");
    }

    sem_destroy(&detailA);
    sem_destroy(&detailB);
    sem_destroy(&detailC);
    sem_destroy(&module);

    pthread_exit((void*)0);
}
