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
}

int parseHttp(int sock, char* buf, int size) {
    //parse http line
    Request *request = parse(buf,size,sock);
    printf("Http Method %s\n",request->http_method);
    printf("Http Version %s\n",request->http_version);
    printf("Http Uri %s\n",request->http_uri);
    int index;
    for(index = 0;index < request->header_count;index++){
        printf("Request Header\n");
        printf("Header name %s Header Value %s\n",request->headers[index].header_name,request->headers[index].header_value);
    }
    //parse http header
    //if post parse body
    return 0;
}

int handle_read(int client_sock) {
    int readret = 0;
    char buf[BUF_SIZE];
    if((readret = recv(client_sock, buf, BUF_SIZE, 0)) > 1) {
        //handle with buf and readret
        parseHttp(client_sock, buf, readret);
        senddata(client_sock, buf, readret);
    } else {
        loginfo("connection close by client.\n");
        return -1;
    }
    return 0;
}

typedef struct {
    fd_set readset;
    fd_set readresult;
    fd_set writeset;
    fd_set writeresult;
    int client[FD_SETSIZE];
    int maxfd;
    int maxi;
    int nready;
}poller;

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
    FD_SET(sock, &p.readset);
    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        p.readresult = p.readset;
        p.nready = select(p.maxfd + 1, &p.readresult, NULL, NULL, NULL);
        if (FD_ISSET(sock, &p.readresult)) {
            //new connection
            client_sock = OnNewConnection(sock);
            for(i = 0; i < FD_SETSIZE; i++) {
                if(p.client[i] < 0) {
                    p.client[i] = client_sock;
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
                r = handle_read(client_sock);
                if (r < 0) {
                    close_socket(client_sock);
                    FD_CLR(client_sock, &p.readset);
                    p.client[i] = -1;
                }
                if (--p.nready <= 0)
                    break;
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
