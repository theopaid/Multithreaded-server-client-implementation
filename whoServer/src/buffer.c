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

struct node
{
    struct node *next;
    socketInfo *socket;
};
typedef struct node node_t;

node_t *head = NULL;
node_t *tail = NULL;
int nodesCounter = 0;

void addToBuffer(int *socketfd, int typeOfConnection)
{
    nodesCounter++;
    node_t *newnode = malloc(sizeof(node_t));
    newnode->socket = (socketInfo *) malloc(sizeof(socketInfo));
    newnode->socket->socketfd = socketfd;
    newnode->socket->typeOfConnection = typeOfConnection;
    newnode->next = NULL;
    if (tail == NULL)
    {
        head = newnode;
    }
    else
    {
        tail->next = newnode;
    }
    tail = newnode;
}

// returns NULL is buffer is empty, or else the pointer to a client socket
socketInfo *removeFromBuffer()
{
    if (head == NULL)
    {
        return NULL;
    }
    else
    {
        nodesCounter--;
        socketInfo *result = head->socket;
        node_t *tmp = head;
        head = head->next;
        if (head == NULL)
        {
            tail = NULL;
        }
        free(tmp);
        return result;
    }
}

int getNodesCount() {
    return nodesCounter;
}