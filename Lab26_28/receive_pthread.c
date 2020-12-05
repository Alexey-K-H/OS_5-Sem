//
// Created by ninetail on 12/5/20.
//

#include "service/cond.h"
#include "service/console_colors.h"
#include "service/mutex.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFSIZE 8192
#define PAGE_SIZE 25

typedef struct {
    int socket_fd;
    char *buffer;
    int *buffer_bytes_count;
    int *is_socket_eof;
    pthread_mutex_t *mutex;
    pthread_cond_t *cond;
}Socket_Routine_Args;

void *socket_routine(void *args_raw){
    Socket_Routine_Args *args = (Socket_Routine_Args*)args_raw;

    while (!*args->is_socket_eof){
        mutex_try_lock(args->mutex);
        while (*args->buffer_bytes_count){
            cond_try_wait(args->cond, args->mutex);
        }

        int bytes_read = read(args->socket_fd, args->buffer, BUFSIZE);
        if(-1 == bytes_read){
            throw_and_exit("read");
        }
        else if(bytes_read == 0){
            *args->is_socket_eof = 1;
        }
        else{
            *args->buffer_bytes_count = bytes_read;
        }

        cond_try_signal(args->cond);
        mutex_try_unlock(args->mutex);
    }

    pthread_exit((void*)0);
}

void receiving_routine(int socket_fd){
    pthread_mutex_t mutex;
    mutex_try_init(&mutex);
    pthread_cond_t cond;
    cond_try_init(&cond);

    char buffer[BUFSIZE];
    int buffer_bytes_count = 0;
    int is_socket_eof = 0;
    int lines_left = PAGE_SIZE;

    Socket_Routine_Args args;
    args.socket_fd = socket_fd;
    args.buffer = buffer;
    args.buffer_bytes_count = &buffer_bytes_count;
    args.is_socket_eof = &is_socket_eof;
    args.mutex = &mutex;
    args.cond = &cond;

    pthread_t socket_thread;
    if(pthread_create(&socket_thread, NULL, socket_routine, &args)){
        throw_and_exit("pthread_create");
    }

    while (!is_socket_eof){
        mutex_try_lock(&mutex);
        while (!buffer_bytes_count && !is_socket_eof){
            cond_try_wait(&cond, &mutex);
        }

        while (buffer_bytes_count){
            int pos;
            for(pos = 0; pos < buffer_bytes_count; pos++){
                if(buffer[pos] == '\n'){
                    lines_left--;
                    if(!lines_left){
                        pos++;
                        break;
                    }
                }
            }

            if(write(STDOUT_FILENO, buffer, pos) < pos){
                throw_and_exit("write");
            }
            else{
                buffer_bytes_count -= pos;
            }

            if(buffer_bytes_count){
                memmove(buffer, buffer + pos, buffer_bytes_count);
            }

            if(!lines_left){
                printf("%sPress enter to scroll down.%s\n", YELLOW_COLOR, WHITE_COLOR);
                while (getchar() != '\n');
                lines_left = PAGE_SIZE;
            }
        }

        cond_try_signal(&cond);
        mutex_try_unlock(&mutex);
    }

    if(pthread_join(socket_thread, NULL)){
        throw_and_exit("pthread_join");
    }
}