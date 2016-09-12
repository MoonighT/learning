#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


/* $begin signal1 */
//#include "csapp.h"

void handler1(int sig) 
{
    pid_t pid;

    if ((pid = waitpid(-1, NULL, 0)) < 0)
	printf("waitpid error");
    printf("Handler reaped child %d\n", (int)pid);
    sleep(2);
    return;
}

int main() 
{
    int i, n;
    char buf[255];

    if (signal(SIGCHLD, handler1) == SIG_ERR)
	printf("signal error");
printf("Hello from parent %d\n", (int)getpid());
    /* Parent creates children */
    for (i = 0; i < 3; i++) {
	if (fork() == 0) { 
	    printf("Hello from child %d\n", (int)getpid());
	    sleep(1);
	    exit(0);
	}
    }

    /* Parent waits for terminal input and then processes it */
    ////if ((n = read(STDIN_FILENO, buf, sizeof(buf))) < 0)
	//unix_error("read");
	gets(buf);

    printf("Parent processing input\n");
    while (1)
	; 

    exit(0);
}
/* $end signal1 */
