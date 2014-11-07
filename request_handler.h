/* name:               Colin Hunt
 * ONE Card number:    1222665
 * Unix id:            colin
 * lecture section:    A1
 * instructor's name:  Mohammad Bhuiyan
 * lab section:        D05

Request handling library.
 */

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <netinet/in.h>

/* Handle a request by a client.
* sendfd is the client socket.
* Log information in logFileName. */
void handleRequest(int sendfd, struct sockaddr_in clientAddr,
		const char *logFileName);

#endif /* REQUEST_HANDLER_H */