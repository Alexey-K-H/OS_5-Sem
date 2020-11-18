#include "Connection.h"
#include "ConsoleColors.h"
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int frontendFd;
int isDisposed = 0;
struct addrinfo *backendAi;
Connection *connections;

void selectLoop(Connection **connections, int frontendFd, void(*onClientConnect)());
void getAddrInfo(char *url, char *port, struct addrinfo *hints, struct addrinfo **res){
    int ret = getaddrinfo(url, port, hints, res);
    if(ret){
        fprintf(stderr, "%sFailed to gettaddrinfo: %s\n", RED_COLOR, gai_strerror(ret));
        exit(EXIT_FAILURE);
    }
}

int getSocketFd(struct addrinfo *ai, int(*bindOrConnect)(int, const struct sockaddr *, socklen_t)){
    while (1){
        if(!ai){
            throwAndExit("getSocketFd()");
        }
        int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if(-1 == fd){
            continue;
        }

        if(!bindOrConnect(fd, ai->ai_addr, ai->ai_addrlen)){
            return fd;
        }
        close(fd);
        ai = ai->ai_next;
    }
}

void onClientConnect(){
    int clientFd = accept(frontendFd, NULL, NULL);
    if(-1 == clientFd){
        throwAndExit("accept()");
    }
    int backendFd = getSocketFd(backendAi, connect);
    if(!connectionCreate(clientFd, backendFd, &connections)){
        throwAndExit("connectionCreate()");
    }
}

void atExit(){
    if(isDisposed){
        return;
    }

    isDisposed = 1;
    freeaddrinfo(backendAi);
    close(frontendFd);
    printf("%sStop listening.\n", GREEN_COLOR);
}

int main(int argc, char** argv) {
    if(argc != 4){
        fprintf(stderr, "%sUsage: %s [frontendPort] [backendUrl] [backendPort]\n", ERROR_COLOR, argv[0]);
        exit(EXIT_FAILURE);
    }

    atexit(atExit);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, atExit);
    signal(SIGTERM, atExit);

    char *frontendPort = argv[1];
    char *backendUrl = argv[2];
    char *backendPort = argv[3];

    connections = NULL;

    struct addrinfo hints, *frontendAi;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    getAddrInfo(backendUrl, backendPort, &hints, &backendAi);

    hints.ai_flags = AI_PASSIVE;
    getAddrInfo(NULL, frontendPort, &hints, &frontendAi);
    frontendFd = getSocketFd(frontendAi, bind);
    freeaddrinfo(frontendAi);

    if(listen(frontendFd, SOMAXCONN)){
        throwAndExit("listen()");
    }
    printf("%sStart listening\n", YELLOW_COLOR);
    selectLoop(&connections, frontendFd, onClientConnect);

    return 0;
}
