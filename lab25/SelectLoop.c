#include "Connection.h"
#include "ConsoleColors.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define NO_UPDATE_LIMIT_SEC 5
#define SELECT_TIMEOUT_SEC 100

void timeOrExpect(time_t *timer){
    if(-1 == time(timer)){
       throwAndExit("time()");
    }
}

void updateMaxFdOrExcept(int fd, int *maxFd){
    if(fd + 1 >= FD_SETSIZE){
        fprintf(stderr, "%sFailed to update max_fd: fd > FD_SETSIZE\n", RED_COLOR);
        exit(EXIT_FAILURE);
    }
    if(fd > *maxFd){
        *maxFd = fd;
    }
}

void selectLoop(Connection **connections, int frontendFd, void(*onClientConnect)()){
    fd_set readFds, writeFds;
    int ready;
    time_t timeNow;
    struct timeval timeout;
    timeout.tv_sec = SELECT_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    while (1){
        int maxFd = 0;
        FD_ZERO(&readFds);
        FD_ZERO(&writeFds);
        FD_SET(frontendFd, &readFds);

        (updateMaxFdOrExcept(frontendFd, &maxFd));

        for(Connection *con = *connections; con; con = con->next){
            timeOrExpect(&timeNow);

            if(con->isBroken){
                connectionDrop(con, connections);
            }
            else if(timeNow - con->lastUpdate > NO_UPDATE_LIMIT_SEC){
                connectionDrop(con, connections);
            }
            else{
                updateMaxFdOrExcept(con->clientFD, &maxFd);
                updateMaxFdOrExcept(con->backendFD, &maxFd);
                if(con->clientToBackendBytesCount){
                    FD_SET(con->backendFD, &writeFds);
                }else{
                    FD_SET(con->clientFD, &readFds);
                }

                if(con->backendToClientBytesCount){
                    FD_SET(con->clientFD, &writeFds);
                }
                else{
                    FD_SET(con->backendFD, &readFds);
                }
            }
        }

        ready = select(maxFd + 1, &readFds, &writeFds, NULL, &timeout);
        if(-1 == ready){
            throwAndExit("select");
        }
        else if(!ready){
            continue;
        }

        for(Connection *con = *connections; con; con = con->next){
            int connectionIsUpdated = 0;

            if(FD_ISSET(con->clientFD, &readFds)){
                int bytesReceived = recv(con->clientFD, con->clientToBackendBytes, sizeof(con->clientToBackendBytes), 0);
                con->clientToBackendBytesCount = bytesReceived;
                if(bytesReceived){
                    connectionIsUpdated = 1;
                }
            }

            if(FD_ISSET(con->backendFD, &writeFds)){
                if(-1 == send(con->backendFD, con->clientToBackendBytes, con->clientToBackendBytesCount, 0)){
                    con->isBroken = 1;
                    continue;
                }
                con->clientToBackendBytesCount = 0;
                connectionIsUpdated = 1;
            }

            if(FD_ISSET(con->backendFD, &readFds)){
                int bytesReceived = recv(con->backendFD, con->backendToClientBytes, sizeof(con->backendToClientBytes), 0);
                con->backendToClientBytesCount = bytesReceived;
                if(bytesReceived){
                    connectionIsUpdated = 1;
                }
            }

            if(FD_ISSET(con->clientFD, &writeFds)){
                if(-1 == send(con->clientFD, con->backendToClientBytes, con->backendToClientBytesCount, 0)){
                    con->isBroken = 1;
                    continue;
                }
                con->backendToClientBytesCount = 0;
                connectionIsUpdated = 1;
            }
            if(connectionIsUpdated){
                timeOrExpect(&con->lastUpdate);
            }
        }

        if(FD_ISSET(frontendFd, &readFds)){
            onClientConnect();
        }
    }
}
