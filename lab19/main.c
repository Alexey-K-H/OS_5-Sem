#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define BUF_SIZE 80
#define EMPTY_STRING "\n"
#define EXIT_STRING "exit\n"

typedef struct Node {
    char* string;
    struct Node* next;
} Node;

Node* list;
int listSize = 0;
int finish = 0;

pthread_rwlock_t rwlock;

void freeList(Node* head) {
    if (head == NULL) {
        return;
    }

    Node* tmpNode;
    for (Node* iter = head; iter; iter = tmpNode) {
        tmpNode = iter->next;
        free(iter);
    }
}

void destroyRwlock(){
    pthread_rwlock_destroy(&rwlock);
}

void rLockRwlock(pthread_rwlock_t* rwlock_param){
    if(pthread_rwlock_rdlock(rwlock_param)){
        printf("Error locking rwlock_param for read\n");
    }
}

void wLockRwlock(pthread_rwlock_t* rwlock_param){
    if(pthread_rwlock_wrlock(rwlock_param)){
        printf("Error locking rwlock_param for write\n");
    }
}

void unlockRwlock(pthread_rwlock_t* rwlock_param){
    if(pthread_rwlock_unlock(rwlock_param)){
        printf("Error unlocking rwlock_param\n");
    }
}

void lockMutex(pthread_mutex_t* mutex){
    if(pthread_mutex_lock(mutex)){
        printf("Error locking mutex\n");
    }
}

void unlockMutex(pthread_mutex_t* mutex){
    if(pthread_mutex_unlock(mutex)){
        printf("Error unlocking mutex\n");
    }
}

Node* initNode(char* str, size_t size) {
    Node* node = malloc(sizeof(Node));
    if (node == NULL) {
        free(node);
        printf("Allocation failed\n");
        return NULL;
    }

    node->next = NULL;
    node->string = malloc(size + 1);
    if(strcpy(node->string, str) == NULL){
        free(node->string);
        free(node);
        perror("Error creating node for input string");
        return NULL;
    }

    return node;
}

void pushFront(Node* head, char* str) {
    if(head == NULL){
        return;
    }

    Node* newNode = initNode(str, strlen(str));

    if (newNode == NULL) {
        return;
    }

    newNode->next = head->next;
    head->next = newNode;
    listSize++;
}

void printList(Node* head){
    if(head == NULL){
        return;
    }

    printf("\n\n------------------LIST:\n");
    for (Node* iter = head->next; iter; iter = iter->next) {
        printf("%s\n", iter->string);
    }

    printf("----------------------:\n");

}

Node* initList(){
    Node* head = malloc(sizeof(Node));
    head->string = NULL;
    head->next = NULL;
    return head;
}

int isEmptyString(char* buf){
    if(!strcmp(EMPTY_STRING, buf)){
        rLockRwlock(&rwlock);
        printList(list);
        unlockRwlock(&rwlock);
        return 1;
    }

    return 0;
}

int isExitString(char *buf){
    if(!strcmp(EXIT_STRING, buf)){
        return 1;
    }

    return 0;
}

void getStrings(){
    char buf[BUF_SIZE + 1];
    list = initList();

    while (1){
        if(fgets(buf, BUF_SIZE + 1, stdin)==NULL){
            continue;
        }
        if(isExitString(buf)){
            finish = 1;
            fflush(stdout);
            break;
        }
        else{

            if(!isEmptyString(buf)){
                wLockRwlock(&rwlock);
                pushFront(list, buf);
                unlockRwlock(&rwlock);
            }
            else{
                rLockRwlock(&rwlock);
                printList(list);
                unlockRwlock(&rwlock);
            }
            fflush(stdout);
        }
    }
}

void swap(char** left, char** right){
    char* tmp = *left;
    *left = *right;
    *right = tmp;
}

int compare(char* left, char* right){
    unsigned int leftLen = strlen(left), rightLen = strlen(right);

    unsigned int minLength = (leftLen > rightLen) ? rightLen : leftLen;
    unsigned int maxLength = (leftLen < rightLen) ? rightLen : leftLen;

    for(int i = 0; i < minLength; ++i){
        if(left[i] != right[i]){
            return 2 * (left[i] > right[i]) - 1;
        }
    }

    if (!(maxLength - minLength)){
        return 0;
    }

    return 2 * (maxLength == strlen(left)) - 1;
}

void* sort(void* data){
    pthread_mutex_t waitMutex = PTHREAD_MUTEX_INITIALIZER;
    lockMutex(&waitMutex);
    while (1){
        pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
        struct timespec delay;
        clock_gettime(CLOCK_REALTIME, &delay);
        delay.tv_sec += 5;
        while(pthread_cond_timedwait(&cond, &waitMutex, &delay) != ETIMEDOUT);
        pthread_cond_destroy(&cond);

        wLockRwlock(&rwlock);
        for(Node* node = list->next; node; node = node->next){
            for(Node* innerNode = list->next; innerNode->next; innerNode = innerNode->next){
                if(compare(innerNode->next->string, innerNode->string) < 0){
                    swap(&(innerNode->next->string), &(innerNode->string));
                }
                if(finish){
                    unlockRwlock(&rwlock);
                    unlockMutex(&waitMutex);
                    pthread_mutex_destroy(&waitMutex);
                    pthread_exit(data);
                }
            }
        }
        unlockRwlock(&rwlock);
    }
}

void createThreads(int count){
    pthread_rwlock_init(&rwlock, NULL);

    pthread_t* threads;
    threads = (pthread_t*)malloc(sizeof(pthread_t)*count);
    for(int i = 0; i < count; i++) {
        if(pthread_create(&threads[i], NULL, sort, NULL)){
            printf("Error creating thread\n");
        }
    }

    getStrings();

    printf("Waiting for sorting threads...\n");
    for(int i =0; i < count; i++){
        if (pthread_join(threads[i], NULL)){
            printf("Error waiting thread\n");
        }
    }

    destroyRwlock();
    printf("Free list...\n");
    freeList(list);
}

int main(int argc, char** argv){
    if(argc < 2){
        pthread_exit((void*)0);
    }

    int count = atoi(argv[1]);
    createThreads(count);
    pthread_exit((void*)0);
}
