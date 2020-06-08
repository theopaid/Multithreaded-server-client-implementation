#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/Interface.h"

void freeList(listNode *head) {

    listNode *tmp;
    while(head != NULL) {
        tmp = head;
        head = head->next;

        free(tmp->record->recordID);
        free(tmp->record->patientFirstName);
        free(tmp->record->patientLastName);
        free(tmp->record->diseaseID);
        free(tmp->record->country);
        free(tmp->record);

        free(tmp);
    }
}

void freeAVL(bstNode *root) {

    if(root != NULL){
        freeAVL(root->left);
        freeAVL(root->right);
        free(root);
    }
}

void freeBuckets(bucket *firstBucket) {

    bucket *tmpBucket;
    while(firstBucket != NULL) {
        tmpBucket = firstBucket;
        firstBucket = firstBucket->next;

        for(int j=0; j < tmpBucket->pairsCounter; j++) {
            free(tmpBucket->pairsInBucket[j].key);
            freeAVL(tmpBucket->pairsInBucket[j].root);
        }

        free(tmpBucket);
    }
}

void freeHTable(hashTable *hashTable) {

    bucket **tmpbucketPtrs;
    for(int i=0; i < hashTable->counter; i++) {
        tmpbucketPtrs = hashTable->bucketPtrs;

        if(tmpbucketPtrs != NULL) {
            freeBuckets(tmpbucketPtrs[i]);
        }
    }
    free(tmpbucketPtrs);
}
