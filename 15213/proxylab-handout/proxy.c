#include <stdio.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";
void processConn(void* clientInfo);
void serve(int connfd);
void readHeaders(rio_t *rio,  char *otherheaders);

typedef struct clientInfo_t {
		struct sockaddr_in clientaddr;
		int connfd;
} clientInfo_t;


int main(int argc, char** argv)
{
    printf("%s", user_agent_hdr);
		int listenfd, client_len;
		struct clientInfo_t *clientInfo;
		//check argc number
		if (argc != 2) {
				fprintf(stderr, "usage : %s <port num>", argv[0]);
				return 1;
		}
		//ignore sigpipe
		Signal(SIGPIPE, SIG_IGN);
		listenfd = Open_listenfd(argv[1]);
		printf("listenfd = %d\n", listenfd);

		while (1) {
				//Accpet
				clientInfo = Malloc(sizeof(struct clientInfo_t));
				client_len = sizeof(struct sockaddr_in);
				clientInfo->connfd = Accept(listenfd, (SA *)&clientInfo->clientaddr, (socklen_t *)&client_len);
				processConn((void*)clientInfo);
		}

    return 0;
}

void processConn(void* clientData) {
		struct clientInfo_t *clientInfo;
		clientInfo = (clientInfo_t *)clientData;
		int clientfd = clientInfo->connfd;
		printf("clientfd = %d\n", clientfd);

		struct sockaddr_in clientaddr = clientInfo->clientaddr;
		unsigned short clientPort = ntohs(clientaddr.sin_port);
		char *clientIp = inet_ntoa(clientaddr.sin_addr);
		printf("request from client ip=%s, port=%d\n", clientIp, clientPort);

		//free clientinfo made for each routine
		Free(clientData);
		serve(clientfd);
}

void serve(int connfd) {
		char method[MAXLINE], url[MAXLINE], version[MAXLINE];
		char buf[MAXLINE];
		char headers[MAXLINE], otherheaders[MAXLINE];
		rio_t rio;
		Rio_readinitb(&rio, connfd);
		Rio_readlineb(&rio, buf, MAXLINE);
		sscanf(buf, "%s %s %s", method, url, version);
		printf("method = %s, url = %s, version = %s\n", method, url, version);
		if(strcasencmp(method, "GET", 3)) {
				//output error to client
				return;
		}
		readHeaders(rio, headers, otherheaders);
}

void readHeaders(rio_t *rio, char *otherheaders) {
			
}
