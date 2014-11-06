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
#include <arpa/inet.h>

static const unsigned BUFSIZE = 512;

typedef struct {
    char* clientIP;
    int sendfd;
    char* message;
    char* body;
    char* requestLine;
} Response;


size_t formatedDate(char *dateBuf, size_t size) {
    time_t timer = time(NULL);
    return strftime(dateBuf, size, "%a %d %b %Y %H:%M:%S %Z", gmtime(&timer));
}

void sendHeader(int sendfd, char message[], size_t lenBody) {
    char header[BUFSIZE];
    char date[BUFSIZE];
    size_t pos = 0;
    formatedDate(date, BUFSIZE);
    pos += sprintf(header + pos, "HTTP/1.1 %s\n", message);
    pos += sprintf(header + pos, "Date: %s\n", date);
    pos += sprintf(header + pos, "Content-Type: text/html\n");
    pos += sprintf(header + pos, "Content-Length: %u\n\n", lenBody);
    printf(header);
    write(sendfd, header, pos);
}

void sendTextBody(Response r) {
    size_t lenBody = strlen(r.body);
    sendHeader(r.sendfd, r.message, lenBody);
    write(r.sendfd, r.body, lenBody);
//    logger(r.clientIP, r.requestLine, r.message);
}

void sendNotFound(Response r) {
    perror("Not found!!!");
    r.message = "404 Not Found";
    r.body = "<html><body>\n"
             "<h2>Document not found</h2>\n"
             "You asked for a document that doesn't exist. That is so sad.\n"
             "</body></html>\n";
    sendTextBody(r);
}

void sendBadRequest(Response r) {
    r.message = "400 Bad Request";
    r.body = "<html><body>\n"
            "<h2>Malformed Request</h2>\n"
            "Your browser sent a request I could not understand.\n"
            "</body></html>\n";
    sendTextBody(r);
}

void sendForbidden(Response r) {
    r.message = "403 Forbidden";
    r.body = "<html><body>\n"
            "<h2>Permission Denied</h2>\n"
            "You asked for a document you are not permitted to see. It sucks to be you.\n"
            "</body></html>\n";
    sendTextBody(r);
}

void sendServerError(Response r) {
    r.message = "500 Internal Server Error";
    r.body = "<html><body>\n"
            "<h2>Oops. That Didn't work</h2>\n"
            "I had some sort of problem dealing with your request. Sorry, I'm lame.\n"
            "</body></html>\n";
    sendTextBody(r);
}

int blankLineTerminated(char buffer[], ssize_t n) {
    if (buffer[n - 1] != '\n') {
        return 0;
    }
    if (buffer[n - 2] == '\r') {
        if (buffer[n - 3] != '\n') {
            return 0;
        }
    } else if (buffer[n - 2] != '\n') {
        return 0;
    }

    return 1;
}

void handleRequest(int sendfd, char clientIP[]) {
    unsigned int i;
    char buffer[BUFSIZE + 1];
    bzero(buffer, BUFSIZE + 1);
    ssize_t n = read(sendfd, buffer, BUFSIZE);
    Response response;
    response.clientIP = clientIP;
    response.sendfd = sendfd;

    if (n <= 0) {
        perror("ERROR reading from socket");
        return sendServerError(response);
    }

    if (n < BUFSIZE) {
        buffer[n] = 0;
    } else {
        buffer[0] = 0;
    }

    if (!blankLineTerminated(buffer, n))
        return sendBadRequest(response);

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
        return sendBadRequest(response);
    }

    // check for HTTP/1.1 at the end
    ssize_t fileNameEnd = n - 9;
    if (strncmp(&buffer[fileNameEnd], " HTTP/1.1", 9) != 0) {
        printf("Invalid request, not HTTP %d\n", n);
        return sendBadRequest(response);
    }

    // now we know the line is formatted like "GET /(.*) HTTP/1.1"
    char reqLine[n + 1];
    ssize_t fnLen = fileNameEnd - fileNameStart;
    char fileName[fnLen + 1];
    strncpy(reqLine, buffer, n);
    reqLine[n] = 0;
    strncpy(fileName, &buffer[fileNameStart], fnLen);
    fileName[fnLen] = 0;

    printf(fileName);

    int fd;
    if ((fd = open(fileName, O_RDONLY)) != -1) {
        struct stat stat_buf;
        fstat(fd, &stat_buf);

//        printf("%ld\n", stat_buf.st_size);
        sendHeader(sendfd, "200 OK", stat_buf.st_size);

        /* copy file using sendfile */
        off_t offset = 0;
        ssize_t rc = sendfile(sendfd, fd, &offset, stat_buf.st_size);
        if (rc == -1) {
            close(fd);
            return sendServerError(response);
        } else {
            printf("Success!!!\n");
        }
        close(fd);

    } else {
        if (errno == EACCES)
            return sendForbidden(response);
        else
            return sendNotFound(response);
    }
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
        length = sizeof(clientAddr);
        sendfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length);
        if (sendfd < 0) {
            perror("Server: accept failed");
            exit(1);
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        // here we would call whatever concurrent handler with the sendfd
        //  which would then call the handleRequest func and cleanup after itself
        handleRequest(sendfd, clientIP);

        // we shouldn't have to do any of the below in this loop
        fflush(stdout);
        sleep(1);
        close(sendfd);
    }

    return 0;
}