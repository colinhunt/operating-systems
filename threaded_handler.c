/* name:               Colin Hunt
 * ONE Card number:    1222665
 * Unix id:            colin
 * lecture section:    A1
 * instructor's name:  Mohammad Bhuiyan
 * lab section:        D05

Implements concurrent handling using threads.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "concurrent_handler.h"
#include "request_handler.h"

/* Params for a thread. */
typedef struct {
	int sendfd;
	struct sockaddr_in clientAddr;
	char *logFileName;
} Params;

/* Thread handler. */
void *handle(void *params);

void *handle(void *params) {
	Params *p = params;
	handleRequest(p->sendfd, p->clientAddr, p->logFileName);
	sleep(1);
	close(p->sendfd);
	free(p);
	return NULL;
}

void handleConcurrently(int listenfd, int sendfd, struct sockaddr_in clientAddr,
		char logFileName[]) {
	pthread_t thread;
	Params *params;

	params = malloc(sizeof(Params));
	if (params == NULL) {
		perror("Error allocating memory for thread arg");
		return;
	}

	params->sendfd = sendfd;
	params->clientAddr = clientAddr;
	params->logFileName = logFileName;

	if (pthread_create(&thread, NULL, handle, params) != 0) {
		perror("Failed to create thread\n");
	}
}