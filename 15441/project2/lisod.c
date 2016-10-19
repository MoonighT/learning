/******************************************************************************
 * echo_server.c                                                               *
 *                                                                             *
 * Description: This file contains the C source code for an echo server.  The  *
 *              server runs on a hard-coded port and simply write back anything*
 *              sent to it by connected clients.  It does not support          *
 *              concurrent clients.                                            *
 *                                                                             *
 * Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
 *          Wolf Richter <wolf@cs.cmu.edu>                                     *
 *                                                                             *
 *******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "log.h"
#include "hio.h"
#include "parser/parse.h"

#define ECHO_PORT 9998
#define BUF_SIZE 8192

typedef struct {
    int sessionid;
    char* data[FD_SETSIZE];
    int len;
}session;

typedef struct {
    fd_set readset;
    fd_set readresult;
    fd_set writeset;
    fd_set writeresult;
    int client[FD_SETSIZE];
    int maxfd;
    int maxi;
    int nready;
    char *www;
}poller;

static session sess[FD_SETSIZE];

int close_socket(int sock)
{
    if (close(sock))
    {
        logerr("Failed closing socket.\n");
        return 1;
    }
    return 0;
}

void Init() {
    init_log("./log/server.log");
}

void Deinit() {
    log_close();
}

int bind_listen(int port) {
    /* all networked programs must create a socket */
    int sock;
    struct sockaddr_in addr;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        logerr("Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        logerr("Failed binding socket.\n");
        return EXIT_FAILURE;
    }
    if (listen(sock, 5))
    {
        close_socket(sock);
        logerr("Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    return sock;
}

int OnNewConnection(int sock) {
    int client_sock;
    socklen_t cli_size;
    struct sockaddr_in cli_addr;
    cli_size = sizeof(cli_addr);
    if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr, &cli_size)) == -1)
    {
        close(sock);
        logerr("Error accepting connection.\n");
    } else {
        char *ip = inet_ntoa(cli_addr.sin_addr);
        loginfo("accept a new client. %s\n", ip);
    }
    return client_sock;
}

int senddata(int sock, char* buf, int count) {
    return send(sock, buf, count, 0);
    //return rio_writen(sock, buf, count);
}

bool validHeader(Request* req) {
    if (req->http_method != "GET" || req->http_method != "POST" ||
            req->http_method != "HEAD") {
        return false;
    }
    return true;
}

void clientError(int sock, char* number, char *message) {

}

char* getHeaderByKey(Request_header* headers, int count, char *key) {
    int i;
    for(i=0; i<count; i++) {
        if(!strcmp(headers[i].header_name, key)) {
            return headers[i].header_value;
        }
    }
    return NULL;
}

typedef struct {
    HttpStatus status;
    char header[BUF_SIZE];
    int header_length;
    char body[BUF_SIZE];
    int body_length;
} Response;

void serve_static(int sock, char* filename, Response* resp) {
    resp->status = HTTP_REPLY;
    char buf[4096];
    struct stat sbuf;
    if stat(filename, &sbuf) < 0 {
        resp->status = HTTP_ERROR;
        clientError(sock, "404", "missing file");
    }
    int filesize = sbuf.st_size;
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    sprintf(buf, "%sServer: Liso/1.0\r\n", buf);
    sprintf(buf, "%sDate:%s\r\n", buf, date);
    sprintf(buf, "%sConnection:keep-alive\r\n", buf);
    sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
    sprintf(buf, "%sLast-Modified:%s\r\n", buf, modify_time);
    sprintf(buf, "%sContent-Type: %s\r\n\r\n", buf, filetype);
    strcpy(resp->header, buf);
    resp->header_length = strlen(buf);

    int srcfd = open(filename, O_RDONLY, 0);
    char *srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
}

Response* parseHttp(int sock, char* buf, int size, char* www) {
    //parse http line
    //parse http header
    Response *resp = (Response*)malloc(sizeof(Response));
    HttpStatus status = HTTP_NORMAL;
    Request *request = parse(buf,size,sock);
    printf("Http Method %s\n",request->http_method);
    printf("Http Version %s\n",request->http_version);
    printf("Http Uri %s\n",request->http_uri);
    int index;
    for(index = 0;index < request->header_count;index++){
        printf("Request Header\n");
        printf("Header name %s Header Value %s\n",request->headers[index].header_name,request->headers[index].header_value);
    }
    if (!validHeader(requeset)) {
        resp->status = HTTP_ERROR;
        clientError(sock, "505", "Method not implement");
        return resp;
    }
    if(!strcmp(request->method, "POST")) {
        if(getHeaderByKey(request->headers, request->header_count, "Content-Length") == NULL) {
            resp->status = HTTP_ERROR;
            clientError(sock, "411", "post missing content length");
            return resp;
        }
    }
    //if post parse body
    //handle filename
    if(!strstr(request->http_uri, "cgi-bin")) {
        //static file
        char *filename;
        strcpy(filename, www);
        strcat(filename, request->http_uri);
        if(request->http_uri[strlen(request->http_uri)-1] == '/')
            strcat(filename, "index.html");
        if(strlen(filename) > 0) {
            serve_static(sock, filename, resp);
        }
    }
    return resp;
}

int handle_read(int client_sock, poller *p) {
    int readret = 0;
    char buf[BUF_SIZE];
    if((readret = rio_readn(client_sock, buf, BUF_SIZE, 0)) > 1) {
        //handle with buf and readret
        Response* resp = parseHttp(client_sock, buf, readret, p->www);
        //FD_SET(sock, &p->write_set);
    } else {
        loginfo("connection close by client.\n");
        return -1;
    }
    return 0;
}

int handle_write(int client_sock, poller *p) {
    for(i = 0; i < FD_SETSIZE; i++) {
        if(sess[i].sessionid == client_sock)
            break;
    }
    if(i == FD_SETSIZE) {
        //error no session data
        logerr("No session data for client sock = %d", client_sock);
        close_socket(client_sock);
        FD_CLR(client_sock, &p.writeset);
        p.client[i] = -1;
    }
    char *data = sess[i].data;
    int len = sess[i].len;
    senddata(client_sock, data, len);
    FD_CLR(client_sock, &p->writeset);
}

void poll(int sock) {
    int i, r, client_sock;
    ssize_t readret;
    poller p;
    //init and set allset
    p.maxfd = sock;
    p.maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        p.client[i] = -1;
    }
    FD_ZERO(&p.readset);
    FD_ZERO(&p.writeset);
    FD_SET(sock, &p.readset);
    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        p.readresult = p.readset;
        p.writeresult = p.writeset;
        p.nready = select(p.maxfd + 1, &p.readresult, &p.writeresult, NULL, NULL);
        if (FD_ISSET(sock, &p.readresult)) {
            //new connection
            client_sock = OnNewConnection(sock);
            for(i = 0; i < FD_SETSIZE; i++) {
                if(p.client[i] < 0) {
                    p.client[i] = client_sock;
                    sess[i].sessionid = client_sock;
                    break;
                }
            }
            if(FD_SETSIZE == i) {
                close(client_sock);
                logerr("Error too many connection.\n");
            } else {
                FD_SET(client_sock, &p.readset);
                if (client_sock > p.maxfd)
                    p.maxfd = client_sock;
                if (i > p.maxi)
                    p.maxi = i;
                if (--p.nready <= 0)
                    continue;
            }
        }
        //read or write
        for (i = 0; i <= p.maxi; i++) {
            if ((client_sock = p.client[i]) < 0)
                continue;
            if (FD_ISSET(client_sock, &p.readresult))	{
                //handle read
                r = handle_read(client_sock, &p);
                if (r < 0) {
                    close_socket(client_sock);
                    FD_CLR(client_sock, &p.readset);
                    p.client[i] = -1;
                }
                if (--p.nready <= 0)
                    break;
            }

            if (FD_ISSET(client_sock, &p.writeresult)) {
                handle_write(client_sock, &p);
            }
        }
    }
}

int main(int argc, char* argv[])
{
    Init();
    int sock;
    fprintf(stdout, "----- Echo Server -----\n");
    loginfo("fd set size = %d\n", FD_SETSIZE);
    sock = bind_listen(ECHO_PORT);
    poll(sock);
    Deinit();
    return EXIT_SUCCESS;
}
