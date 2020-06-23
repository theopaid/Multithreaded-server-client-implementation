#define LINE_MAX 1000

typedef int bool;
#define true 1;
#define false 0;

typedef struct socketInfo
{
    int *socketfd;
    int typeOfConnection; // 0 if worker, 1 if query
} socketInfo;

typedef struct workersInfoNode {
    int port;
    struct workersInfoNode *next;
} workersInfoNode;


bool validArgs(int argc, char *argv[]);

void getArgs(int *numThreads, int *bufferSize, int *queryPortNum, int *statisticsPortNum, char *argv[]);

void perrorexit(char *message);

char *bin2hex(const unsigned char *input, size_t len);

void addToBuffer(int *client_socket, int typeOfConnection);

socketInfo *removeFromBuffer();

int getNodesCount();

void *socketDistribution(void *arg);

void handleConnectionForWorker(int *ptrClient);

void handleConnectionForQuery(int *ptrClient);

void OverAndOut(int *sockfd);

void addWorkerToList(workersInfoNode **head, int port);

void sendQueryToWorkers(uint8_t *sendline, int * clientFd);

void sendAndCleanBuff(int *sockfd, uint8_t *sendline);

void sendAnswerToClient(char *sendline, int *clientFd);

void sigint_handler(int sig);