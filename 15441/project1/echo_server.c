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

#define ECHO_PORT 9998
#define BUF_SIZE 8192

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    int i, sock, client_sock, maxfd, maxi, nready;
		int client[FD_SETSIZE];
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char buf[BUF_SIZE];
		fd_set rset, allset;

    fprintf(stdout, "----- Echo Server -----\n");
    fprintf(stdout, "fd set size = %d\n", FD_SETSIZE);
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(sock, 5))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }
		//init and set allset
	  maxfd = sock;
		maxi = -1;
		for (i = 0; i < FD_SETSIZE; i++) {
				client[i] = -1;
		}
		FD_ZERO(&allset);
		FD_SET(sock, &allset);
    /* finally, loop waiting for input and then write it back */
    while (1)
    {
			 rset = allset;
			 nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
			 if (FD_ISSET(sock, &rset)) {
					//new connection
       		cli_size = sizeof(cli_addr);
					if ((client_sock = accept(sock, (struct sockaddr *) &cli_addr,
                                 &cli_size)) == -1)
       		{
           		close(sock);
           		fprintf(stderr, "Error accepting connection.\n");
       		} else {
					 		char *ip = inet_ntoa(cli_addr.sin_addr);
					 		fprintf(stdout, "accept a new client. %s\n", ip);
			 		}
					for(i = 0; i < FD_SETSIZE; i++) {
							if(client[i] < 0) {
									client[i] = client_sock;
									break;
							}
					}
					if(FD_SETSIZE == i) {
							close(client_sock);
           		fprintf(stderr, "Error too many connection.\n");
					} else {
							FD_SET(client_sock, &allset);	
							if (client_sock > maxfd)
									maxfd = client_sock;
							if (i > maxi)
									maxi = i;
							if (--nready <= 0)
									continue;
					}
			 }
			 for (i = 0; i <= maxi; i++) {
						if ((client_sock = client[i]) < 0)
								continue;
						if (FD_ISSET(client_sock, &rset))	{
       					readret = 0;
								if((readret = recv(client_sock, buf, BUF_SIZE, 0)) > 1) {
           					if (send(client_sock, buf, readret, 0) != readret) {
               					close_socket(client_sock);
												FD_CLR(client_sock, &allset);
												client[i] = -1;
               					fprintf(stderr, "Error sending to client.\n");
           					}
       					} else {
										close_socket(client_sock);
										FD_CLR(client_sock, &allset);
										client[i] = -1;
               			fprintf(stdout, "connection close by client.\n");
								}

								if (--nready <= 0)
										break;
						}
			 }
		}
    return EXIT_SUCCESS;
}
