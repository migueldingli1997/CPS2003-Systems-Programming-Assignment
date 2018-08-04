#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "Client.h"
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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s hostname port\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    // Check server port
    const int serverPort = atoi(argv[2]);
    if (serverPort < 1024) {
        fprintf(stderr, "Invalid port; Use a port greater or equal to 1024.\n");
        exit(EXIT_SUCCESS);
    }

    // Set exit handlers
    atexit(exit_closeSocket);
    atexit(exit_closeGame);

    //--------------------------------------------------------------------------------

    // Open socket
    joinGame(argv[1], serverPort);

    // Set up ncurses
    initscr();      // start curses mode
    noecho();       // do not show user input
    curs_set(0);    // hide cursor
    start_color();  // enable colours
    init_pair(1, COLOR_BLACK, COLOR_WHITE); // define colour pair 1 (for backgr. and foregr.)
    init_pair(2, COLOR_RED, COLOR_WHITE);   // define colour pair 2 (for snake head)
    wbkgd(stdscr, COLOR_PAIR(1));           // set background and foreground

    // Start game
    initGame();
    startGame();

    return 0;
}
