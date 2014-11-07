/* name:               Colin Hunt
 * ONE Card number:    1222665
 * Unix id:            colin
 * lecture section:    A1
 * instructor's name:  Mohammad Bhuiyan
 * lab section:        D05

Implements concurrent handling using fork().
 */

#include "concurrent_handler.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "request_handler.h"

/* Register child handler to reap zombie processes */
int setupChildHandler();

int setupChildHandler() {
    static int set = 0;
    if (!set) {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            perror("Error registering child handler");
            exit(EXIT_FAILURE);
        }
        set = 1;
    }
    return 1;
}

void handleConcurrently(int listenfd, int sendfd, struct sockaddr_in clientAddr, char logFileName[]) {
    pid_t pid;

    setupChildHandler();
    if ((pid = fork()) < 0) {
        perror("Error forking");
    } else {
        if (pid == 0) { /* child */
            close(listenfd);

            handleRequest(sendfd, clientAddr, logFileName);

            sleep(1);
            exit(EXIT_SUCCESS);
        }
        close(sendfd);
    }
}
