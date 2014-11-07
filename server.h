#include <netinet/in.h>

#ifndef SERVER_H
#define SERVER_H

int startServer(int argc, char **argv, void (*fun)(int, int, struct sockaddr_in, char[]));

#endif /* SERVER_H */