#ifndef CLIENT_DESERIALIZE_H
#define CLIENT_DESERIALIZE_H

#include "../General.h"

unsigned char *deserialize_int(unsigned char *buffer, int *value);

unsigned char *deserialize_coordinates(unsigned char *buffer, Coordinates *value);

unsigned char *deserialize_mapChange(unsigned char *buffer, MapChange *value);

unsigned char *deserialize_mapChangeArray(unsigned char *buffer, MapChange value[], const int size);

#endif //CLIENT_DESERIALIZE_H
