#ifndef THREADED_HANDLER_H
#define THREADED_HANDLER_H

#include <netinet/in.h>

void handleConcurrently(int listenfd, int sendfd, struct sockaddr_in clientAddr, char logFileName[]);

#endif /* THREADED_HANDLER_H */