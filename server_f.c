#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <time.h>

static const unsigned BUFSIZE = 512;


size_t formatedDate(char *dateBuf, size_t size) {
    time_t timer = time(NULL);
    return strftime(dateBuf, size, "%a %d %b %Y %H:%M:%S %Z", gmtime(&timer));
}

int httpHeader(char *header, unsigned len) {
    int pos = 0;
    char date[BUFSIZE];
    formatedDate(date, BUFSIZE);
    pos += sprintf(header + pos, "HTTP/1.1 200 OK\n");
    pos += sprintf(header + pos, "Date: %s\n", date);
    pos += sprintf(header + pos, "Content-Type: text/html\n");
    pos += sprintf(header + pos, "Content-Length: %u\n\n", len);
    return pos;
}

int main(int argc, char **argv) {
    unsigned i;
    int fd, port, pid, listenfd, sendfd, optval;
    socklen_t length;
    static struct sockaddr_in clientAddr;
    static struct sockaddr_in serverAddr;


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

    optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

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

    while (1) {
        char buffer[BUFSIZE+1];
        length = sizeof(clientAddr);
        sendfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
        if (sendfd < 0) {
            perror("Server: accept failed");
            exit(1);
        }

        bzero(buffer,BUFSIZE+1);
        unsigned n = read(sendfd, buffer, BUFSIZE);
        if (n <= 0)
        {
            perror("ERROR reading from socket");
            exit(1);
        }

        if (n < BUFSIZE) {
            buffer[n]=0;
        } else {
            buffer[0]=0;
        }

//        printf(buffer);

//        printf(buffer);
        if (buffer[n-1] != '\n')
            printf("Invalid request %d\n", n);
//        printf("blah\n");
//        printf(buffer);
// parse request, ensure valid, get filename

        // terminate after the first newline
        for (i = 0; i < n; i++) {
            if (buffer[i] == '\n') {
                buffer[i] = 0;
                n = i;
                if (buffer[i - 1] == '\r') {
                    buffer[i - 1] = 0;
                    n = i - 1;
                }
                break;
            }
        }
//        printf(buffer);

        unsigned fileNameStart = 5;
        if (strncmp(buffer, "GET /", fileNameStart) != 0 && strncmp(buffer, "get /", fileNameStart) != 0) {
            printf("Invalid request, not a GET %d\n", n);
        }

        // check for HTTP/1.1 at the end
        unsigned fileNameEnd = n - 9;
        if (strncmp(&buffer[fileNameEnd], " HTTP/1.1", 9) != 0) {
            printf("Invalid request, not HTTP %d\n", n);
            printf(&buffer[fileNameEnd]);
        }

        // now we know the line is formatted like "GET /(.*) HTTP/1.1"
        buffer[fileNameEnd] = 0;
//        printf(&buffer[fileNameStart]);
        char* fileName = &buffer[fileNameStart];

        printf(fileName);

        if ((fd = open(fileName, O_RDONLY)) == -1) {
            perror("Not found!!!");
        }

        struct stat stat_buf;
        fstat(fd, &stat_buf);

//        printf("%ld\n", stat_buf.st_size);
        char header[BUFSIZE];

        int len = httpHeader(header, stat_buf.st_size);
        printf(header);
        n = write(sendfd, header, len);

        /* copy file using sendfile */
        off_t offset = 0;
        int rc = sendfile(sendfd, fd, &offset, stat_buf.st_size);
        if (rc == -1) {
            perror("Error trying to send file");
        } else {
            printf("Success!!!\n");
        }


//        printf("%d:", n);
        fflush(stdout);
        sleep(1);
        close(sendfd);
        close(fd);
    }

    return 0;
}