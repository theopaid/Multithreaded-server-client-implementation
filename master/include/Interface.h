#define LINE_MAX 1000

typedef struct dirListNode
{
    char *dirName;
    struct dirListNode *next;
} dirListNode;

typedef struct workersInfo
{
    pid_t *workerPIDs;
    int **workerFDs;
    char ***workerPATHs;
} workersInfo;

typedef int bool;
#define true 1;
#define false 0;

typedef struct Date
{
    int day;
    int month;
    int year;
} Date;

typedef struct patientRecord
{
    char *recordID;
    char *patientFirstName;
    char *patientLastName;
    char *diseaseID;
    char *country;
    Date entryDate;
    Date exitDate;
    int age;
} patientRecord;

typedef struct listNode
{
    patientRecord *record;
    struct listNode *next;
} listNode;

typedef struct bstNode
{
    listNode *record;
    Date dateValue;
    struct bstNode *left;
    struct bstNode *right;
    int height;
    int count;
} bstNode;

typedef struct bucketPair
{
    char *key;
    bstNode *root;
} bucketPair;

typedef struct bucket
{
    bucketPair *pairsInBucket;
    int pairsCounter;
    struct bucket *next;
} bucket;

typedef struct hashTable
{
    bucket **bucketPtrs;
    int counter;
    int bucketSize;
} hashTable;

bool validArgs(int argc, const char *argv[]);

void getArgs(int *numOfWorkers, int *bufferSize, char **input_dir, const char *argv[]);

void distributeToWorkers(workersInfo *myWorkersInfo, int numOfWorkers, int bufferSize, char *input_dir);

void renderMenu(workersInfo *myWorkersInfo, int numOfWorkers);

dirListNode *dirListingToList(char *inputDir);

int listNodeCounter(dirListNode *head);

listNode *storeData(char *patientRecordsFile, listNode **head, char *date, char *country);

listNode *patientsEntryRecord(listNode *head, char *recordID);

listNode *sortDateInsert(listNode **head, patientRecord **record);

void printList(listNode *head);

void printRecord(patientRecord record);

void freeDirList(dirListNode *head);

void allocateWorkersInfo(workersInfo *myWorkersInfo, int numOfWorkers);

void workerExec(char *input_dir, int numOfDirs, char **dirNames, int fdWrite, int fdRead);

bool isUniqueID(listNode *head, char *newID);

int compareDates(listNode *current, patientRecord *record);

int compareStructDates(Date date1, Date date2);

int max(int a, int b);

int height(bstNode *N);

bstNode *rightRotate(bstNode *y);

bstNode *leftRotate(bstNode *x);

bstNode *insert(bstNode *node, Date keydateValue, listNode *record);

void preOrder(bstNode *root);

int getBalance(bstNode *N);

bstNode *newNode(Date dateValue, listNode *record);

bucket **hashTableInit(int tableSize);

void hashTableInsert(hashTable *hTable, char *keyName, listNode *record);

int compareStructDates(Date date1, Date date2);

bool isUniqueID(listNode *head, char *newID);

int hashFunction(char *deseaseID, int hashTableSize);

void freeList(listNode *head);

void freeAVL(bstNode *root);

void freeBuckets(bucket *firstBucket);

void freeHTable(hashTable *hashTable);

void sendStatistics(listNode *head, int fdWrite, int numOfDirs, char **dirNames);