#include "hio.h"

//rio without buffer version
ssize_t rio_readn(int fd, char* ubuf, size_t n) {
    size_t nleft = n;
    ssize_t nread = 0;
    char *bufp = ubuf;
    while (nleft > 0) {
        printf("nleft = %zu\n", nleft);
        if((nread = read(fd, bufp, nleft)) < 0 ) {
            if(errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if(nread == 0) {
            //eof
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    printf("result = %zu\n", n-nleft);
    return n-nleft;
}

ssize_t rio_writen(int fd, char* ubuf, size_t n) {
    size_t nleft = n;
    ssize_t nwrite = 0;
    char *bufp = ubuf;
    while(nleft > 0) {
        if((nwrite = write(fd, bufp, nleft)) < 0) {
            if(errno == EINTR)
                nwrite = 0;
            else
                return -1;
        }
        nleft -= nwrite;
        bufp += nwrite;
    }
    return n;
}

//rio buffered version
void rio_initb(rio_t *rp, int fd) {
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_read(rio_t *rp, char* ubuf, size_t n) {
    int cnt;
    while(rp->rio_cnt <= 0) {
        //refill it when unread size is zero
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0) {
            if(errno != EINTR)
                return -1;
        } else if (rp->rio_cnt == 0) {
            return 0;
        } else {
            rp->rio_bufptr = rp->rio_buf;
        }
    }
    cnt = n;
    if(rp->rio_cnt < cnt)
        cnt = rp->rio_cnt;
    memcpy(ubuf, rp->rio_bufptr, cnt);
    rp->rio_cnt -= cnt;
    rp->rio_bufptr += cnt;
    return cnt;
}


ssize_t rio_readnb(rio_t *rp, char* ubuf, size_t n) {
    size_t nleft = n;
    ssize_t nread = 0;
    char *bufp = ubuf;
    while (nleft > 0) {
        if((nread = rio_read(rp, bufp, nleft)) < 0 ) {
            if(errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if(nread == 0) {
            //eof
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return n-nleft;
}

//this read line will contains \n
ssize_t rio_readlineb(rio_t *rp, char* ubuf, size_t maxlen) {
    int n, rc;
    char c, *bufptr = ubuf;
    for(n = 1; n < maxlen; ++n) {
        rc = rio_read(rp, &c, 1);
        if(rc ==1) {
            *bufptr++ = c;
            if(c == '\n') {
                break;
            }
        } else if (rc == 0) {
            if(n == 1)
                return 0;
            else
                break;
        } else
            return -1;
    }
    *bufptr = '\0';
    return n;
}
