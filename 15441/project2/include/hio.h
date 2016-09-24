#ifndef __HIO_H__
#define __HIO_H__
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define RIO_BUFSIZE 8192

typedef struct {
    int rio_fd;
    int rio_cnt;                //unread size in buf
    char *rio_bufptr;           //pointer to next unread position
    char rio_buf[RIO_BUFSIZE];  //internal buf array
} rio_t;

ssize_t rio_readn(int fd, char* ubuf, size_t n);
ssize_t rio_writen(int fd, char* ubuf, size_t n);

void rio_initb(rio_t *rp, int fd);
ssize_t rio_readnb(rio_t *rp, char* ubuf, size_t n);
ssize_t rio_readlineb(rio_t *rp, char* ubuf, size_t maxlen);



#endif//__HIO_H__
