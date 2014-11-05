#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>

static const unsigned BUFSIZE = 8096;

int main(int argc, char **argv) {
    int i, fd, port, pid, listenfd, sendfd;
    socklen_t length;
    static struct sockaddr_in clientAddr;
    static struct sockaddr_in serverAddr;

    if (argc < 4 || argc > 4) {
        printf("Usage:\n");
        printf("\tserver_[f|p] <port> </serve/files/directory> </log/file>\n");
        exit(EXIT_FAILURE);
    }

    if (chdir(argv[2]) == -1) {
        printf("%s is an invalid directory.\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    if ((fd = open(argv[3], O_CREAT, 0644)) != -1) {
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

    port = atoi(argv[1]);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error trying to bind to socket");
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 200) < 0) {
        perror("Error listening to socket");
        exit(EXIT_FAILURE);
    }


    char message[] = "Hello\n";
    char buffer[BUFSIZE+1];
    while (1) {
        length = sizeof(clientAddr);
        sendfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
        if (sendfd < 0) {
            perror("Server: accept failed");
            exit(1);
        }

        bzero(buffer,BUFSIZE+1);
        int n = read(sendfd, buffer, BUFSIZE);
        if (n <= 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        n = write(sendfd, "Hello\n", 6);

        printf("%d", n);
        fflush(stdout);
        sleep(1);
        close(sendfd);
    }

    return 0;
}