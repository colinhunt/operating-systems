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
#include <signal.h>

#define BUFSIZE 512

typedef struct {
    int sendfd;
    char clientIP[INET_ADDRSTRLEN];
    char message[BUFSIZE];
    const char* body;
    const char* requestLine;
    const char* logFileName;
} Response;


size_t formatedDate(char *dateBuf, size_t size) {
    time_t timer = time(NULL);
    return strftime(dateBuf, size, "%a %d %b %Y %H:%M:%S %Z", gmtime(&timer));
}

void sendHeader(int sendfd, char const *message, size_t lenBody) {
    char header[BUFSIZE];
    char date[BUFSIZE];
    size_t pos = 0;
    formatedDate(date, BUFSIZE);
    pos += sprintf(header + pos, "HTTP/1.1 %s\n", message);
    pos += sprintf(header + pos, "Date: %s\n", date);
    pos += sprintf(header + pos, "Content-Type: text/html\n");
    pos += sprintf(header + pos, "Content-Length: %lu\n\n", lenBody);
    printf(header);
    write(sendfd, header, pos);
}

void logger(Response r) {
    FILE *fd;
    char date[BUFSIZE];
    formatedDate(date, BUFSIZE);
    if ((fd = fopen(r.logFileName, "a")) >= 0) {
        fprintf(fd, "%s\t%s\t%s\t%s\n", date, r.clientIP, r.requestLine, r.message);
        fclose(fd);
    } else {
        perror("Can't log!!");
    }
}

void sendTextBody(Response r) {
    size_t lenBody = strlen(r.body);
    sendHeader(r.sendfd, r.message, lenBody);
    write(r.sendfd, r.body, lenBody);
    logger(r);
}

void sendNotFound(Response r) {
    perror("Not found!!!");
    strncpy(r.message, "404 Not Found", sizeof(r.message));
    r.body = "<html><body>\n"
             "<h2>Document not found</h2>\n"
             "You asked for a document that doesn't exist. That is so sad.\n"
             "</body></html>\n";
    sendTextBody(r);
}

void sendBadRequest(Response r) {
    strncpy(r.message, "400 Bad Request", sizeof(r.message));
    r.body = "<html><body>\n"
            "<h2>Malformed Request</h2>\n"
            "Your browser sent a request I could not understand.\n"
            "</body></html>\n";
    sendTextBody(r);
}

void sendForbidden(Response r) {
    strncpy(r.message, "403 Forbidden", sizeof(r.message));
    r.body = "<html><body>\n"
            "<h2>Permission Denied</h2>\n"
            "You asked for a document you are not permitted to see. It sucks to be you.\n"
            "</body></html>\n";
    sendTextBody(r);
}

void sendServerError(Response r) {
    strncpy(r.message, "500 Internal Server Error", sizeof(r.message));
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

void handleRequest(int sendfd, struct sockaddr_in clientAddr, const char *logFileName) {
    unsigned int i;
    char buffer[BUFSIZE + 1];
    bzero(buffer, BUFSIZE + 1);
    ssize_t n = read(sendfd, buffer, BUFSIZE);
    Response response;

    inet_ntop(AF_INET, &clientAddr.sin_addr, response.clientIP, INET_ADDRSTRLEN);

    response.sendfd = sendfd;
    response.logFileName = logFileName;
    response.requestLine = "<malformed request>";

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
    char reqLine[n + 1];
    strncpy(reqLine, buffer, (size_t) n);
    reqLine[n] = 0;

    response.requestLine = reqLine;

    size_t fileNameStart = 5;
    if (strncmp(buffer, "GET /", fileNameStart) != 0 && strncmp(buffer, "get /", fileNameStart) != 0) {
        printf("Invalid request, not a GET.\n");
        return sendBadRequest(response);
    }

    // check for HTTP/1.1 at the end
    size_t fileNameEnd = (size_t) (n - 9);
    if (strncmp(&buffer[fileNameEnd], " HTTP/1.1", 9) != 0) {
        printf("Invalid request, not HTTP/1.1.");
        return sendBadRequest(response);
    }

    // now we know the line is formatted like "GET /(.*) HTTP/1.1"
    size_t fnLen = fileNameEnd - fileNameStart;
    char fileName[fnLen + 1];
    strncpy(fileName, &buffer[fileNameStart], fnLen);
    fileName[fnLen] = 0;

    printf(fileName);

    int fd;
    if ((fd = open(fileName, O_RDONLY)) != -1) {
        struct stat stat_buf;
        fstat(fd, &stat_buf);

//        printf("%ld\n", stat_buf.st_size);
        sendHeader(sendfd, "200 OK", (size_t) stat_buf.st_size);

        /* copy file using sendfile */
        off_t offset = 0;
        ssize_t rc = sendfile(sendfd, fd, &offset, (size_t) stat_buf.st_size);
        if (rc == -1) {
            close(fd);
            return sendServerError(response);
        } else {
            printf("Success!!!\n");
            sprintf(response.message, "200 OK %li/%li", rc, stat_buf.st_size);
            printf(response.message);
            logger(response);
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
    int fd, port, listenfd, sendfd, optval;
    socklen_t length;
    static struct sockaddr_in clientAddr;
    static struct sockaddr_in serverAddr;

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
        handleRequest(sendfd, clientAddr, logFileName);

        // we shouldn't have to do any of the below in this loop
        fflush(stdout);
        sleep(1);
        close(sendfd);
    }
}