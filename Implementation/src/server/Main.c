#include <netinet/in.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "Server.h"
#include "Game/Game.h"

int main(int argc, char *argv[]) {

    // Set randomness
    srand((unsigned int) time(NULL));

    // Set signal handler
    struct sigaction sa;
    sa.sa_handler = terminationHandler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGTSTP, &sa, NULL) == -1 || sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Couldn't set SIGTSTP or SIGINT handler");
        exit(EXIT_FAILURE);
    }

    // Ignore SIGPIPE (handled manually)
    signal(SIGPIPE, SIG_IGN);

    //--------------------------------------------------------------------------------

    // Make sure that number of command line arguments is correct
    if (argc != 4) {
        fprintf(stderr, "Usage: %s port mapWidth mapHeight\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    // Check server port
    const int serverPort = atoi(argv[1]);
    if (serverPort < 1024) {
        fprintf(stderr, "Invalid port; Use a port greater or equal to 1024.\n");
        exit(EXIT_SUCCESS);
    }

    // Check map dimensions
    const int mapWidth = atoi(argv[2]);
    const int mapHeight = atoi(argv[3]);
    if (mapWidth < 10 || mapHeight < 10) {
        fprintf(stderr, "Invalid map width or height; Use values greater or equal to 10.\n");
        exit(EXIT_SUCCESS);
    }

    // Set exit handlers
    atexit(exit_closeServerSocket);
    atexit(exit_closeClientSockets);
    atexit(exit_closeGame);

    //--------------------------------------------------------------------------------

    initGame(mapWidth, mapHeight);
    startListeningForClients(serverPort);
    startGame();

    return 0;
}