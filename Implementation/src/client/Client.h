#ifndef CLIENT_JOINGAME_H
#define CLIENT_JOINGAME_H

#include <netinet/in.h>
#include <stdint.h>
#include "../general/General.h"

void joinGame(const char *hostName, const int port);

//---------------------------------------------------------------------------

Outcome recv_initialData(GameDetails **details);

Outcome recv_gameUpdate(GameUpdate **update, GameMessage *messageIndex);

Outcome send_snakeDirection(const Direction direction);

//---------------------------------------------------------------------------

void exit_closeSocket(void);

#endif //CLIENT_JOINGAME_H
