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

void addWorkerToList(workersInfoNode **head, int port) {

    workersInfoNode *newNode = (workersInfoNode *) malloc(sizeof(workersInfoNode));
    newNode->port = port;
    newNode->next = NULL;

    if(*head == NULL) {
        *head = newNode;
        return;
    }
    workersInfoNode *current = *head;
    if(current->next!=NULL) {
        current = current->next;
    }
    current->next = newNode;
}