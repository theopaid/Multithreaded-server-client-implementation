#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#include "../include/Interface.h"

bool validArgs(int argc, const char *argv[])
{

    if (argc == 7)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void getArgs(int *numWorkers, int *bufferSize, char **input_dir, const char *argv[])
{

    for (int i = 1; i < 7; i = i + 2)
    {
        if (!strcmp(argv[i], "-i"))
        {
            *input_dir = argv[i + 1];
        }
        else if (!strcmp(argv[i], "-b"))
        {
            *bufferSize = atoi(argv[i + 1]);
        }
        else if (!strcmp(argv[i], "-w"))
        {
            *numWorkers = atoi(argv[i + 1]);
        }
    }
}

dirListNode *dirListingToList(char *inputDir)
{

    DIR *dp;
    struct dirent *ep;
    dp = opendir(inputDir);

    dirListNode *head = NULL, *current = NULL, *newNode = NULL, *previousNode = NULL;
    if (dp != NULL)
    {
        while ((ep = readdir(dp)) != NULL)
        {
            if ((strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0 || (*ep->d_name) == '.'))
            {
            }
            else
            {
                if (head == NULL)
                {
                    head = (dirListNode *)malloc(sizeof(dirListNode));
                    head->dirName = malloc(sizeof(char) * (strlen(ep->d_name) + 1));
                    strcpy(head->dirName, ep->d_name);
                    head->next = NULL;
                    current = head;
                }
                else
                {
                    newNode = (dirListNode *)malloc(sizeof(dirListNode));
                    newNode->dirName = malloc(sizeof(char) * (strlen(ep->d_name) + 1));
                    strcpy(newNode->dirName, ep->d_name);
                    newNode->next = NULL;

                    current = head;
                    previousNode = NULL;
                    while(1) {
                        if(strcmp(ep->d_name, current->dirName) == -1) {
                            if(!strcmp(head->dirName, current->dirName)) { // if we replace the head
                                newNode->next = current;
                                head = newNode;
                            } else {
                                newNode->next = current;
                                previousNode->next = newNode;
                            }
                            break;
                        }
                        if(current->next == NULL) { // reached last node, let's insert here
                            current->next = newNode;
                            break;
                        }
                        previousNode = current;
                        current = current->next;
                    }
                }
            }
        }

        (void)closedir(dp);
    }
    else
        perror("Couldn't open the directory");

    return head;
}

int listNodeCounter(dirListNode *head)
{

    int count = 0;
    while (head != NULL)
    {
        count++;
        head = head->next;
    }

    return count;
}

void freeDirList(dirListNode *head) {

    dirListNode *tmp;
    while(head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp->dirName);
        free(tmp);
    }
}

    void allocateWorkersInfo(workersInfo *myWorkersInfo, int numOfWorkers)
{

    myWorkersInfo->workerPIDs = (pid_t *)malloc(numOfWorkers * sizeof(pid_t));

    myWorkersInfo->workerFDs = (int **)malloc(numOfWorkers * sizeof(int *));
    for (int i = 0; i < numOfWorkers; i++)
        myWorkersInfo->workerFDs[i] = (int *)malloc(2 * sizeof(int));

    myWorkersInfo->workerPATHs = (char ***)malloc(numOfWorkers * sizeof(char **));
    for (int k = 0; k < numOfWorkers; k++)
        myWorkersInfo->workerPATHs[k] = (char **)malloc(2 * sizeof(char *));
}

int compareDates(listNode *current, patientRecord *record)
{                      // 0:for same dates
                       // 1:if new Date is older than list node's one
    int sameDates = 0; // 2:if new Date is newer than listNode's one

    if (record->entryDate.year < current->record->entryDate.year)
    {
        return 1;
    }
    else if (record->entryDate.year > current->record->entryDate.year)
    {
        return 2;
    }

    if (record->entryDate.month < current->record->entryDate.month)
    {
        return 1;
    }
    else if (record->entryDate.month > current->record->entryDate.month)
    {
        return 2;
    }

    if (record->entryDate.day < current->record->entryDate.day)
    {
        return 1;
    }
    else if (record->entryDate.day > current->record->entryDate.day)
    {
        return 2;
    }

    return sameDates;
}

int compareStructDates(Date date1, Date date2)
{                      // -1: date1 < date2
                       // 0: date1 = date2
    int sameDates = 0; // 1: date1 > date2

    if (date1.year < date2.year)
    {
        return -1;
    }
    else if (date1.year > date2.year)
    {
        return 1;
    }

    if (date1.month < date2.month)
    {
        return -1;
    }
    else if (date1.month > date2.month)
    {
        return 1;
    }

    if (date1.day < date2.day)
    {
        return -1;
    }
    else if (date1.day > date2.day)
    {
        return 1;
    }

    return sameDates;
}

int hashFunction(char *keyName, int hashTableSize)
{

    int hashVal = 0;
    int len = strlen(keyName);

    for (int i = 0; i < len; i++)
    {
        hashVal += keyName[i];
    }

    return hashVal % hashTableSize;
}