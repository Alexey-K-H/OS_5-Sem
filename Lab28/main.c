#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define DEFAULT_PORT 80
#define BUFFER_SIZE 128
#define DNS_TIMEOUT 10

int finished = 0;

struct Buffer
{
    char * buffers[10];
    size_t ends[10];

    sem_t full;
    sem_t empty;

    int socket;
};

void * writeThread(void * p)
{
    struct Buffer * parameter = (struct Buffer *)p;

    int id = 0;
    int linesWritten = 0;

    while (!finished || parameter->ends[id] != 0)
    {
        sem_wait(&(parameter->full));

        char * buffer = parameter->buffers[id];
        size_t localEnd = parameter->ends[id];

        for (size_t localStart = 0; localStart < localEnd;)
        {
            char * enter = strchr(buffer + localStart, '\n');

            size_t lineEnd;

            if (NULL == enter)
            {
                lineEnd = localEnd;
            }
            else
            {
                lineEnd = enter - buffer + 1;
            }

            int written = write(STDOUT_FILENO, buffer + localStart, lineEnd - localStart);

            if (-1 == written)
            {
                perror("write error");
                exit(EXIT_FAILURE);
            }
            else
            {
                localStart += written;

                if (localStart == lineEnd && NULL != enter)
                {
                    linesWritten++;
                }
            }

            if (linesWritten == 25)
            {
                char temp;

                printf("Press enter to scroll down\n");
                do {
                    if (-1 == read(STDIN_FILENO, &temp, 1)) {
                        perror("Cannot read from terminal");
                    }
                } while (temp != '\n');

                linesWritten = 0;
            }
        }

        memset(buffer, 0, localEnd);
        parameter->ends[id] = 0;

        id = (id + 1) % 10;

        sem_post(&(parameter->empty));
    }

    return NULL;
}

void * readThread(void * p)
{
    struct Buffer * parameter = (struct Buffer *)p;
    int socket = parameter->socket;

    int id = -1;

    while(!finished)
    {
        sem_wait(&(parameter->empty));

        id = (id + 1) % 10;

        char * buffer = parameter->buffers[id];

        int localEnd = 0;

        while (!finished && localEnd < BUFFER_SIZE)
        {
            int wasRead = recv(socket, buffer + localEnd, BUFFER_SIZE - localEnd, 0);

            switch (wasRead)
            {
                case -1:
                    perror("receive error");
                    exit(EXIT_FAILURE);
                case 0:
                    finished = 1;
                    break;
                default:
                    localEnd += wasRead;
                    break;
            }
        }

        parameter->ends[id] = localEnd;

        sem_post(&(parameter->full));
    }

    close(socket);
    return NULL;
}

void sigHandler(int signum) {
    fprintf(stderr, "Dns request timed out\n");
    exit(EXIT_FAILURE);
}

int makeConnection(const char *hostName) {
    signal(SIGALRM, sigHandler);
    alarm(DNS_TIMEOUT);
    struct hostent *hostInfo = gethostbyname(hostName);
    alarm(0);

    if (NULL == hostInfo) {
        fprintf(stderr, "Cannot get host by name\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in destinationAddress;

    destinationAddress.sin_family = AF_INET;
    destinationAddress.sin_port = htons(DEFAULT_PORT);
    memcpy(&destinationAddress.sin_addr, hostInfo->h_addr, hostInfo->h_length);

    int httpSocket = socket(AF_INET, SOCK_STREAM, 0);


    if (-1 == httpSocket) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(httpSocket, (struct sockaddr *) &destinationAddress, sizeof(destinationAddress))) {
        perror("Cannot connect");
        exit(EXIT_FAILURE);
    }

    return httpSocket;
}

void sendRequest(const char *path, const char *host, int httpSocket) {
    char pattern[] = "GET %s HTTP/1.0\nHost: %s\n\n\0";
    char *buffer = (char *) malloc(BUFFER_SIZE);

    if (NULL == buffer) {
        perror("Cannot allocate memory for buffer");
        exit(EXIT_FAILURE);
    }

    sprintf(buffer, pattern, path, host);

    if (-1 == send(httpSocket, buffer, strlen(buffer), 0)) {
        perror("Cannot send request");
        exit(EXIT_FAILURE);
    }

    free(buffer);
}

void checkCountArguments(int argc) {
    if (argc != 2) {
        perror("Wrong count of arguments");
        exit(EXIT_FAILURE);
    }
}

void createThread(pthread_t *thread, void *function(), void* parameter) {
    if (pthread_create(thread, NULL, function, parameter) != 0) {
        fprintf(stderr, "Cannot create thread");
        exit(EXIT_FAILURE);
    }
}

void initSemaphore(sem_t *sem, unsigned int value) {
    if (sem_init(sem, 0, value) < 0) {
        fprintf(stderr, "Cannot create semaphore");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    checkCountArguments(argc);

    char *url = argv[1];
    char *protocolEnd = strstr(url, "://");

    if (protocolEnd == NULL) {
        fprintf(stderr, "Incorrect input\n");
        exit(EXIT_FAILURE);
    }

    char *hostEnd = strchr(protocolEnd + 3, '/');
    size_t hostLength;
    char protocol[protocolEnd - url + 1];

    protocol[protocolEnd - url] = 0;
    strncpy(protocol, url, strlen(protocol));
    if (strcmp(protocol, "http") != 0 && strcmp(protocol, "https") != 0) {
        fprintf(stderr, "Unsupported protocol\n");
        exit(EXIT_FAILURE);
    }

    if (NULL == hostEnd) {
        hostLength = strlen(protocolEnd + 3);
    }
    else {
        hostLength = hostEnd - (protocolEnd + 3);
    }

    char *hostName = (char *) malloc(hostLength + 1);

    if (NULL == hostName) {
        perror("Cannot allocate memory for hostname");
        exit(EXIT_FAILURE);
    }

    strncpy(hostName, protocolEnd + 3, hostLength);
    hostName[hostLength] = 0;

    int httpSocket = makeConnection(hostName);
    if (hostEnd == NULL) {
        sendRequest("/", hostName, httpSocket);
    }
    else {
        sendRequest(hostEnd, hostName, httpSocket);
    }

    struct Buffer parameter;

    for(int i = 0; i < 10; i++){
        parameter.buffers[i] = (char*) malloc(BUFFER_SIZE);
        parameter.ends[i] = 0;
    }

    initSemaphore(&parameter.empty, 10);
    initSemaphore(&parameter.full, 0);

    parameter.socket = httpSocket;

    pthread_t thread;
    createThread(&thread, writeThread, &parameter);

    readThread(&parameter);
    pthread_join(thread, NULL);

    free(hostName);
    putchar('\n');
    for(int i = 0; i < 10; i++){
        free(parameter.buffers[i]);
    }

    return EXIT_SUCCESS;
}

