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

void perrorexit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

bool validArgs(int argc, char *argv[])
{

    if (argc == 9)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void getArgs(int *numThreads, int *servPort, char **queryFile, char **servIP, char *argv[])
{

    for (int i = 1; i < 9; i = i + 2)
    {
        if (!strcmp(argv[i], "-q"))
        {
            *queryFile = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-sp"))
        {
            *servPort = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-w"))
        {
            *numThreads = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-sip"))
        {
            *servIP = argv[i + 1];
        }
    }
}