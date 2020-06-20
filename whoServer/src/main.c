#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include "../include/Interface.h"

#define MAXLINE 4096

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conditionVar = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{

    // dirListNode *headDirList = dirListingToList("input_dir");

    if (!validArgs(argc, argv))
    {
        printf("The arguments given are invalid. Try again.\n");
        return -1;
    }

    int numThreads, bufferSize, queryPortNum, statisticsPortNum;
    getArgs(&numThreads, &bufferSize, &queryPortNum, &statisticsPortNum, argv);
    printf("%d %d %d %d\n", numThreads, bufferSize, queryPortNum, statisticsPortNum);
    int listenfd, connfd;
    SA_IN servaddr;
    uint8_t buff[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    // create all the threads that we need for future connections 
    pthread_t thread_pool[numThreads];
    for(int i = 0; i< numThreads; i++) {
        pthread_create(&thread_pool[i], NULL, socketDistribution, NULL);
    }

    // allocate new socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perrorexit("socket creation");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(statisticsPortNum);

    if ((bind(listenfd, (SA *)&servaddr, sizeof(servaddr))) < 0)
        perrorexit("bind error");

    if ((listen(listenfd, 50)) < 0)
        perrorexit("listen error");

    while (1)
    { // wait until an incoming connection arrives
        printf("Waiting for a connection on port: %d\n", statisticsPortNum);

        if(getNodesCount() >= bufferSize) {
            puts("Refusing connection: Buffer is full :(");
            continue;
        }

        if((connfd = accept(listenfd, (SA *)NULL, NULL)) < 0)
            perrorexit("accept failed");
        puts("Connected!");

        int *ptrClient = malloc(sizeof(int));
        *ptrClient = connfd;
        pthread_mutex_lock(&mutex);
        addToBuffer(ptrClient);
        pthread_cond_signal(&conditionVar);
        pthread_mutex_unlock(&mutex);

    }

    return 0;
}

void * socketDistribution(void *arg) {
    while (1)
    {
        int *ptrClient;
        pthread_mutex_lock(&mutex);
        if((ptrClient = removeFromBuffer()) == NULL) { // we should only wait if the buffer is empty
            pthread_cond_wait(&conditionVar, &mutex);

            ptrClient = removeFromBuffer();
        }
        pthread_mutex_unlock(&mutex);
        if (ptrClient != NULL) {
            handleConnection(ptrClient);
        }
    }
    
}

void handleConnection(int *ptrClient) {
    int clientSocket = *ptrClient, n;
    free(ptrClient); // we don't need it anymore

    uint8_t buff[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    // zero out the receiving buffer 1st to make sure it ends up null terminated
    memset(recvline, 0, MAXLINE);

    // read worker's port
    int tmp, workerPort;
    if((n = read(clientSocket, &tmp, sizeof(tmp))) > 0)
        workerPort = ntohl(tmp);
    else
        perrorexit("read error");
    printf("Worker's Port: %d\n", workerPort);

    // read worker's stats
    printf("===== Worker's statistics below ======\n");
    while ((n = read(clientSocket, recvline, MAXLINE - 1)) > 0)
    {
        printf("%s", recvline);
        if (recvline[n-1] == '\0') // protocol: sign that message is over
            break;

        memset(recvline, 0, MAXLINE);

    }
    // can't read negative bytes
    if (n < 0)
        perrorexit("read error");

    // send message to client
    snprintf((char *)buff, sizeof(buff), "Read stats from worker with port: %d\n\0", workerPort);
    write(clientSocket, (char *)buff, strlen((char *)buff));
    close(clientSocket);
}
