#include "forked_handler.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "request_handler.h"


void handleConcurrently(int listenfd, int sendfd, struct sockaddr_in clientAddr, char logFileName[]) {
    pid_t pid;
    if ((pid = fork()) < 0) {
        perror("Error forking");
    } else {
        if (pid == 0) { /* child */
            close(listenfd);

            handleRequest(sendfd, clientAddr, logFileName);

            printf("\nChild %d exiting\n", getpid());
            fflush(stdout);
            sleep(1);
            exit(EXIT_SUCCESS);
        }
        close(sendfd);
    }
}
