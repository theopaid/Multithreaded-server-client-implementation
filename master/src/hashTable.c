#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/Interface.h"

bucket **hashTableInit(int tableSize) {

    bucket **hashTable = malloc(tableSize * sizeof(bucket*));
    int i;
    for(i = 0; i < tableSize; i++) {
        hashTable[i] = NULL;
    }
    return hashTable;
}

void hashTableInsert(hashTable *hashTable, char *keyName, listNode *record) {

    int key = hashFunction(keyName, hashTable->counter);
    int remainingSize = hashTable->bucketSize - sizeof(int) - sizeof(bucket*);
    int canHoldPairs = remainingSize / (sizeof(char*) + sizeof(bstNode*));
    int i;

    if(canHoldPairs < 1) {
        printf("Can't hold any info, because the bucket size given is pretty small!\n");
        exit(-1);
    }

    if(hashTable->bucketPtrs[key] == NULL) {
        // if this index is empty create new bucket and allocate memory for it
        hashTable->bucketPtrs[key] = malloc(sizeof(bucket));
        hashTable->bucketPtrs[key]->next = NULL;
        hashTable->bucketPtrs[key]->pairsInBucket = malloc(canHoldPairs * sizeof(bucketPair));
        hashTable->bucketPtrs[key]->pairsInBucket[0].root = NULL;
        hashTable->bucketPtrs[key]->pairsInBucket[0].key = malloc(sizeof(char)*(strlen(keyName) + 1));
        strcpy(hashTable->bucketPtrs[key]->pairsInBucket[0].key, keyName);
        hashTable->bucketPtrs[key]->pairsInBucket[0].root = insert(hashTable->bucketPtrs[key]->pairsInBucket[0].root, record->record->entryDate, record);
        preOrder(hashTable->bucketPtrs[key]->pairsInBucket[0].root);
        hashTable->bucketPtrs[key]->pairsCounter = 1;
    }
    else {
        bucket *tempBucket = hashTable->bucketPtrs[key];
        while(1) {
            for(i = 0; i < tempBucket->pairsCounter; i++) {
                if(strcmp(keyName, tempBucket->pairsInBucket[i].key) == 0) {
                    // this key is already stored in a bucket
                    hashTable->bucketPtrs[key]->pairsInBucket[i].root = insert(hashTable->bucketPtrs[key]->pairsInBucket[i].root, record->record->entryDate, record);
                    preOrder(hashTable->bucketPtrs[key]->pairsInBucket[0].root);
                    return;
                }
            }
            if(tempBucket->next == NULL) {
                break;
            }
            tempBucket = tempBucket->next;
        }
        // first check if there is space in the last bucket
        if(tempBucket->pairsCounter < canHoldPairs) {
            // there is free space
            tempBucket->pairsInBucket[tempBucket->pairsCounter].root = NULL;
            tempBucket->pairsInBucket[tempBucket->pairsCounter].key = malloc(sizeof(char)*(strlen(keyName) + 1));
            strcpy(tempBucket->pairsInBucket[tempBucket->pairsCounter].key, keyName);
            tempBucket->pairsInBucket[tempBucket->pairsCounter].root = insert(tempBucket->pairsInBucket[tempBucket->pairsCounter].root, record->record->entryDate, record);
            preOrder(tempBucket->pairsInBucket[tempBucket->pairsCounter].root);
            tempBucket->pairsCounter += 1;
            return;
        }
        // we need to create a new bucket
        tempBucket->next = malloc(sizeof(bucket));
        tempBucket = tempBucket->next; // point to newly created bucket
        tempBucket->next = NULL;
        tempBucket->pairsInBucket = malloc(canHoldPairs * sizeof(bucketPair));
        tempBucket->pairsInBucket[0].root = NULL;

        tempBucket->pairsInBucket[0].key  = malloc(sizeof(char)*(strlen(keyName) + 1));
        strcpy(tempBucket->pairsInBucket[0].key, keyName);
        tempBucket->pairsCounter = 1;
        tempBucket->pairsInBucket[0].root = insert(tempBucket->pairsInBucket[0].root, record->record->entryDate, record);
        preOrder(tempBucket->pairsInBucket[0].root);

    }
}
