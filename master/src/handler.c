#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#include "../include/Interface.h"

void distributeToWorkers(workersInfo *myWorkersInfo, int numOfWorkers, int bufferSize, char *input_dir)
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

            while(read(fdRead, &numOfDirs, sizeof(int)) == -1) {}
            n = numOfDirs;
            char *dirNames[numOfDirs];
            while(n--) {
                bytesToRead = 0;
                while(read(fdRead, &bytesToRead, sizeof(int)) == -1) {}
                dirName = (char *) malloc(sizeof(char) * bytesToRead);
                while(read(fdRead, dirName, bytesToRead) == -1) {}
                dirNames[numOfDirs-n-1] = dirName;
            }

            workerExec(input_dir, numOfDirs, dirNames, fdWrite, fdRead);
            //handleWorkerExit();
            exit(-1);
        }
        else
        {
            myWorkersInfo->workerPIDs[i - 1] = childpid;

            myWorkersInfo->workerFDs[i - 1][0] = open(fifoToWorker, O_RDWR | O_NONBLOCK); //write
            if (myWorkersInfo->workerFDs[i - 1][0] == -1) {
                perror("Error on opening");
                exit(-1);
             }
            myWorkersInfo->workerFDs[i - 1][1] = open(fifoFromWorker, O_RDWR | O_NONBLOCK); //read
            if (myWorkersInfo->workerFDs[i - 1][1] == -1) {
                perror("Error on opening");
                exit(-1);
            }

            continue;
        }
    }
}

void workerExec(char *input_dir, int numOfDirs, char **dirNames, int fdWrite, int fdRead) {

    //read directories
    char *inputPath = NULL, *fullPath = NULL;
    dirListNode *headList, *current;
    listNode *recordsListHead = NULL, *tmp = NULL;
    for(int i=0; i<numOfDirs; i++) {
        inputPath = (char *)malloc(sizeof(char) * (strlen(input_dir) + strlen(dirNames[i]) + 2));
        sprintf(inputPath, "%s/%s", input_dir, dirNames[i]);

        headList = dirListingToList(inputPath);
        current = headList;

        while(current != NULL) {
            fullPath = (char *)malloc(sizeof(char) * (strlen(inputPath) + 16)); // 15 is the static size of a Date
            sprintf(fullPath, "%s/%s", inputPath, current->dirName);
            printf("Reading data of %s\n", fullPath);
            //make all the structures
            //when done, get the data and send them sendStatistics()
            recordsListHead = storeData(fullPath, &recordsListHead, current->dirName, dirNames[i]);
            


            free(fullPath);
            fullPath = NULL;
            current = current->next;
        }

        free(inputPath);
        freeDirList(headList);
        inputPath = NULL;
    }
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

    freeList(recordsListHead);
    freeHTable(&diseaseHTable);
    freeHTable(&countryHTable);

    int done = 1;
    write(fdWrite, &done, sizeof(int));
}

void sendStatistics(listNode *head, int fdWrite, int numOfDirs, char **dirNames) {

    int casesForAge[4];


}