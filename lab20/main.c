#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BUF_SIZE 80
#define EMPTY_STRING "\n"

typedef struct Node {
    char* string;
    struct Node* next;
    pthread_rwlock_t rwlock;
} Node;

Node* list;
int listSize = 0;
int finish = 0;
pthread_mutex_t sleepMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sleepCond = PTHREAD_COND_INITIALIZER;

void notifySortListener(int sign) {
    if (sign == SIGALRM) {
        pthread_cond_signal(&sleepCond);
    }

    if (sign == SIGINT) {
        finish = 1;
        signal(sign, SIG_IGN);
    }
}

void freeList(Node* head) {
    if (head == NULL) {
        return;
    }

    Node* tmpNode;
    for (Node* iter = head; iter; iter = tmpNode) {
        pthread_rwlock_destroy(&(iter->rwlock));
        tmpNode = iter->next;
        free(iter);
    }
}

void ExitFailure(char* errorMsg){
    perror(errorMsg);
    freeList(list);
    pthread_exit((void*)0);
}

void destroyRwlock(pthread_rwlock_t* rwlock_param){
    pthread_rwlock_destroy(rwlock_param);
}

void rLockRwlock(pthread_rwlock_t* rwlock_param){
    if(pthread_rwlock_rdlock(rwlock_param)){
        ExitFailure("Error locking rwlock_param for read");
    }
}

void wLockRwlock(pthread_rwlock_t* rwlock_param){
    if(pthread_rwlock_wrlock(rwlock_param)){
        ExitFailure("Error locking rwlock_param for write");
    }
}

void unlockRwlock(pthread_rwlock_t* rwlock_param){
    if(pthread_rwlock_unlock(rwlock_param)){
        ExitFailure("Error unlocking rwlock_param");
    }
}

void lockMutex(pthread_mutex_t* mutex){
    if(pthread_mutex_lock(mutex)){
        ExitFailure("Error locking mutex");
    }
}

void unlockMutex(pthread_mutex_t* mutex){
    if(pthread_mutex_unlock(mutex)){
        ExitFailure("Error unlocking mutex");
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

    //lockMutex(&(head->mutex));
    wLockRwlock(&(head->rwlock));
    newNode->next = head->next;
    head->next = newNode;
    listSize++;

    unlockRwlock(&(head->rwlock));
    //unlockMutex(&(head->mutex));
}

void printList(Node* head){
    if(head == NULL){
        return;
    }
    //lockMutex(&(head->mutex));
    rLockRwlock(&(head->rwlock));
    Node *prev = head;
    printf("\n\n------------------LIST:\n");
    for (Node* iter = head->next; iter; iter = iter->next) {
        //lockMutex(&(iter->mutex));
        rLockRwlock(&(iter->rwlock));
        //unlockMutex(&(prev->mutex));
        unlockRwlock(&(prev->rwlock));
        printf("%s\n", iter->string);
        prev = iter;
    }
    //unlockMutex(&(prev->mutex));
    unlockRwlock(&(prev->rwlock));
    printf("----------------------:\n");

}

Node* initList(){
    Node* head = malloc(sizeof(Node));
    head->string = NULL;
    head->next = NULL;
    //pthread_mutex_init(&(head->mutex), NULL);
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

void getStrings(){
    char buf[BUF_SIZE + 1];
    list = initList();

    while (finish != 1){
        if(fgets(buf, BUF_SIZE + 1, stdin)==NULL){
            continue;
        }
        if(!isEmptyString(buf)){
            pushFront(list, buf);
        }
        fflush(stdout);
    }
}

void swap(char** left, char** right){
    char* tmp = *left;
    *left = *right;
    *right = tmp;
}

int compare(char* left, char* right){
    int leftLen = strlen(left), rightLen = strlen(right);

    int minLength = (leftLen > rightLen) ? rightLen : leftLen;
    int maxLength = (leftLen < rightLen) ? rightLen : leftLen;

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
    lockMutex(&sleepMutex);
    while (1){
        if(finish){
            unlockMutex(&sleepMutex);
            pthread_exit((void*)0);
        }

        signal(SIGALRM, notifySortListener);
        alarm(5);
        pthread_cond_wait(&sleepCond, &sleepMutex);

        Node *prev;
        int i = 0, j = 0;
        for(Node* node = list->next; node; node = node->next, ++i, j = 0){
            prev = list;
            //lockMutex(&(list->mutex));
            wLockRwlock(&(list->rwlock));
            //lockMutex(&(list->next->mutex));
            wLockRwlock(&(list->next->rwlock));
            for(Node* innerNode = list->next; innerNode->next; innerNode = innerNode->next, ++j){
                //lockMutex(&(innerNode->next->mutex));
                wLockRwlock(&(innerNode->next->rwlock));
                if(compare(innerNode->next->string, innerNode->string) < 0){
                    swap(&(innerNode->next->string), &(innerNode->string));
                }
                //unlockMutex(&(prev->mutex));
                unlockRwlock(&(prev->rwlock));
                prev = innerNode;
            }
            //unlockMutex(&(prev->mutex));
            unlockRwlock(&(prev->rwlock));
            //unlockMutex(&(prev->next->mutex));
            unlockRwlock(&(prev->next->rwlock));
        }

    }
}

void createThread(){
    pthread_t threadId, threadId2;

    if(pthread_create(&threadId, NULL, sort, NULL)){
        ExitFailure("Error creating thread");
    }
    /*if(pthread_create(&threadId2, NULL, sort, NULL)){
	ExitFailure("Error creating second thread");
    }*/

    getStrings();

    if (pthread_join(threadId, NULL)){
        ExitFailure("Error waiting thread");
    }

    freeList(list);
}

int main(){
    createThread();
    pthread_exit((void*)0);
}
