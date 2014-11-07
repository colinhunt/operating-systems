#ifndef FORKED_HANDLER_H
#define FORKED_HANDLER_H

#include <netinet/in.h>

/* Handle a request concurrently.
* listenfd is the server socket. sendfd is the client socket.
* Responsible for closing sendfd when done.
* If this forks, the child will close listenfd.
* Log to logFileName. */
void handleConcurrently(int listenfd, int sendfd, struct sockaddr_in clientAddr,
		char logFileName[]);

#endif /* FORKED_HANDLER_H */