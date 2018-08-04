#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include "../../general/General.h"

pthread_mutex_t mapLock;

//---------------------------------------------------------------------------

void initGame(void);

void startGame(void);

//---------------------------------------------------------------------------

void *thread_mapPrintLoop(void *);

void *thread_updatesLoop(void *);

void *thread_directionsLoop(void *);

//---------------------------------------------------------------------------

void exit_closeGame(void);

#endif //CLIENT_GAME_H
