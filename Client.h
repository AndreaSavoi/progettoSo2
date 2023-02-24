#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#define PORT 25245
#define SIZE 25
#define fflush(stdin) while ((getchar()) != '\n')

void sigpipe_handler(int);
void sigint_handler(int);

int matrix[SIZE][SIZE];

int ParseCmdLine(int, char**, char**);

int c_socket, ret;
//int p_counter = 0;
struct sockaddr_in server;
char username[128];
char password[128];
char passwordC[128];
char buffer[1024];
char unModBuff[1024];
char littleBuf[512];
char *resBuf;
char *singleSeatBuff;
char *delim = ";";
char *delim1 = " ";
int rows, seats, i, j, reserved, r_seat, r_row, choice, code, fd;
char filename[1024];
struct hostent *he = NULL;
//he = NULL;
char *szAddress;
struct sigaction sigpipe;
struct sigaction sigint;
ssize_t rets;