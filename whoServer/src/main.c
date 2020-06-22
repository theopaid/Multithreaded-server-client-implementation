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
#include <sys/select.h>

#include "../include/Interface.h"

#define MAXLINE 4096

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

workersInfoNode *headOfWorkers = NULL;
SA_IN workersAddr;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexForStdout = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexForSendingQuery = PTHREAD_MUTEX_INITIALIZER;
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
    int listenfd, listenfdQueries, connfd;
    SA_IN servaddr, servaddrQueries;
    uint8_t buff[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    // create all the threads that we need for future connections
    pthread_t thread_pool[numThreads];
    for (int i = 0; i < numThreads; i++)
    {
        pthread_create(&thread_pool[i], NULL, socketDistribution, NULL);
    }

    // allocate new socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perrorexit("socket creation");
    if ((listenfdQueries = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perrorexit("socket creation");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(statisticsPortNum);

    bzero(&servaddrQueries, sizeof(servaddrQueries));
    servaddrQueries.sin_family = AF_INET;
    servaddrQueries.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddrQueries.sin_port = htons(queryPortNum);

    if ((bind(listenfd, (SA *)&servaddr, sizeof(servaddr))) < 0)
        perrorexit("bind error");
    if ((bind(listenfdQueries, (SA *)&servaddrQueries, sizeof(servaddrQueries))) < 0)
        perrorexit("bind error");

    if ((listen(listenfd, 50)) < 0)
        perrorexit("listen error");
    if ((listen(listenfdQueries, 50)) < 0)
        perrorexit("listen error");

    // let's prepare for select()
    fd_set currentSockets, readySockets;
    FD_ZERO(&currentSockets);
    FD_SET(listenfd, &currentSockets);
    FD_SET(listenfdQueries, &currentSockets);

    while (1)
    { // wait until an incoming connection arrives
        printf("Waiting for a connection on ports: %d , %d\n", statisticsPortNum, queryPortNum);
        socklen_t slen = sizeof(workersAddr);

        // make a copy, because select() will destroy currentSocckets
        readySockets = currentSockets;
        if (select(FD_SETSIZE, &readySockets, NULL, NULL, NULL) < 0)
        {
            perrorexit("select error");
        }
        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &readySockets))
            { // there is a connection to accept
                if (getNodesCount() >= bufferSize)
                {
                    puts("[LOG] Refusing connection: Buffer is full :(");
                    continue;
                }
                if (i == listenfd)
                {
                    if ((connfd = accept(listenfd, (SA *)&workersAddr, &slen)) < 0)
                        perrorexit("accept failed");

                    int *ptrClient = malloc(sizeof(int));
                    *ptrClient = connfd;
                    pthread_mutex_lock(&mutex);
                    addToBuffer(ptrClient, 0); // 0 because it's a worker

                    pthread_cond_signal(&conditionVar);
                    pthread_mutex_unlock(&mutex);
                }
                else if (i == listenfdQueries)
                {
                    if ((connfd = accept(listenfdQueries, (SA *)NULL, NULL)) < 0)
                        perrorexit("accept failed");
                    puts("Connected!");

                    int *ptrClient = malloc(sizeof(int));
                    *ptrClient = connfd;
                    pthread_mutex_lock(&mutex);
                    addToBuffer(ptrClient, 1); // 1 because it's a query from whoClient
                    pthread_cond_signal(&conditionVar);
                    pthread_mutex_unlock(&mutex);
                }
            }
        }
    }

    return 0;
}

void *socketDistribution(void *arg)
{
    while (1)
    {
        struct socketInfo *incomingSocket;
        pthread_mutex_lock(&mutex);
        if ((incomingSocket = removeFromBuffer()) == NULL)
        { // we should only wait if the buffer is empty
            pthread_cond_wait(&conditionVar, &mutex);

            // get it when one is finally available
            incomingSocket = removeFromBuffer();
        }
        pthread_mutex_unlock(&mutex);
        if (incomingSocket != NULL)
        {
            if (incomingSocket->typeOfConnection == 0) // worker
                handleConnectionForWorker(incomingSocket->socketfd);
            else if (incomingSocket->typeOfConnection == 1) // query
                handleConnectionForQuery(incomingSocket->socketfd);
            free(incomingSocket);
        }
    }
}

void handleConnectionForWorker(int *ptrClient)
{
    int clientSocket = *ptrClient, n;
    free(ptrClient); // we don't need it anymore

    uint8_t buff[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    // zero out the receiving buffer 1st to make sure it ends up null terminated
    memset(recvline, 0, MAXLINE);

    // read worker's port
    int tmp, workerPort;
    //SA_IN *pV4Addr = (SA_IN *) &clie
    if ((n = read(clientSocket, &tmp, sizeof(tmp))) > 0)
        workerPort = ntohl(tmp);
    else
        perrorexit("read error");
    pthread_mutex_lock(&mutexForStdout);
    printf("[LOG] Connected Worker's Port: %d\n", workerPort);
    addWorkerToList(&headOfWorkers, workerPort);

    // read worker's stats
    printf("===== Worker's statistics below ======\n");
    while ((n = read(clientSocket, recvline, MAXLINE - 1)) > 0)
    {
        printf("%s", recvline);
        if (recvline[n - 1] == '\0') // protocol: sign that message is over
            break;

        memset(recvline, 0, MAXLINE);
    }
    pthread_mutex_unlock(&mutexForStdout);
    // can't read negative bytes
    if (n < 0)
        perrorexit("read error");

    // send message to client
    snprintf((char *)buff, sizeof(buff), "Read statistics from worker with port: %d\n\0", workerPort);
    int sendbytes = strlen((char *)buff);
    if (write(clientSocket, buff, sendbytes) != sendbytes)
        perrorexit("write error");
    close(clientSocket);
}

void handleConnectionForQuery(int *ptrClient)
{
    int clientSocket = *ptrClient, n;
    free(ptrClient); // we don't need it anymore

    uint8_t buff[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    // zero out the receiving buffer 1st to make sure it ends up null terminated
    memset(recvline, 0, MAXLINE);

    // read client's query
    while ((n = read(clientSocket, recvline, MAXLINE - 1)) > 0)
    {
        if (recvline[0] == '.')
        { // if a command is read
            sendQueryToWorkers(recvline);
        }
        if (recvline[n - 1] == '\0')
        { // protocol: sign that message is over
            break;
        }

        memset(recvline, 0, MAXLINE);
    }
    // can't read negative bytes
    if (n < 0)
        perrorexit("read error");
    OverAndOut(&clientSocket);
}

void OverAndOut(int *sockfd)
{
    uint8_t sendline[MAXLINE + 1];
    memset(sendline, 0, MAXLINE);
    sprintf(sendline, "\0");
    if (write(*sockfd, sendline, 1) != 1)
        perrorexit("write error");
}

void sendQueryToWorkers(uint8_t *sendline)
{
    uint8_t recvline[MAXLINE + 1];
    memset(recvline, 0, MAXLINE);
    int n, workersCount = 0, sockfd;

    // let's prepare for select()
    // fd_set currentWorkerSockets, readyWorkerSockets;
    // FD_ZERO(&currentWorkerSockets);

    printf("Sending to workers: %s\n", sendline);
    workersInfoNode *current = headOfWorkers;
    while (current != NULL)
    {
        workersCount++;
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            perrorexit("socket creation");
        //const int optionval = 1;
        //setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optionval, sizeof(optionval));
        //FD_SET(sockfd, &currentWorkerSockets);

        SA_IN workeraddr;
        bzero(&workeraddr, sizeof(workeraddr));
        workeraddr.sin_family = AF_INET;
        workeraddr.sin_port = htons(current->port);
        workeraddr.sin_addr = workersAddr.sin_addr;

        if (connect(sockfd, (SA *)&workeraddr, sizeof(workeraddr)) < 0)
            perrorexit("socket connection failed");
        // printf("sockfd: %d\n", sockfd);
        //FD_SET(sockfd, &currentWorkerSockets);

        if (write(sockfd, sendline, strlen(sendline)) != strlen(sendline))
            perrorexit("write error");
        OverAndOut(&sockfd);

        while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0)
        {
            if (recvline[n - 1] == '\0')
            { // protocol: sign that message is over
                printf("[LOG] Command: %s , read by worker(%d)\n", sendline, current->port);
                //close(sockfd);
                break;
            }
        }

        // let's get the answer back
        memset(recvline, 0, MAXLINE);
        printf("Worker's Answer: ");
        while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0)
        {
            if (recvline[0] != '\0')
            { // if it's a regular message
                printf("%s", recvline);
            }
            if (recvline[n - 1] == '\0')
            { // protocol: sign that message is over
                close(sockfd);
                break;
            }

            memset(recvline, 0, MAXLINE);
        }
        if (n < 0)
            perrorexit("read error");
        puts(""); // newline

        current = current->next;
    }

    //let's get the answers back
    // while(workersCount--) {
    //     memset(recvline, 0, MAXLINE);
    //     puts("Waiting to get the answers back from the Workers...");

    //     // make a copy, because select() will destroy currentSocckets
    //     FD_ZERO(&readyWorkerSockets);
    //     readyWorkerSockets = currentWorkerSockets;
    //     if (select(FD_SETSIZE, &readyWorkerSockets, NULL, NULL, NULL) < 0)
    //     {
    //         perrorexit("select error");
    //     }
    //     for (int i = 0; i < FD_SETSIZE; i++)
    //     {
    //         if (FD_ISSET(i, &readyWorkerSockets))
    //         { // there is a connection to accept
    //             //FD_CLR(i, &currentWorkerSockets);
    //             printf("fd: %d\n", i);
    //             printf("Worker's Answer: ");
    //             while ((n = read(i, recvline, MAXLINE - 1)) > 0)
    //             {
    //                 printf("ok:%s", recvline);
    //                 if(recvline[0] != '\0') { // if it's a regular message
    //                     printf("%s", recvline);
    //                 }
    //                 if (recvline[n - 1] == '\0')
    //                 { // protocol: sign that message is over
    //                     break;
    //                 }

    //                 memset(recvline, 0, MAXLINE);
    //             }
    //             if (n < 0)
    //                 perrorexit("read error");
    //             puts(""); // newline
    //         }
    //     }
    // }
}