#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include "../Server.h"
#include "Game.h"

// Game update time period
struct timespec ts = {0, MILLION_100}; // must be equal to or a multiple of clients' update rate

int X_MAX, Y_MAX;       // not modified (initialized in initGame)
char **map;
List *snakesList;
bool gameRunning;
Coordinates fruitCoordinates;

static Snake *initializeSnake(const int sockFD) {

    // Set direction and coordinates (loop until blank space found)
    int randomX, randomY;
    Direction randomDirection;
    do {
        randomX = (rand() % (X_MAX / 2)) + (X_MAX / 4); // from 1/4 to 3/4
        randomY = (rand() % (Y_MAX / 2)) + (Y_MAX / 4); // from 1/4 to 3/4
        randomDirection = (Direction) (rand() % 4);
    } while (map[randomY][randomX] != CHAR_BLANK);
    Coordinates newCoord = {randomX, randomY};

    // Lengthen and return snake
    Snake *snake = newSnake(sockFD, randomDirection, newCoord);
    lengthenSnake(snake); // length = 2
    lengthenSnake(snake); // length = 3
    return snake;
}

static void addNewMapChange(List *mapChanges, Coordinates coord, char newChar) {

    MapChange *newMapChange = malloc(sizeof(MapChange));
    newMapChange->newChar = newChar;
    newMapChange->coord = coord;
    addToList(mapChanges, newMapChange);
}

static SnakeMoveOutcome moveSnake(Snake *snake, List *mapChanges) {

    addNewMapChange(mapChanges, snake->tail->coord, CHAR_BLANK); // clear previous tail
    moveSnakeInCurrDirection(snake);
    Coordinates hc = snake->head->coord; // new head coordinates

    // Check for collisions
    if (hc.y < 0 || hc.x < 0 || (hc.y >= Y_MAX) || (hc.x >= X_MAX) || map[hc.y][hc.x] == CHAR_SNAKE) {
        return MOV_COLLISION;
    }

    // If no collision, add new head
    addNewMapChange(mapChanges, snake->head->coord, CHAR_SNAKE);
    return MOV_NORMAL;
}

static void moveAllSnakes(List *mapChanges) {

    // Snakes list traversal
    Node *node = snakesList->first;
    while (node != NULL) {
        Snake *snake = node->value;
        if (snake->playing) {

            // If move was a collision, clear the snake from the map
            const SnakeMoveOutcome moveResult = moveSnake(snake, mapChanges);
            if (moveResult == MOV_COLLISION) {
                // Head excluded from removal since it's either part of another snake or part of the wall
                clearPartsFromMap(snake->head->next, mapChanges);
                snake->playing = false;
                send_gameMessage(snake->clientSFD, MSG_COLLISION);
            }
        }
        node = node->next;
    }
}

static void checkHeadCollisions(List *mapChanges) {

    // Outer snakes list traversal
    const Node *node1 = snakesList->first;
    while (node1 != NULL) {
        Snake *snake1 = node1->value;
        Coordinates *snake1HC = &snake1->head->coord;
        if (snake1->playing) {

            // Inner snakes list traversal
            const Node *node2 = snakesList->first;
            while (node2 != NULL) {
                Snake *snake2 = node2->value;
                Coordinates *snake2HC = &snake2->head->coord;
                if (snake2->playing) {

                    // Check for collision if snakes are distinct
                    if (snake1->clientSFD != snake2->clientSFD && coordEqual(snake1HC, snake2HC)) {
                        Coordinates collisionCoord = *snake1HC;
                        clearPartsFromMap(snake1->head, mapChanges);
                        clearPartsFromMap(snake2->head, mapChanges);
                        snake1->playing = snake2->playing = false;
                        send_gameMessage(snake1->clientSFD, MSG_COLLISION);
                        send_gameMessage(snake2->clientSFD, MSG_COLLISION);

                        // If snakes collided on top of fruit, re-draw fruit
                        if (coordEqual(&collisionCoord, &fruitCoordinates)) {
                            addNewMapChange(mapChanges, fruitCoordinates, CHAR_FRUIT);
                        }
                    }
                }
                node2 = node2->next;
            }
        }
        node1 = node1->next;
    }
}

static void checkIfFruitEaten(List *mapChanges, Snake **winner) {

    // Snakes list traversal
    Node *node = snakesList->first;
    while (node != NULL) {
        Snake *snake = node->value;
        if (snake->playing && coordEqual(&snake->head->coord, &fruitCoordinates)) {
            lengthenSnake(snake);
            if (snake->length >= MAX_SNAKE_LENGTH) { // Check if snake reached max height
                *winner = snake;
            } else {
                send_gameMessage(snake->clientSFD, MSG_ATEFRUIT);
                addNewFruit(mapChanges); // Add new fruit
            }
            break; // Only one snake can eat a fruit
        }
        node = node->next;
    }
}

static void applyMapChanges(List *mapChanges) {

    Node *node = mapChanges->first;
    while (node != NULL) {
        const MapChange *ch = node->value;
        map[ch->coord.y][ch->coord.x] = ch->newChar;
        node = node->next;
    }
}

void clearPartsFromMap(SnakePart *part, List *mapChanges) {

    while (part != NULL) {
        if (part->coord.x >= 0 && part->coord.y >= 0 && part->coord.x < X_MAX && part->coord.y < Y_MAX) {
            addNewMapChange(mapChanges, part->coord, CHAR_BLANK);
        }
        part = part->next;
    }
}

void addNewFruit(List *mapChanges) {

    int newFruitX, newFruitY;
    while (map[newFruitY = rand() % Y_MAX][newFruitX = rand() % X_MAX] != CHAR_BLANK);
    fruitCoordinates.x = newFruitX;
    fruitCoordinates.y = newFruitY;
    addNewMapChange(mapChanges, fruitCoordinates, CHAR_FRUIT); // add fruit change
}


//---------------------------------------------------------------------------

void initGame(const int xMax, const int yMax) {

    // Initialize bounds
    X_MAX = xMax;
    Y_MAX = yMax;

    // Initialize map
    map = malloc(sizeof(char *) * Y_MAX);
    for (int y = 0; y < Y_MAX; y++) {
        map[y] = malloc(sizeof(char) * X_MAX);
        for (int x = 0; x < X_MAX; x++) {
            map[y][x] = CHAR_BLANK;
        }
    }

    // Initialize and add fruit
    fruitCoordinates.x = rand() % X_MAX;
    fruitCoordinates.y = rand() % Y_MAX;
    map[fruitCoordinates.y][fruitCoordinates.x] = CHAR_FRUIT;

    // Other
    snakesList = createList();  // Create list of snakes
    gameRunning = true;         // Initialize running state
    pthread_mutex_init(&snakesListAndFruitLock, NULL);  // Initialize lock
}

void startGame(void) {

    pthread_t mainLoopTID, acceptClientsTID;
    pthread_create(&acceptClientsTID, NULL, thread_acceptClientsLoop, NULL); // Start accepting clients
    pthread_create(&mainLoopTID, NULL, thread_mainLoop, NULL); // Start main loop

    // Wait for main loop to end
    pthread_join(mainLoopTID, NULL);
}

void restartGame(List *mapChanges) {

    // Clear map
    for (int y = 0; y < Y_MAX; y++) {
        for (int x = 0; x < X_MAX; x++) {
            map[y][x] = CHAR_BLANK;
        }
    }

    pthread_mutex_lock(&snakesListAndFruitLock); // Lock snakes list and fruit

    // Replace snakes in snakes list
    Node *node = snakesList->first;
    while (node != NULL) {
        // Clearing old snake from map and initializing new snake
        Snake *snake = node->value;
        clearPartsFromMap(snake->head, mapChanges); // Clear from map
        node->value = initializeSnake(snake->clientSFD); // New snake

        // Apply new snake head to map (this is not done in main loop)
        const Snake *newSnake = node->value;
        map[newSnake->head->coord.y][newSnake->head->coord.x] = CHAR_SNAKE;
        addNewMapChange(mapChanges, newSnake->head->coord, CHAR_BLANK);

        // Free previous snake and advance to next
        freeSnake(snake);
        node = node->next;
    }
    // New fruit
    addNewFruit(mapChanges);

    pthread_mutex_unlock(&snakesListAndFruitLock); // Unlock snakes list and fruit
}

//---------------------------------------------------------------------------

void *thread_acceptClientsLoop(void *arg) {

    pthread_t newThread; // thread ID of latest thread
    while (gameRunning) {

        // Accept client
        struct sockaddr_in cli_addr;
        const int clientSFD = acceptConnection(&cli_addr, sizeof(cli_addr));

        // Lock snake list and fruit
        pthread_mutex_lock(&snakesListAndFruitLock);

        // Add snake
        Snake *newSnake = initializeSnake(clientSFD);
        Node *newNode = addToList(snakesList, newSnake); // add snake

        // Initialize GameDetails (excl. changes array)
        GameDetails gd;
        gd.startCoordinates = newSnake->head->coord;
        gd.yMax = Y_MAX;
        gd.xMax = X_MAX;
        gd.numOfChanges = 0; // calculated below

        // Calculating number of changes
        Node *node = snakesList->first;
        while (node != NULL) {
            const Snake *snake = node->value;
            gd.numOfChanges += (snake->playing ? snake->length : 0);
            node = node->next;
        }
        gd.numOfChanges++; // for the fruit

        // Send init data
        if (send_gameDetails(clientSFD, &gd, snakesList, fruitCoordinates) == SUCCESSFUL &&
            send_gameMessage(clientSFD, MSG_WELCOME) == SUCCESSFUL) {
            pthread_create(&newThread, NULL, (void *) thread_directionsLoop, newNode);
            printf("Client joined the game. New client count: %d\n", snakesList->nodeCount);
        } else {
            clearNodeFromList(snakesList, (freeNodeValueFn) &freeSnake, newNode);
            printf("Client left the game (Reason: sending of initial data failed). "
                       "New client count: %d.\n", snakesList->nodeCount);
        }

        // Unlock snake list and fruit
        pthread_mutex_unlock(&snakesListAndFruitLock);
    }
    return EXIT_SUCCESS;
}

void *thread_mainLoop(void *arg) {

    List *primaryChanges = createList();    // changes due to snakes moving
    List *secondaryChanges = createList();  // changes due to clients disconnecting

    Snake *winner = NULL;                   // winner snake
    bool postRestartRound = false;          // set after winner is found

    while (gameRunning) {

        // Reset winner
        winner = NULL;

        // Go through a cycle if there are snakes
        if (snakesList->nodeCount > 0) {

            // Reset lists and set primary changes list as secondary changes list
            clearList(primaryChanges, &free);
            free(primaryChanges);
            primaryChanges = secondaryChanges;
            secondaryChanges = createList();

            pthread_mutex_lock(&snakesListAndFruitLock); // Lock snakes list and fruit
            moveAllSnakes(primaryChanges);
            checkHeadCollisions(primaryChanges);
            checkIfFruitEaten(primaryChanges, &winner);
            pthread_mutex_unlock(&snakesListAndFruitLock); // Unlock snakes list and fruit

            // Send changes to clients (At this point, primary contains previous secondary followed by latest primary)
            send_gameUpdates(snakesList, primaryChanges, secondaryChanges);

            // Apply changes to map
            applyMapChanges(primaryChanges);
            applyMapChanges(secondaryChanges);

            // If all snakes disconnected, clear secondary changes list since new players
            // will not have snakes at the locations cleared by secondary changes list.
            if (snakesList->nodeCount == 0) {
                clearList(secondaryChanges, &free);
            }

            // If winner was found during last cycle
            if (winner != NULL) {
                // Send general message to all, but win message to winners
                send_gameMessageToAll(snakesList, MSG_OTHERWON);
                send_gameMessage(winner->clientSFD, MSG_YOUWON);
                restartGame(secondaryChanges);
                sleep(2);
                postRestartRound = true;
            } else if (postRestartRound) {
                send_gameMessageToAll(snakesList, MSG_COUNTDOWN3);
                sleep(1);
                send_gameMessageToAll(snakesList, MSG_COUNTDOWN2);
                sleep(1);
                send_gameMessageToAll(snakesList, MSG_COUNTDOWN1);
                sleep(1);
                send_gameMessageToAll(snakesList, MSG_COUNTDOWN0);
                postRestartRound = false;
            }
        }
        nanosleep(&ts, NULL);
    }

    // Free lists (primary could have already been freed)
    free(secondaryChanges);
    return EXIT_SUCCESS;
}

void *thread_directionsLoop(Node *snakeNode) {

    Snake **snake = (Snake **) &(snakeNode->value);

    Direction response;
    while (gameRunning) {
        if (recv_direction((*snake)->clientSFD, &response) == SUCCESSFUL) {
            // Check to see if direction is valid (if up/down, previous must be left/right, and vice-versa)
            if ((*snake)->direction != response && (*snake)->direction != ((response + 2) % 4)) {
                (*snake)->direction = response;
            }
        } else {
            // Should crash even if snake is freed
            printf("Receiving of direction failed. Disabling direction changes for snake %d.\n", (*snake)->clientSFD);
            break;
        }
    }
    return EXIT_SUCCESS;
}

//---------------------------------------------------------------------------

void exit_closeClientSockets(void) {

    // Close client sockets
    Node *node = snakesList->first;
    while (node != NULL) {
        close(((Snake *) node->value)->clientSFD);
        node = node->next;
    }
    printf("Closing :: Closed client sockets.\n");
}

void exit_closeGame(void) {

    gameRunning = false;
    printf("Closing :: Set game to not running.\n");

    // Send final message to users
    send_gameMessageToAll(snakesList, MSG_SERVERCLOSEDOWN);

    // Deallocate all snakes and the list itself
    clearList(snakesList, (freeNodeValueFn) &freeSnake);
    free(snakesList);
    printf("Closing :: Called free on snake list.\n");

    for (int y = 0; y < Y_MAX; y++) {
        free(map[y]); // Free map row
    }
    free(map); // Free remainder of map
    printf("Closing :: Called free on map.\n");

    // Destroy lock
    pthread_mutex_destroy(&snakesListAndFruitLock);
    printf("Closing :: Destroyed snakes list lock.\n");
}
