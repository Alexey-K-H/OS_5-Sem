#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILO];
pthread_mutex_t lforks;
pthread_t phils[PHILO];
void *philosopher(void *num);
int food_on_table();
void get_fork(int, int, char*);
void down_forks(int, int);
pthread_mutex_t foodlock;

int main(int argc, char** argv){
    int i;
    int status;

    pthread_mutex_init(&foodlock, NULL);
    pthread_mutex_init(&lforks, NULL);

    for(i = 0; i < PHILO; i++){
        pthread_mutex_init(&forks[i], NULL);
    }

    for(i = 0; i < PHILO; i++){
        status = pthread_create(&phils[i], NULL, philosopher, (void *)i);
        if(status != 0){
            printf("Failed to create thread#%d\n", i);
            return 0;
        }
    }

    for(i = 0; i < PHILO; i++){
        status = pthread_join(phils[i], NULL);
        if(status != 0){
            printf("Failed to join thread#%d\n", i);
            return 0;
        }
    }

    return 0;
}

void* philosopher(void* num){
    int id;
    int left_fork, right_fork, f;

    id = (int)num;
    printf("Philosopher#%d sit down to dinner.\n", id);
    right_fork = id % PHILO;
    left_fork = (id + 1) % PHILO;

    while (f = food_on_table()){
        printf("Philosopher#%d: get dinner %d,\n", id, f);
        pthread_mutex_lock(&lforks);
        get_fork(id, left_fork, "left ");
        get_fork(id, right_fork, "right ");
        pthread_mutex_unlock(&lforks);
        printf("Philosopher#%d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }

    printf("Philosopher %d is done eating.\n", id);
    return NULL;
}

int food_on_table(){
    static int food = FOOD;
    int myfood;

    pthread_mutex_lock(&foodlock);
    if(food > 0){
        food--;
    }
    myfood = food;
    pthread_mutex_unlock(&foodlock);
    return myfood;
}

void get_fork(int phil, int fork, char *hand){
    pthread_mutex_lock(&forks[fork]);
    printf("Philosopher#%d: got %s fork %d\n", phil, hand, fork);
}

void down_forks(int f1, int f2){
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
}
