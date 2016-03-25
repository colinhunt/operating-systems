# C File Server
A simple HTTP 1.1 concurrent file server written in C.

This is a web server which implements a minimal subset of
the HTTP 1.1 protocol.

## Build
Build the server using
  
    make all

## Run
The web server is started with three command line arguments. 

    ./server_[f|p] <port> </serve/files/directory> </log/file>

Specify `server_f` for the forking current handler, or `server_p` for the threaded handler. 
The first argument is a TCP port on which it will listen to service requests made by clients.
The second argument is a directory from where the server serves documents to
the clients. The last argument is a log file in which the server logs all
transactions with all clients.
