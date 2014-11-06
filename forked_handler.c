#include "forked_handler.h"

#include <unistd.h>
#include <stdio.h>
#include "request_handler.h"


void handleConcurrently(int sendfd, struct sockaddr_in clientAddr, char logFileName[]) {
    handleRequest(sendfd, clientAddr, logFileName);

    fflush(stdout);
    sleep(1);
    close(sendfd);
}
