#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#include "../include/Interface.h"

void renderMenu(workersInfo *myWorkersInfo, int numOfWorkers)
{

    char line[LINE_MAX];
    char *command = NULL, *userInput = NULL, *arguments = NULL;

    printf("-------------------------------------------------------------------\n\n");
    printf("\t\t----------- DISEASE AGGREGATOR -----------\n\n");
    printf("For instructions on how to use the app, type /man :\n");

    while (1)
    {

        printf("\n%s> ", "DiseaseAggregator");

        if (fgets(line, LINE_MAX, stdin) == NULL)
        {
            break;
        }

        userInput = line;
        command = strtok_r(userInput, " \n", &userInput); // Getting main
        arguments = strtok(userInput, "\n");              // Getting all the arguments. NULL if there are none

        if (command != NULL)
        {
            if (strcmp(command, "/globalDiseaseStats") == 0)
            {
                if (arguments != NULL && strlen(arguments) < 19)
                {
                    printf("Time specific search needs both Entry and Exit dates!\n");
                    continue;
                }
                else
                {
                    //globalDiseaseStats(arguments, diseaseHTable);
                }
            }
            else if (strcmp(command, "/diseaseFrequency") == 0)
            {
                if (arguments != NULL && strlen(arguments) < 10)
                {
                    printf("Data is not valid!\n");
                    continue;
                }
                else if (arguments == NULL)
                {
                    printf("No arguments were given!\n");
                }
                else
                {
                    //diseaseFrequency(arguments, diseaseHTable);
                }
            }
            else if (strcmp(command, "/topk-Diseases") == 0)
            {
                if (arguments != NULL && (strlen(arguments) < 24 && strlen(arguments) > 12))
                {
                    printf("Time specific search needs both Entry and Exit dates!\n");
                    continue;
                }
                else
                {
                    // topDiseases(arguments, diseaseHTable, head, headOfUniqueDiseases);
                }
            }
            else if (strcmp(command, "/topk-Countries") == 0)
            {
                if (arguments != NULL && (strlen(arguments) < 24 && strlen(arguments) > 15))
                {
                    printf("Time specific search needs both Entry and Exit dates!\n");
                    continue;
                }
                else
                {
                    //topkCountries(arguments, countryHTable, head);
                }
            }
            else if (strcmp(command, "/insertPatientRecord") == 0)
            {
                if (arguments != NULL && strlen(arguments) < 30)
                {
                    printf("Data is not valid!\n");
                    continue;
                }
                else
                {
                    //  insertPatientRecord(arguments, diseaseHTable, countryHTable, head, headOfUniqueCountries, headOfUniqueDiseases);
                }
            }
            else if (strcmp(command, "/recordPatientExit") == 0)
            {
                if (arguments != NULL && strlen(arguments) < 10)
                {
                    printf("Enter a valid Exit date!\n");
                    continue;
                }
                else
                {
                    //  recordPatientExit(arguments, head);
                }
            }
            else if (strcmp(command, "/man") == 0)
            {
                // printManual();
            }
            else if (strcmp(command, "/exit") == 0)
            {
                for(int i=0; i<numOfWorkers; i++) {
                    close(myWorkersInfo->workerFDs[i][0]); // close write
                    close(myWorkersInfo->workerFDs[i][1]); // close read

                    unlink(myWorkersInfo->workerPATHs[i][0]);
                    unlink(myWorkersInfo->workerPATHs[i][1]);

                    kill(myWorkersInfo->workerPIDs[i], SIGKILL);
                }
                printf("\nExiting the application. Goodbye and stay safe..\n");
                return;
            }
            else
            {
                printf("Command not found!\n");
            }
        }
    }
}