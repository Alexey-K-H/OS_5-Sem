#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define MAX_STRING_SIZE 80

typedef struct node {
    char str[MAX_STRING_SIZE];
    struct node *next;
    pthread_mutex_t nodeMutex;
} node;

node *head;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t listMutex = PTHREAD_MUTEX_INITIALIZER;
int flag = 0;

void push(const char *new_data);
void printList(node *);
void notifySortListener(int sign);
void bubbleStrSort(node *list);
void freeList(node *listStr);
void *sortListener(void *args);

int main() {
    pthread_t pthread;
    head = (node *) calloc(1, sizeof(node));
    pthread_mutex_init(&(head->nodeMutex), NULL);

    char buffer[MAX_STRING_SIZE];

    if (pthread_create(&pthread, NULL, sortListener, NULL) != 0) {
        printf("ERROR can't create thread");
        pthread_exit((void *) 0);
    }

    while (flag == 0) {
        int count = read(0, buffer, MAX_STRING_SIZE - 1);

        if (strcmp(buffer, "\n") == 0) {
            pthread_mutex_lock(&listMutex);
            printList(head);
            pthread_mutex_unlock(&listMutex);
            memset(buffer, 0, MAX_STRING_SIZE);
            continue;
        }
        if (buffer[count - 1] == '\n') {
            buffer[count - 1] = 0;
        }
        pthread_mutex_lock(&listMutex);
        push(buffer);
        pthread_mutex_unlock(&listMutex);
        memset(buffer, 0, MAX_STRING_SIZE);
    }
    freeList(head);
    pthread_exit((void *) 0);
}

void printList(node *list) {
    node *cur, *prev;
    prev = list;
    pthread_mutex_lock(&(prev->nodeMutex));

    int index = 0;
    if (list != NULL) {
        cur = list->next;
        while (cur != NULL) {
            pthread_mutex_lock(&(cur->nodeMutex));
            if(strcmp(cur->str, "") != 0){
                printf("Node[%d]:", index);
                index++;
                printf("%s\n", cur->str);
            }
            pthread_mutex_unlock(&(cur->nodeMutex));
            cur = cur->next;
        }
    }
    pthread_mutex_unlock(&(prev->nodeMutex));
}

void push(const char *new_data) {
    node *new_node = (node *) calloc(1, sizeof(node));
    strcpy(new_node->str, new_data);
    pthread_mutex_init(&(new_node->nodeMutex), NULL);

    pthread_mutex_lock(&(head->nodeMutex));
    node* tmp = head->next;
    new_node->next = tmp;
    head->next = new_node;
    pthread_mutex_unlock(&(head->nodeMutex));
}

void notifySortListener(int sign) {

    if (sign == SIGALRM) {
        pthread_cond_signal(&cond);
    }

    if (sign == SIGINT) {
        flag = 1;
        signal(sign, SIG_IGN);
    }
}

void bubbleStrSort(node *list) {
    node *node, *nestedNode;

    if (list != NULL) {
        pthread_mutex_lock(&(list->nodeMutex));
        node = list->next;
        while (node != NULL) {
            pthread_mutex_lock(&(node->nodeMutex));
            nestedNode = node->next;

            while (nestedNode != NULL) {
                pthread_mutex_lock(&(nestedNode->nodeMutex));
                if (strcmp(node->str, nestedNode->str) > 0) {
                    char tmp[MAX_STRING_SIZE + 1] = {0};
                    strcpy(tmp, nestedNode->str);
                    strcpy(nestedNode->str, node->str);
                    strcpy(node->str, tmp);
                }
                pthread_mutex_unlock(&(nestedNode->nodeMutex));
                nestedNode = nestedNode->next;
            }
            pthread_mutex_unlock(&(node->nodeMutex));
            node = node->next;
        }
        pthread_mutex_unlock(&(list->nodeMutex));
    }
}

void freeList(node *listStr) {
    node *curNode;
    node *prev;
    curNode = listStr;
    if (listStr != NULL) {
        while (curNode != NULL) {
            prev=curNode;
            curNode = curNode->next;
            free(prev);
        }
    }
}

void *sortListener(void *args) {
    pthread_mutex_lock(&listMutex);
    while (flag == 0) {
        signal(SIGALRM, notifySortListener);
        alarm(5);
        pthread_cond_wait(&cond, &listMutex);
        bubbleStrSort(head);
    }
    pthread_mutex_unlock(&listMutex);

    pthread_exit((void *) 0);
}
