#include "server.h"
#include "threaded_handler.h"

int main(int argc, char **argv) {
    return startServer(argc, argv, handleConcurrently);
}
