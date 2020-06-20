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

#include "../include/Interface.h"

#define MAXLINE 4096
#define SA struct sockaddr

void distributeToWorkers(workersInfo *myWorkersInfo, int numOfWorkers, int bufferSize, char *input_dir, char *serverIP, int serverPort)
{

    pid_t childpid;

    dirListNode *headDirList = dirListingToList(input_dir);
    int numOfDirs = listNodeCounter(headDirList);
    int divRes = numOfDirs / numOfWorkers;
    int modRes = numOfDirs % numOfWorkers;

    int i, dirsForWorker;
    dirListNode *current = headDirList;
    for (i = 1; i <= numOfWorkers; i++)
    {
        dirsForWorker = divRes;
        if (modRes - i >= 0)
            dirsForWorker++;

        char fifoToWorker[20], fifoFromWorker[20];
        sprintf(fifoToWorker, "/tmp/Worker%dIn", i);
        sprintf(fifoFromWorker, "/tmp/Worker%dOut", i);
        if (mkfifo(fifoToWorker, 0666) == -1)
            perror("mkfifo1");
        if (mkfifo(fifoFromWorker, 0666) == -1)
            perror("mkfifo2");

        myWorkersInfo->workerPATHs[i - 1][0] = (char *)malloc(sizeof(char) * (strlen(fifoToWorker) + 1));
        myWorkersInfo->workerPATHs[i - 1][1] = (char *)malloc(sizeof(char) * (strlen(fifoFromWorker) + 1));

        strcpy(myWorkersInfo->workerPATHs[i - 1][0], fifoToWorker);
        strcpy(myWorkersInfo->workerPATHs[i - 1][1], fifoFromWorker);

        if ((childpid = fork()) <= 0)
        {

            if (childpid == -1)
            {
                perror("failed to fork");
                exit(-1);
            }
            int fdRead = open(fifoToWorker, O_RDWR | O_NONBLOCK);
            int fdWrite = open(fifoFromWorker, O_RDWR | O_NONBLOCK);

            int numOfDirs = 0, bytesToRead, n;
            char *dirName;

            while (read(fdRead, &numOfDirs, sizeof(int)) == -1)
            {
            }
            n = numOfDirs;
            char *dirNames[numOfDirs];
            while (n--)
            {
                bytesToRead = 0;
                while (read(fdRead, &bytesToRead, sizeof(int)) == -1)
                {
                }
                dirName = (char *)malloc(sizeof(char) * bytesToRead);
                while (read(fdRead, dirName, bytesToRead) == -1)
                {
                }
                dirNames[numOfDirs - n - 1] = dirName;
            }

            workerExec(input_dir, numOfDirs, dirNames, fdWrite, fdRead, serverIP, serverPort);
            //handleWorkerExit();
            exit(-1);
        }
        else
        {
            myWorkersInfo->workerPIDs[i - 1] = childpid;

            myWorkersInfo->workerFDs[i - 1][0] = open(fifoToWorker, O_RDWR | O_NONBLOCK); //write
            if (myWorkersInfo->workerFDs[i - 1][0] == -1)
            {
                perror("Error on opening");
                exit(-1);
            }
            myWorkersInfo->workerFDs[i - 1][1] = open(fifoFromWorker, O_RDWR | O_NONBLOCK); //read
            if (myWorkersInfo->workerFDs[i - 1][1] == -1)
            {
                perror("Error on opening");
                exit(-1);
            }

            continue;
        }
    }
}

void workerExec(char *input_dir, int numOfDirs, char **dirNames, int fdWrite, int fdRead, char *serverIP, int serverPort)
{

    StatsCountryNode *countriesListHead = NULL, *currCountryNode = NULL;
    StatsDateNode *currDateNode = NULL;
    //read directories
    char *inputPath = NULL, *fullPath = NULL;
    dirListNode *headList, *current;
    listNode *recordsListHead = NULL, *tmp = NULL;
    for (int i = 0; i < numOfDirs; i++)
    {

        currCountryNode = appendToCountriesList(&countriesListHead, dirNames[i]); // append each country

        inputPath = (char *)malloc(sizeof(char) * (strlen(input_dir) + strlen(dirNames[i]) + 2));
        sprintf(inputPath, "%s/%s", input_dir, dirNames[i]);

        headList = dirListingToList(inputPath);
        current = headList;

        while (current != NULL)
        {
            fullPath = (char *)malloc(sizeof(char) * (strlen(inputPath) + 16)); // 15 is the static size of a Date
            sprintf(fullPath, "%s/%s", inputPath, current->dirName);
            printf("Reading data of %s\n", fullPath);

            currDateNode = appendToSortedDatesList(&(currCountryNode->dateListPtr), current->dirName); // append each date of the country

            recordsListHead = storeData(fullPath, &recordsListHead, current->dirName, dirNames[i], &currDateNode);

            free(fullPath);
            fullPath = NULL;
            current = current->next;
        }

        free(inputPath);
        freeDirList(headList);
        inputPath = NULL;
    }
    //printStats(countriesListHead);
    //printList(recordsListHead);

    // let's create the hash tables
    hashTable diseaseHTable;
    int diseaseHashTableNumOfEntries = 7;
    bucket **diseaseHashTable = hashTableInit(diseaseHashTableNumOfEntries);
    diseaseHTable.bucketPtrs = diseaseHashTable;
    diseaseHTable.counter = diseaseHashTableNumOfEntries;
    int bucketSize = 40;
    diseaseHTable.bucketSize = bucketSize;

    hashTable countryHTable;
    int countryHashTableNumOfEntries = 7;
    bucket **countryHashTable = hashTableInit(countryHashTableNumOfEntries);
    countryHTable.bucketPtrs = countryHashTable;
    countryHTable.counter = countryHashTableNumOfEntries;
    countryHTable.bucketSize = bucketSize;

    listNode *currentRecordsList = recordsListHead;
    while (currentRecordsList != NULL)
    {
        hashTableInsert(&diseaseHTable, currentRecordsList->record->diseaseID, currentRecordsList);
        hashTableInsert(&countryHTable, currentRecordsList->record->country, currentRecordsList);
        currentRecordsList = currentRecordsList->next;
    }

    connectToServer(&diseaseHTable, &countryHTable, &countriesListHead, serverIP, serverPort);

    freeList(recordsListHead);
    freeHTable(&diseaseHTable);
    freeHTable(&countryHTable);

    // int done = 1;
    // write(fdWrite, &done, sizeof(int));
}

void connectToServer(hashTable *diseaseHTable, hashTable *countryHTable, StatsCountryNode **countriesListHead, char *serverIP, int serverPort)
{
    int sockfd, n;
    int sendbytes;
    int listenQueriesfd;
    struct sockaddr_in servaddr, servaddrQueries, servaddrTmp;
    uint8_t sendline[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perrorexit("socket creation");
    if ((listenQueriesfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perrorexit("socket creation");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(serverPort);

    // where the worker is going to listen for Queries
    bzero(&servaddrQueries, sizeof(servaddrQueries));
    servaddrQueries.sin_family = AF_INET;
    servaddrQueries.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddrQueries.sin_port = htons(0);

    // let's bind the listed socket so a random port is generated
    if ((bind(listenQueriesfd, (SA *)&servaddrQueries, sizeof(servaddrQueries))) < 0)
        perrorexit("bind error");
    socklen_t len = sizeof(servaddrTmp);
    if (getsockname(listenQueriesfd, (SA *)&servaddrTmp, &len) == -1)
        perrorexit("getsockname");
    printf("worker port: %d\n", ntohs(servaddrTmp.sin_port));

    int workersPort = ntohs(servaddrTmp.sin_port);
    u_int32_t convertedPort = htonl(workersPort);

    // let's connect to the server and send stats and port number
    if (inet_pton(AF_INET, serverIP, &servaddr.sin_addr) <= 0)
        perrorexit("inet_pton");
    sendStatsAndPort(&servaddr, &sockfd, &convertedPort, countriesListHead);

    memset(recvline, 0, MAXLINE);
    //read server's response
    printf("Server msg: ");
    while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0)
    {
        printf("%s", recvline);

        if (recvline[n] == '\0')
            break;
        
        memset(recvline, 0, MAXLINE);
    }
    // can't read negative bytes
    if (n < 0)
        perrorexit("read error");

    listenForQueries(&listenQueriesfd, &servaddrQueries);

        exit(0);
}

void perrorexit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void sendStatsAndPort(struct sockaddr_in *servaddr, int *sockfd, u_int32_t *convertedPort, StatsCountryNode **countriesListHead)
{
    uint8_t sendline[MAXLINE + 1];
    memset(sendline, 0, MAXLINE);
    uint8_t recvline[MAXLINE + 1];
    memset(recvline, 0, MAXLINE);
    int n, sendbytes;

    if(connect(*sockfd, (SA *)servaddr, sizeof(*servaddr)) < 0)
        perrorexit("socket connection failed");

    puts("Sending msg to server");
    if (write(*sockfd, convertedPort, sizeof(*convertedPort)) != sizeof(*convertedPort))
        perrorexit("write error");
    
    sendStatsToServer(*countriesListHead, sockfd);
}

void sendStatsToServer(StatsCountryNode *countriesListHead, int *sockfd)
{
    uint8_t sendline[MAXLINE + 1];
    int sendbytes;
    memset(sendline, 0, MAXLINE);

    StatsCountryNode *currCountryNode = countriesListHead;

    while (currCountryNode != NULL)
    {
        StatsDateNode *currDateNode = currCountryNode->dateListPtr;

        while (currDateNode != NULL)
        {
            StatsDiseaseNode *currDiseaseNode = currDateNode->diseaseListPtr;

            sprintf(sendline, "%d-%d-%d\n", currDateNode->entryDate.day, currDateNode->entryDate.month, currDateNode->entryDate.year);
            sendAndCleanBuff(sockfd, sendline);

            sprintf(sendline, "%s\n", currCountryNode->name);
            sendAndCleanBuff(sockfd, sendline);

            while (currDiseaseNode != NULL)
            {
                sprintf(sendline, "%s\n", currDiseaseNode->name);
                sendAndCleanBuff(sockfd, sendline);

                sprintf(sendline, "Age range 0-20 years: %d cases\n", currDiseaseNode->range0to20);
                sendAndCleanBuff(sockfd, sendline);
                sprintf(sendline, "Age range 21-40 years: %d cases\n", currDiseaseNode->range21to40);
                sendAndCleanBuff(sockfd, sendline);
                sprintf(sendline, "Age range 41-60 years: %d cases\n", currDiseaseNode->range41to60);
                sendAndCleanBuff(sockfd, sendline);
                sprintf(sendline, "Age range 60+ years: %d cases\n", currDiseaseNode->range61to120);
                sendAndCleanBuff(sockfd, sendline);

                currDiseaseNode = currDiseaseNode->next;
            }

            currDateNode = currDateNode->next;
        }
        sprintf(sendline, "==================================\n");
        sendAndCleanBuff(sockfd, sendline);

        currCountryNode = currCountryNode->next;
    }
    OverAndOut(sockfd); // protocol: send that message is over
}

void sendAndCleanBuff(int *sockfd, uint8_t *sendline) {
    int sendbytes = strlen(sendline);
    if (write(*sockfd, sendline, sendbytes) != sendbytes)
        perrorexit("write error");
    memset(sendline, 0, MAXLINE);
}

void listenForQueries(int *listenQueriesfd, struct sockaddr_in *servaddrQueries) {

    int connfd, n;
    uint8_t recvline[MAXLINE + 1];

    if ((listen(*listenQueriesfd, 10)) < 0)
        perrorexit("listen error");

    while(1) { // wait until an incoming connection arrives
        puts("I'm waiting for incoming Queries...");
        connfd = accept(*listenQueriesfd, (SA *)NULL, NULL);
        // zero out the receiving buffer 1st to make sure it ends up null terminated
        memset(recvline, 0, MAXLINE);

        //read server's query
        while ((n = read(connfd, recvline, MAXLINE - 1)) > 0)
        {
            printf("%s", recvline);
            if (recvline[n] == '\0') // protocol: sign that message is over
                break;

            memset(recvline, 0, MAXLINE);
        }
        // can't read negative bytes
        if (n < 0)
            perrorexit("read error");
    }
}

void OverAndOut(int *sockfd) {
    uint8_t sendline[MAXLINE + 1];
    memset(sendline, 0, MAXLINE);
    sprintf(sendline, "\0");
    if (write(*sockfd, sendline, 1) != 1)
        perrorexit("write error");
}