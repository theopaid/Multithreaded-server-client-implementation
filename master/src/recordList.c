#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/Interface.h"

void printRecord(patientRecord record)
{

    printf("%s | %s | %s | %s | %s | %d | %d-%d-%d | %d-%d-%d\n", record.recordID, record.patientFirstName, record.patientLastName, record.diseaseID, record.country, record.age, record.entryDate.day, record.entryDate.month, record.entryDate.year, record.exitDate.day, record.exitDate.month, record.exitDate.year);
}

void printList(listNode *head)
{

    listNode *current = head;
    while (current != NULL)
    {
        printRecord(*(current->record));
        current = current->next;
    }
}

listNode *sortDateInsert(listNode **head, patientRecord **record)
{

    int sortFlag;

    if (*head == NULL)
    { // if list is empty
        *head = (listNode *)malloc(sizeof(listNode));
        if (*head == NULL)
        {
            exit(-1);
        }
        (*head)->record = *record;
        (*head)->next = NULL;
        return *head;
    }

    listNode *current;
    sortFlag = compareDates(*head, *record);

    if (sortFlag == 0 || sortFlag == 1)
    { // if new record's date is older than head's (or same)
        current = (listNode *)malloc(sizeof(listNode));
        if (current == NULL)
        {
            exit(-1);
        }
        current->record = *record;
        current->next = *head;
        *head = current;
        return *head;
    }

    current = *head;
    listNode *previous = NULL;
    listNode *newNode = (listNode *)malloc(sizeof(listNode));
    if (newNode == NULL)
    {
        exit(-1);
    }
    newNode->record = *record;

    while (current->next != NULL && compareDates(current, *record) == 2)
    {
        previous = current;
        current = current->next;
    }

    if (current->next == NULL)
    {
        if (compareDates(current, *record) == 2)
        {
            current->next = newNode;
            newNode->next = NULL;
        }
        else
        {
            newNode->next = current;
            previous->next = newNode;
        }
    }
    else
    {
        newNode->next = current;
        previous->next = newNode;
    }
    return newNode;
}

listNode *patientsEntryRecord(listNode *head, char *recordID) {

    listNode *current = head;
    while(current != NULL) {
        if(!strcmp(current->record->recordID, recordID))
            return current;

        current = current->next;
    }

    return NULL;
}

listNode *storeData(char *patientRecordsFile,listNode **head, char *date, char *country) {

    listNode *activeCase = NULL;
    char tmpDateInfo[11];
    char tmpEntryInfo[32];

    FILE *fp = fopen(patientRecordsFile, "r");

    if (fp == NULL)
    {
        printf("Could not open file %s\n", patientRecordsFile);
        exit(-1);
    }

    patientRecord *tmpRecordPtr;

    for (int i = 0; !feof(fp); i++) {

        switch(i % 6)
        {
            case 0: // start of a record
                tmpRecordPtr = malloc(sizeof(patientRecord));
                if (tmpRecordPtr == NULL)
                {
                    exit(-1);
                }

                fscanf(fp, "%s", tmpEntryInfo);

                tmpRecordPtr->recordID = malloc(sizeof(char) * (strlen(tmpEntryInfo) + 1));
                strcpy(tmpRecordPtr->recordID, tmpEntryInfo);

                break;
            case 1:
                
                fscanf(fp, "%s", tmpEntryInfo);
                if(!strcmp(tmpEntryInfo, "ENTRY")) { // create a new record and set the entry date
                    tmpRecordPtr->country = malloc(sizeof(char) * (strlen(country) + 1));
                    strcpy(tmpRecordPtr->country, country);

                    sscanf(date, "%d-%d-%d.txt", &(tmpRecordPtr->entryDate.day), &(tmpRecordPtr->entryDate.month), &(tmpRecordPtr->entryDate.year));
                    tmpRecordPtr->exitDate.day = 0;
                    tmpRecordPtr->exitDate.month = 0;
                    tmpRecordPtr->exitDate.year = 0;
                }
                else if (!strcmp(tmpEntryInfo, "EXIT")) { // check if there is already a patient and update that record. Else ERROR.
                    activeCase = patientsEntryRecord(*head, tmpRecordPtr->recordID);
                    if(activeCase != NULL) { // there is an active case with this patient
                        sscanf(date, "%d-%d-%d.txt", &(activeCase->record->exitDate.day), &(activeCase->record->exitDate.month), &(activeCase->record->exitDate.year));
                    }
                    else {
                        printf("ERROR: Invalid record with ID = %s\n", tmpRecordPtr->recordID);
                    }
                    // tmpRecord must be destroyed now because it's invalid, and we continue to the next record
                    free(tmpRecordPtr->recordID);
                    free(tmpRecordPtr);
                    int n = 4;
                    i += 4;
                    while(n--) // cycle through the invalid record
                        fscanf(fp, "%s", tmpEntryInfo);
                    continue;
                }

                break;
            case 2:
                fscanf(fp, "%s", tmpEntryInfo);
                tmpRecordPtr->patientFirstName = malloc(sizeof(char) * (strlen(tmpEntryInfo) + 1));
                strcpy(tmpRecordPtr->patientFirstName, tmpEntryInfo);

                break;
            case 3:
                fscanf(fp, "%s", tmpEntryInfo);
                tmpRecordPtr->patientLastName = malloc(sizeof(char) * (strlen(tmpEntryInfo) + 1));
                strcpy(tmpRecordPtr->patientLastName, tmpEntryInfo);

                break;
            case 4:
                fscanf(fp, "%s", tmpEntryInfo);
                tmpRecordPtr->diseaseID = malloc(sizeof(char) * (strlen(tmpEntryInfo) + 1));
                strcpy(tmpRecordPtr->diseaseID, tmpEntryInfo);

                break;
            case 5:
                fscanf(fp, "%s", tmpEntryInfo);
                tmpRecordPtr->age = atoi(tmpEntryInfo);

                sortDateInsert(head, &tmpRecordPtr);

                break;
        }
    }

    fclose(fp);
    return *head;
}

bool isUniqueID(listNode *head, char *newID)
{

    listNode *current = head;

    while (current->next != NULL && strcmp(current->record->recordID, newID))
    {
        current = current->next;
    }

    if (current->next == NULL)
    {
        if (strcmp(current->record->recordID, newID))
        {
            return true;
        }
    }

    return false;
}