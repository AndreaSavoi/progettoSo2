#include "Helper.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

ssize_t safeRead(int sockd, void *buff, size_t maxlen) {
    ssize_t n, rc;
    char    c, *buffer;

    buffer = buff;

    for ( n = 1; n < maxlen; n++ )
    {
        rc = read(sockd, &c, 1);
        if ( rc == 1 ) {
            *buffer++ = c;
            if ( c == '\n' )
                break;
        } else if ( rc == 0 ) {
            perror("[-]Connection timed out");
            return -1;
        }
        else {
            if ( errno == EINTR )
                continue;
            return -1;
        }
    }
    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t safeWrite(int sockd, const void *buff, size_t n) {
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = buff;
    nleft  = n;

    while ( nleft > 0 )
    {
        if ( (nwritten = write(sockd, buffer, nleft)) <= 0 )
        {
            if ( errno == EINTR ) nwritten = 0;
            else return -1;
        }
        nleft  -= nwritten;
        buffer += nwritten;
    }
    return n;
}

void handle_error_rs(ssize_t ret_value, int sockfd) {
    if (ret_value <= 0) {
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            perror("[-]Timed out");
            close(sockfd);
            pthread_exit(NULL);
        } else {
            perror("[-]Receive error");
            close(sockfd);
            pthread_exit(NULL);
        }
    }
}

void handle_error_ws(ssize_t ret_value, int sockfd) {
    if (ret_value <= 0) {
        perror("[-]Send error");
        close(sockfd);
        pthread_exit(NULL);
    }
}

void handle_error_rc(ssize_t ret_value, int sockfd) {
    if (ret_value <= 0) {
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            perror("[-]Timed out");
            close(sockfd);
            exit(1);
        } else {
            perror("[-]Receive error");
            close(sockfd);
            pthread_exit(NULL);
        }
    }
}

void handle_error_wc(ssize_t ret_value, int sockfd) {
    if (ret_value <= 0) {
        perror("[-]Send error");
        close(sockfd);
        exit(1);
    }
}