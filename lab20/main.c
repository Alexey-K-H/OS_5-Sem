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
    pthread_rwlock_t rwlock;
} Node;

Node* list;
int listSize = 0;
int finish = 0;

void destroyRwlock(pthread_rwlock_t* rwlock_param){
    pthread_rwlock_destroy(rwlock_param);
}

void freeList(Node* head) {
    if (head == NULL) {
        return;
    }

    Node* tmpNode;
    for (Node* iter = head; iter; iter = tmpNode) {
        destroyRwlock(&(iter->rwlock));
        tmpNode = iter->next;
        free(iter);
    }
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

    pthread_rwlock_init(&(node->rwlock), NULL);
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

    wLockRwlock(&(head->rwlock));
    newNode->next = head->next;
    head->next = newNode;
    listSize++;

    unlockRwlock(&(head->rwlock));
}

void printList(Node* head){
    if(head == NULL){
        return;
    }

    printf("\n\n------------------LIST:\n");
    for (Node* iter = head->next; iter; iter = iter->next) {
        rLockRwlock(&(iter->rwlock));
        printf("%s\n", iter->string);
        unlockRwlock(&(iter->rwlock));
    }
    printf("----------------------:\n");
}

Node* initList(){
    Node* head = malloc(sizeof(Node));
    head->string = NULL;
    head->next = NULL;
    pthread_rwlock_init(&(head->rwlock), NULL);
    return head;
}

int isEmptyString(char* buf){
    if(!strcmp(EMPTY_STRING, buf)){
        printList(list);
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

    while (finish != 1){
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
                pushFront(list, buf);
            }
            else{
                printList(list);
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

        Node* prev;
        Node* curr;
        for(Node* currNode = list->next; currNode->next; currNode = currNode->next){
            prev = list;
            curr = currNode;
            for(Node* nextNode = currNode->next; nextNode; nextNode = nextNode->next){
                wLockRwlock(&(prev->rwlock));
                wLockRwlock(&(curr->rwlock));
                wLockRwlock(&(nextNode->rwlock));

                if(compare(nextNode->string, currNode->string) < 0){
                    swap(&(nextNode->string), &(currNode->string));

                    unlockRwlock(&(nextNode->rwlock));
                    unlockRwlock(&(curr->rwlock));
                    unlockRwlock(&(prev->rwlock));

                    pthread_cond_t innerCond = PTHREAD_COND_INITIALIZER;
                    struct timespec innerDelay;
                    clock_gettime(CLOCK_REALTIME, &innerDelay);
                    innerDelay.tv_sec += 1;
                    while(pthread_cond_timedwait(&innerCond, &waitMutex, &innerDelay) != ETIMEDOUT);
                    pthread_cond_destroy(&innerCond);

                    wLockRwlock(&(prev->rwlock));
                    wLockRwlock(&(curr->rwlock));
                    wLockRwlock(&(nextNode->rwlock));
                }

                unlockRwlock(&(nextNode->rwlock));
                unlockRwlock(&(curr->rwlock));
                unlockRwlock(&(prev->rwlock));

                if(finish){
                    unlockMutex(&waitMutex);
                    pthread_mutex_destroy(&waitMutex);
                    pthread_exit(data);
                }

                curr = curr->next;
                prev = prev->next;
            }
        }
    }
}

void createThreads(int count){
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
