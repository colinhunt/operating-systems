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

#include "concurrent_handler.h"


int main(int argc, char **argv) {
    int fd, port, listenfd, sendfd, optval;
    socklen_t length;
    static struct sockaddr_in clientAddr;
    static struct sockaddr_in serverAddr;

//    if (daemon(1, 1) == -1) {
//        perror("Error daemonizing");
//        exit(EXIT_FAILURE);
//    }

    signal(SIGPIPE, SIG_IGN);
//    printf(header);
//    fflush(stdout);

    if (argc < 4 || argc > 4) {
        printf("Usage:\n");
        printf("\tserver_[f|p] <port> </serve/files/directory> </log/file>\n");
        exit(EXIT_FAILURE);
    }

    if (chdir(argv[2]) == -1) {
        printf("%s is an invalid directory.\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    char logFileName[strlen(argv[3]) + 1];
    strncpy(logFileName, argv[3], strlen(argv[3]) + 1);

    if ((fd = open(logFileName, O_CREAT, 0644)) != -1) {
        printf("%d\n", fd);
        close(fd);
    } else {
        perror("Error trying to create/access log file");
        exit(errno);
    }

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error trying to open socket for listening");
        exit(errno);
    }

    optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    port = atoi(argv[1]);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons((uint16_t) port);

    if (bind(listenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error trying to bind to socket");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 200) < 0) {
        perror("Error listening to socket");
        exit(EXIT_FAILURE);
    }

    while (1) {
        length = sizeof(clientAddr);
        sendfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
        if (sendfd < 0) {
            perror("Server: accept failed");
            exit(1);
        }

        // here we would call whatever concurrent handler with the sendfd
        //  which would then call the handleRequest func and cleanup after itself
        handleConcurrently(listenfd, sendfd, clientAddr, logFileName);
    }
}