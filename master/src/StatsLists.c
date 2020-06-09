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

StatsCountryNode *appendToCountriesList(StatsCountryNode **countriesListHead,char *countryName) {

    StatsCountryNode *newNode; // create the new node to be appended
    newNode = malloc(sizeof(StatsCountryNode));
    newNode->name = malloc(strlen(countryName) + 1);
    strcpy(newNode->name, countryName);
    newNode->dateListPtr = NULL;
    newNode->next = NULL;

    if(*countriesListHead == NULL) { // if list is empty
        *countriesListHead = newNode;
        return newNode;
    }

    StatsCountryNode *current = *countriesListHead;
    while(current->next != NULL) { // go to last node of list
        current = current->next;
    }

    current ->next = newNode;
    return newNode;
}

StatsDateNode *appendToSortedDatesList(StatsDateNode **datesListHead, char *dateInfo) {

    StatsDateNode *newNode; // create the new node to be inserted in a sorted order
    newNode = malloc(sizeof(StatsDateNode));
    sscanf(dateInfo, "%d-%d-%d.txt", &(newNode->entryDate.day), &(newNode->entryDate.month), &(newNode->entryDate.year));
    newNode->diseaseListPtr = NULL;
    newNode->next = NULL;

    if (*datesListHead == NULL)
    {
        *datesListHead = newNode;
        return newNode;
    }

    StatsDateNode *current = *datesListHead;
    while (current->next != NULL)
    { // go to last node of list
        current = current->next;
    }

    current->next = newNode;
    return newNode;
}

void addStatsToDisease(StatsDiseaseNode **diseaseListHead, char *diseaseID, int age) {

    if(*diseaseListHead == NULL) { // if list is empty
        StatsDiseaseNode *newNode;
        newNode = malloc(sizeof(StatsDiseaseNode));
        newNode->name = malloc(strlen(diseaseID) + 1);
        strcpy(newNode->name, diseaseID);
        newNode->range0to20 = 0;
        newNode->range21to40 = 0;
        newNode->range41to60 = 0;
        newNode->range61to120 = 0;

        if(age < 21)
            newNode->range0to20++;
        else if(age < 41)
            newNode->range21to40++;
        else if(age < 61)
            newNode->range41to60++;
        else
            newNode->range61to120++;
        newNode->next = NULL;
        *diseaseListHead = newNode;

        return;
    }

    StatsDiseaseNode *current = *diseaseListHead;
    while(1) {
        if (!strcmp(current->name, diseaseID))
        { // if current is the disease we are looking for
            if (age < 21)
                current->range0to20++;
            else if (age < 41)
                current->range21to40++;
            else if (age < 61)
                current->range41to60++;
            else
                current->range61to120++;

            return;
        }

        if(current->next == NULL)
            break;
        current = current->next;
    }

    //if no disease match is found, append new disease node and add stats
    StatsDiseaseNode *newNode;
    newNode = malloc(sizeof(StatsDiseaseNode));
    newNode->name = malloc(strlen(diseaseID) + 1);
    strcpy(newNode->name, diseaseID);
    newNode->range0to20 = 0;
    newNode->range21to40 = 0;
    newNode->range41to60 = 0;
    newNode->range61to120 = 0;

    if (age < 21)
        newNode->range0to20++;
    else if (age < 41)
        newNode->range21to40++;
    else if (age < 61)
        newNode->range41to60++;
    else
        newNode->range61to120++;
    newNode->next = NULL;
    current->next = newNode;

    return;
}

void printStats(StatsCountryNode *countriesListHead) {

    StatsCountryNode *currCountryNode = countriesListHead;

    while(currCountryNode != NULL) {
        StatsDateNode *currDateNode = currCountryNode->dateListPtr;

        while(currDateNode != NULL) {
            StatsDiseaseNode *currDiseaseNode = currDateNode->diseaseListPtr;

            printf("%d-%d-%d\n", currDateNode->entryDate.day, currDateNode->entryDate.month, currDateNode->entryDate.year);
            puts(currCountryNode->name);

            while(currDiseaseNode != NULL) {
                puts(currDiseaseNode->name);
                printf("Age range 0-20 years: %d cases\n", currDiseaseNode->range0to20);
                printf("Age range 21-40 years: %d cases\n", currDiseaseNode->range21to40);
                printf("Age range 41-60 years: %d cases\n", currDiseaseNode->range41to60);
                printf("Age range 60+ years: %d cases\n", currDiseaseNode->range61to120);
                puts("");

                currDiseaseNode = currDiseaseNode->next;
            }

            currDateNode = currDateNode->next;
        }

        currCountryNode = currCountryNode->next;
    }
}