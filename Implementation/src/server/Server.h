#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include "Game/Snake.h"
#include "../general/LinkedList/LinkedList.h"

#define REQ_QUEUE_SIZE 10

void startListeningForClients(const int port);

int acceptConnection(struct sockaddr_in *cli_addr, const size_t clilen);

//---------------------------------------------------------------------------

Outcome send_gameDetails(const int clientSFD, const GameDetails *gd, const List *snakesList, const Coordinates fruit);

Outcome send_gameMessage(const int clientSFD, const GameMessage msg);

Outcome send_gameMessageToAll(const List *snakesList, const GameMessage msg);

Outcome send_gameUpdates(List *snakesList, const List *primaryChanges, List *secondaryChanges);

Outcome recv_direction(const int clientSFD, Direction *direction);

//---------------------------------------------------------------------------

void exit_closeServerSocket(void);

#endif //SERVER_SERVER_H
