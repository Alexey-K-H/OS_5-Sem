#ifndef LAB25_CONNECTION_H
#define LAB25_CONNECTION_H

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 16000

typedef struct Connection{
    unsigned id;
    struct Connection *prev, *next;
    int clientFD;
    int backendFD;
    char clientToBackendBytes[BUFFER_SIZE];
    char backendToClientBytes[BUFFER_SIZE];
    size_t clientToBackendBytesCount;
    size_t backendToClientBytesCount;
    int isBroken;
    time_t lastUpdate;
}Connection;

Connection *connectionCreate(int clientID, int backendFD, Connection **connections);
void connectionDrop(Connection *connection, Connection **connections);

#endif
