#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <netinet/in.h>

void handleRequest(int sendfd, struct sockaddr_in clientAddr, const char *logFileName);

#endif /* REQUEST_HANDLER_H */