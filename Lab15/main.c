#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

sem_t *sem1;
sem_t *sem2;

void *childThread(void *arg) {

    for (int i = 0; i < 10; ++i) {
        sem_wait(&sem1);
        printf("Child\n");
        sem_post(&sem2);
    }

    pthread_exit((void *) 0);
}

int main() {
    sem1 = sem_open("/sem1", O_CREAT | O_EXCL, S_IRWXU, 0);
    sem2 = sem_open("/sem2", O_CREAT | O_EXCL, S_IRWXU, 1);

    if (fork()) {
        for (int i = 0; i < 10; ++i) {
            sem_wait(sem2);
            printf("Parent\n");
            sem_post(sem1);
        }
    } else {
        for (int i = 0; i < 10; ++i) {
            sem_wait(sem1);
            printf("Child\n");
            sem_post(sem2);
        }
    }

    sem_unlink("/sem1");
    sem_unlink("/sem2");
    pthread_exit((void *) 0);
}
