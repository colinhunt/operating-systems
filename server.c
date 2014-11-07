/* name:               Colin Hunt
 * ONE Card number:    1222665
 * Unix id:            colin
 * lecture section:    A1
 * instructor's name:  Mohammad Bhuiyan
 * lab section:        D05

This is a web server which implements a minimal subset of
the HTTP 1.1 protocol.
The web server is started with three command line arguments. The first argument
is a TCP port on which it will listen to service requests made by clients.
The second argument is a directory from where the server serves documents to
the clients. The last argument is a log file in which the server logs all
transactions with all clients.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

/* We can change the handler by linking a different one that implements this
   header */
#include "concurrent_handler.h"


int main(int argc, char **argv) {
	int fd, port, listenfd, sendfd, optval;
	socklen_t length;
	static struct sockaddr_in clientAddr, serverAddr;

	/* Don't terminate on broken pipes */
	signal(SIGPIPE, SIG_IGN);

	if (argc < 4 || argc > 4) {
		printf("Usage:\n");
		printf("\tserver_[f|p] <port> </serve/files/directory> "
				"</log/file>\n");
		exit(EXIT_FAILURE);
	}

	/* Change to the serving directory */
	if (chdir(argv[2]) == -1) {
		printf("%s is an invalid directory.\n", argv[2]);
		exit(EXIT_FAILURE);
	}

	char logFileName[strlen(argv[3]) + 1];
	strncpy(logFileName, argv[3], strlen(argv[3]) + 1);

	/* Verify log file access. */
	if ((fd = open(logFileName, O_CREAT, 0644)) != -1) {
		close(fd);
	} else {
		perror("Error trying to create/access log file");
		exit(errno);
	}

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error trying to open socket for listening");
		exit(errno);
	}

	/* Reuse same address. */
	optval = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	port = atoi(argv[1]);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons((uint16_t) port);

	if (bind(listenfd, (struct sockaddr *) &serverAddr,
			sizeof(serverAddr)) < 0) {
		perror("Error trying to bind to socket");
		exit(EXIT_FAILURE);
	}

	if (listen(listenfd, 200) < 0) {
		perror("Error listening to socket");
		exit(EXIT_FAILURE);
	}

	/* Disappear, into the night. */
	if (daemon(1, 0) == -1) {
		perror("Error daemonizing");
		exit(EXIT_FAILURE);
	}

	/* Accept requests and hand them off to our concurrent handler. */
	while (1) {
		length = sizeof(clientAddr);
		sendfd = accept(listenfd, (struct sockaddr *) &clientAddr,
				&length);
		if (sendfd < 0) {
			perror("Server: accept failed");
			exit(1);
		}

		handleConcurrently(listenfd, sendfd, clientAddr, logFileName);
	}
}