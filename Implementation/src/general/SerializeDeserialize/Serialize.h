#ifndef CLIENT_SERIALIZE_H
#define CLIENT_SERIALIZE_H

#include "../General.h"
#include "../../server/Game/Snake.h"

unsigned char *serialize_int(unsigned char *buffer, const int value);

unsigned char *serialize_coordinates(unsigned char *buffer, const Coordinates value);

unsigned char *serialize_mapChange(unsigned char *buffer, const MapChange *value);

unsigned char *serialize_gameDetails(unsigned char *buffer, const GameDetails *value, const List *snakeList, const Coordinates fruit);

unsigned char *serialize_gameUpdate(unsigned char *buffer, const List *value);

#endif //CLIENT_SERIALIZE_H
