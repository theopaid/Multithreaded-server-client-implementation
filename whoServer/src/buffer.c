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
    int *client_socket;
};
typedef struct node node_t;

node_t *head = NULL;
node_t *tail = NULL;
int nodesCounter = 0;

void addToBuffer(int *client_socket)
{
    nodesCounter++;
    node_t *newnode = malloc(sizeof(node_t));
    newnode->client_socket = client_socket;
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
int *removeFromBuffer()
{
    if (head == NULL)
    {
        return NULL;
    }
    else
    {
        nodesCounter--;
        int *result = head->client_socket;
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