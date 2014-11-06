#ifndef FORKED_HANDLER_H
#define FORKED_HANDLER_H

#include <netinet/in.h>

void handleConcurrently(int sendfd, struct sockaddr_in clientAddr, char logFileName[]);

#endif /* FORKED_HANDLER_H */