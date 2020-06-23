#define LINE_MAX 1000

typedef int bool;
#define true 1;
#define false 0;

typedef struct cmdNode
{
    char *cmd;
    struct cmdNode *next;
} cmdNode;

bool validArgs(int argc, char *argv[]);

void getArgs(int *numThreads, int *servPort, char **queryFile, char **servIP, char *argv[]);

void perrorexit(char *message);

char *bin2hex(const unsigned char *input, size_t len);

void addToBuffer(int *client_socket);

int *removeFromBuffer();

int getNodesCount();

void *queriesDistribution(void *arg);

void addCmdsToList(char *queriesFile);

void sendAndCleanBuff(int *sockfd, uint8_t *sendline);

void OverAndOut(int *sockfd);

void freeQueriesList();