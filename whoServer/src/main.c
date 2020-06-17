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
    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    uint8_t buff[MAXLINE + 1];
    uint8_t recvline[MAXLINE + 1];

    // allocate new socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perrorexit("socket creation");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(statisticsPortNum);

    if ((bind(listenfd, (SA *)&servaddr, sizeof(servaddr))) < 0)
        perrorexit("bind error");

    if ((listen(listenfd, 10)) < 0)
        perrorexit("listen error");

    while (1)
    { // wait until an incoming connection arrives
        struct sockaddr_in addr;
        socklen_t addr_len;
        printf("Waiting for a connection on port: %d\n", statisticsPortNum);

        connfd = accept(listenfd, (SA *)NULL, NULL);

        // zero out the receiving buffer 1st to make sure it ends up null terminated
        memset(recvline, 0, MAXLINE);
        
        // read worker's port
        int tmp, workerPort;
        if((n = read(connfd, &tmp, sizeof(tmp))) > 0)
            workerPort = ntohl(tmp);
        else
            perrorexit("read error");
        printf("Worker's Port: %d\n", workerPort);

        // read worker's stats
        printf("===== Worker's statistics below ======\n");
        while ((n = read(connfd, recvline, MAXLINE - 1)) > 0)
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
        write(connfd, (char *)buff, strlen((char *)buff));
        close(connfd);
    }

    return 0;
}
