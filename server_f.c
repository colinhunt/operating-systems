#include "server.h"
#include "concurrent_handler.h"

int main(int argc, char **argv) {
    return startServer(argc, argv, handleConcurrently);
}
