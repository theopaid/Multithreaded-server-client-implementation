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
pthread_mutex_t mutexToPrintAnswers = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conditionVar = PTHREAD_COND_INITIALIZER;

// global structure
typedef struct cmdNode
{
    char *cmd;
    struct cmdNode *next;
} cmdNode;

cmdNode *head = NULL;
cmdNode *current = NULL;
int threadsOccupied = 0;
int numThreads = 0;

int main(int argc, char *argv[])
{

    if (!validArgs(argc, argv))
    {
        printf("The arguments given are invalid. Try again.\n");
        return -1;
    }

    int servPort;
    char *queryFile, *servIP;
    getArgs(&numThreads, &servPort, &queryFile, &servIP, argv);

    addCmdsToList(queryFile);
    current = head;

    SA_IN servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    if (inet_pton(AF_INET, servIP, &servaddr.sin_addr) <= 0)
        perrorexit("inet_pton");

    // create all the threads that we need for future connections
    pthread_t thread_pool[numThreads];
    for (int i = 0; i < numThreads; i++)
    {
        pthread_create(&thread_pool[i], NULL, queriesDistribution, &servaddr);
    }
    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(thread_pool[i], NULL);
    }
    return 0;
}

void *queriesDistribution(void *arg)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perrorexit("socket creation");

    SA_IN *servaddr = (SA_IN *)arg;
    uint8_t sendline[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];
    int sendbytes, n;
    memset(sendline, 0, MAXLINE);
    memset(recvline, 0, MAXLINE);

    pthread_mutex_lock(&mutex);

    threadsOccupied++;
    if (current != NULL)
    { // there are commands left to send
        sprintf(sendline, "%s", current->cmd);
        current = current->next;
        if ((threadsOccupied < numThreads) && (current != NULL))
        {
            pthread_cond_wait(&conditionVar, &mutex);
        }
        else
        { // reached last thread, so we can send all of the commands at once now
            threadsOccupied = 0;
            puts("============================================");
            puts("Sending a batch of commands to the Server...\n");
            pthread_cond_broadcast(&conditionVar);
        }

        pthread_mutex_unlock(&mutex);

        if (connect(sockfd, (SA *)servaddr, sizeof(*servaddr)) < 0)
            perrorexit("socket connection failed");
        sendAndCleanBuff(&sockfd, sendline);
        OverAndOut(&sockfd);
        
        // let's get the answers
        while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0)
        {
            if (recvline[0] != '\0')
            { // if it's a regular message
                printf("Server's Answer: ");
                printf("%s\n", recvline);
            }

            if (recvline[n - 1] == '\0')
            { // protocol: sign that message is over
                //puts("Command read by Server");
                break;
            }

            memset(recvline, 0, MAXLINE);
        }
        close(sockfd);
        if (current != NULL)
        {
            queriesDistribution(arg);
        }
    }
    else
    {
        pthread_mutex_unlock(&mutex);
    }
}

// add all commands from file in a list so we can handle them easily
void addCmdsToList(char *queriesFile)
{
    FILE *fp;
    fp = fopen(queriesFile, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, fp)) != -1)
    { // remove endline char from string if one is read
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }
        cmdNode *newNode = (cmdNode *)malloc(sizeof(cmdNode));
        newNode->cmd = (char *)malloc(sizeof(char) * (strlen(line) + 1));
        strcpy(newNode->cmd, line);
        newNode->next = NULL;

        if (head == NULL)
        { // if list is empty
            head = newNode;
            current = head;
        }
        else
        {
            current->next = newNode;
            current = current->next;
        }
    }

    fclose(fp);
}

void sendAndCleanBuff(int *sockfd, uint8_t *sendline)
{
    int sendbytes = strlen(sendline);
    printf("$%s\n", sendline);
    if (write(*sockfd, sendline, sendbytes) != sendbytes)
        perrorexit("write error");
    memset(sendline, 0, MAXLINE);
}

void OverAndOut(int *sockfd)
{
    uint8_t sendline[MAXLINE + 1];
    memset(sendline, 0, MAXLINE);
    sprintf(sendline, "\0");
    if (write(*sockfd, sendline, 1) != 1)
        perrorexit("write error");
}
