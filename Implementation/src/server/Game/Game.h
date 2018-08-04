#ifndef SERVER_NCURSESEXAMPLE_H
#define SERVER_NCURSESEXAMPLE_H

#include "Snake.h"

pthread_mutex_t snakesListAndFruitLock;

//---------------------------------------------------------------------------

void clearPartsFromMap(SnakePart *part, List *mapChanges);

void addNewFruit(List *mapChanges);

//---------------------------------------------------------------------------

void initGame(const int xMax, const int yMax);

void startGame(void);

void restartGame(List *mapChanges);

//---------------------------------------------------------------------------

void *thread_acceptClientsLoop(void *arg);

void *thread_mainLoop(void *);

void *thread_directionsLoop(Node *snakeNode);

//---------------------------------------------------------------------------

void exit_closeClientSockets(void);

void exit_closeGame(void);

#endif //SERVER_NCURSESEXAMPLE_H
