#include "Connection.h"

Connection *connectionCreate(int clientID, int backendFD, Connection **connections){
    Connection *newConnection;
    newConnection = malloc(sizeof(struct Connection));
    newConnection->clientFD = clientID;
    newConnection->backendFD = backendFD;
    newConnection->clientToBackendBytesCount = 0;
    newConnection->backendToClientBytesCount = 0;
    newConnection->lastUpdate = time(&(newConnection->lastUpdate));
    newConnection->id = backendFD;
    newConnection->isBroken = 0;
    newConnection->prev = NULL;
    newConnection->next = (*connections);
    if((*connections)){
        (*connections)->prev = newConnection;
    }
    *connections = newConnection;
    printf("Create connection#%d\n", newConnection->id);
    return newConnection;
}

void connectionDrop(Connection *connection, Connection **connections){
    if(connection == (*connections)){
        (*connections) = connection->next;
    }
    else{
        connection->prev->next = connection->next;
    }
    if(connection->next){
        connection->next->prev = connection->prev;
    }
    printf("Dropped connection #%d\n", connection->id);
    close(connection->clientFD);
    close(connection->backendFD);
    free(connection);
}

