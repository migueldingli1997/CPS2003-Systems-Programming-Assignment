#include <ncurses.h>
#include <stdlib.h>
#include <pthread.h>
#include "Game.h"
#include "../Client.h"

// Map update time period
struct timespec ts = {0, MILLION_50}; // must be equal to or a factor of server's update rate

Coordinates snakeHead;
int X_MAX, Y_MAX;
char **map;
bool gameRunning;
const char *latestMessage = "";

void initGame(void) {

    // Get init data from server
    GameDetails *gameDetails;
    if (recv_initialData(&gameDetails) == FAILED) {
        exit(EXIT_FAILURE);
    }

    // Set snake head and bounds
    snakeHead = gameDetails->startCoordinates;
    X_MAX = gameDetails->xMax;
    Y_MAX = gameDetails->yMax;

    // Initialize map
    map = malloc(sizeof(char *) * Y_MAX);
    for (int y = 0; y < Y_MAX; y++) {
        map[y] = malloc(sizeof(char) * X_MAX);
        for (int x = 0; x < X_MAX; x++) {
            map[y][x] = CHAR_BLANK;
        }
    }

    // Apply changes (includes snakes and fruit)
    for (int i = 0; i < gameDetails->numOfChanges; i++) {
        const MapChange *mc = &gameDetails->changes[i];
        map[mc->coord.y][mc->coord.x] = mc->newChar;
    }

    // Other
    gameRunning = true; // Initialize running state
    pthread_mutex_init(&mapLock, NULL); // Initialize map lock
    free(gameDetails);// Free received details
}

void startGame(void) {

    pthread_t mapThreadId, updThreadId, snkThreadId;
    pthread_create(&mapThreadId, NULL, thread_mapPrintLoop, NULL);
    pthread_create(&updThreadId, NULL, thread_updatesLoop, NULL);
    pthread_create(&snkThreadId, NULL, thread_directionsLoop, NULL);

    // No return till the main loop ends
    pthread_join(mapThreadId, NULL);
}

//---------------------------------------------------------------------------

void *thread_mapPrintLoop(void *arg) {

    while (gameRunning) {

        pthread_mutex_lock(&mapLock); // lock map

        // Start of screen with respect to map (negative if left or top bound are visible)
        const int screenX = snakeHead.x - (COLS / 2);
        const int screenY = snakeHead.y - (LINES / 2);

        // Coordinate limits of map section that will be printed (0 at minimum, X_MAX and Y_MAX at most)
        // These coordinates are with respect to the map.
        const int minMapX = max(0, screenX), maxMapX = min(X_MAX, screenX + COLS);
        const int minMapY = max(0, screenY), maxMapY = min(Y_MAX, screenY + LINES);

        // Limits of section that will be printed, including the wall
        // These coordinates are with respect to the screen.
        const int minPrtX = (minMapX - screenX - WALLWIDTH), maxPrtX = (maxMapX - screenX + WALLWIDTH);
        const int minPrtY = (minMapY - screenY - WALLWIDTH), maxPrtY = (maxMapY - screenY + WALLWIDTH);

        clear(); // clears previous screen
        for (int y = minPrtY; y < maxPrtY; y++) {
            for (int x = minPrtX; x < maxPrtX; x++) {

                // Coordinate of the map being printed
                const int mapX = x + screenX;
                const int mapY = y + screenY;

                // If map coordinate is within limits, print map character; otherwise print wall
                if (mapX >= minMapX && mapY >= minMapY && mapX < maxMapX && mapY < maxMapY) {
                    mvaddch(y, x, map[mapY][mapX]);
                } else {
                    mvaddch(y, x, CHAR_BOUND);
                }
            }
        }

        // Draw coordinates, latest message, and snake head in red
        attron(COLOR_PAIR(2)); // red foreground
        mvprintw(0, 0, "Coordinates: (%d,%d) ", snakeHead.x, snakeHead.y);  // coordinates indicator
        mvprintw(1, 0, "Latest message: %s", latestMessage);
        mvaddch(snakeHead.y - screenY, snakeHead.x - screenX, CHAR_SNAKE);  // red snake head
        attron(COLOR_PAIR(1)); // black foreground

        // Unlock map
        pthread_mutex_unlock(&mapLock);

        refresh();
        nanosleep(&ts, NULL);
    }
    return EXIT_SUCCESS;
}

void *thread_updatesLoop(void *arg) {

    GameUpdate *gameUpdate;
    GameMessage gameMessageIndex;

    while (gameRunning) {

        // Reset update and message
        gameUpdate = NULL;

        // Get game update from server and lock map immediately
        if (recv_gameUpdate(&gameUpdate, &gameMessageIndex) == FAILED) {
            exit(EXIT_FAILURE);
        } else {
            pthread_mutex_lock(&mapLock); // lock map
            if (gameUpdate != NULL) {

                // Set snake head and apply map changes
                snakeHead = gameUpdate->snakeHead;
                for (int i = 0; i < gameUpdate->numOfChanges; i++) {
                    Coordinates *coord = &gameUpdate->changes[i].coord;
                    map[coord->y][coord->x] = gameUpdate->changes[i].newChar;
                }
                free(gameUpdate); // free update
            } else {
                latestMessage = GameMessages[gameMessageIndex]; // apply message index
            }
            pthread_mutex_unlock(&mapLock); // unlock map
        }
    }
    return EXIT_SUCCESS;
}

void *thread_directionsLoop(void *arg) {

    while (gameRunning) {
        Outcome sendOutcome;
        const int input = getch(); // Get direction
        switch (input) {
            case CHAR_LEFT:
                sendOutcome = send_snakeDirection(DIR_LEFT);
                break;
            case CHAR_UP:
                sendOutcome = send_snakeDirection(DIR_UP);
                break;
            case CHAR_RIGHT:
                sendOutcome = send_snakeDirection(DIR_RIGHT);
                break;
            case CHAR_DOWN:
                sendOutcome = send_snakeDirection(DIR_DOWN);
                break;
            default:
                continue; // skip iteration
        }
        if (sendOutcome == FAILED) {
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------

void exit_closeGame(void) {

    endwin(); // End curses mode
    printf("Closing :: Stopped curses mode.\n");

    // Print latest message
    printf("Closing :: Latest message received: %s\n", latestMessage);

    gameRunning = false; // Game no longer running
    printf("Closing :: Set game to not running.\n");

    for (int y = 0; y < Y_MAX; y++) {
        free(map[y]); // Free map row
    }
    free(map); // Free remainder of map
    printf("Closing :: Called free on map.\n");

    // Destroy lock
    pthread_mutex_destroy(&mapLock);
    printf("Closing :: Destroyed map lock.\n");
}