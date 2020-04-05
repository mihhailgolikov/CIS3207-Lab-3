#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8888

#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int processWrd(char*);
int isWrdANum(char*);

#endif