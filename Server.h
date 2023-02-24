#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include "errno.h"
#include <arpa/inet.h>

#define PORT 25245
#define MAX_DIM 1024
#define QUEUE 4

int rows, r_seats;
FILE *fileL;
FILE *fileB;
FILE *fileBD;
char *singleSeatBuff;
char filename[1024] = "login-database.txt";
char filenameB[1024] = "backup-database.txt";
char filenameBD[1024] = "backupDel-database.txt";
char *delim = " ";
char *delimS = "-";
char unModBuff[1024];
pthread_mutex_t *list;
pthread_mutex_t *login;

typedef struct seatNode {
    int s_i;
    int s_j;
    int s_code;
    char *usr;
    struct seatNode *next;
} s_node;

int code = 0;
int n_counter = 0;
int operation = -1;
int tokind;