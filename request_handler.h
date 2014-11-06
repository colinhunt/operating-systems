#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <netinet/in.h>

#define BUFSIZE 512

typedef struct {
    int sendfd;
    char clientIP[INET_ADDRSTRLEN];
    char message[BUFSIZE];
    const char* body;
    const char* requestLine;
    const char* logFileName;
} Response;

void handleRequest(int sendfd, struct sockaddr_in clientAddr, const char *logFileName);

#endif /* REQUEST_HANDLER_H */