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

char *bin2hex(const unsigned char *input, size_t len) {
    char *result;
    char *hexits = "0123456789ABCDEF";

    if(input == NULL || len <= 0)
        return NULL;

    int resultLength = (len*3)+1;

    result = malloc(resultLength);
    bzero(result, resultLength);

    for(int i = 0; i<len; i++) {
        result[i*3] = hexits[input[i] >> 4];
        result[(i*3)+1] = hexits[input[i] & 0x0F];
        result[(i * 3) + 2] = ' '; // for readability
    }

    return result;
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

void getArgs(int *numThreads, int *bufferSize, int *queryPortNum, int *statisticsPortNum, char *argv[])
{

    for (int i = 1; i < 9; i = i + 2)
    {
        if (!strcmp(argv[i], "-q"))
        {
            *queryPortNum = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-b"))
        {
            *bufferSize = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-w"))
        {
            *numThreads = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-s"))
        {
            *statisticsPortNum = atoi(argv[i + 1]);
        }
    }
}