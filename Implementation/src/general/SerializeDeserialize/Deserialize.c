#include <stdint.h>
#include <netinet/in.h>
#include "Deserialize.h"

unsigned char *deserialize_int(unsigned char *buffer, int *value) {

    // Reading big-endian int value from buffer; assumes 32-bit int and 8-bit char.
    uint32_t network_order = 0;
    network_order += buffer[0] << 24;
    network_order += buffer[1] << 16;
    network_order += buffer[2] << 8;
    network_order += buffer[3];

    // Convert to host byte order
    *value = ntohl(network_order);

    // Return advanced buffer
    return buffer + 4;
}

unsigned char *deserialize_coordinates(unsigned char *buffer, Coordinates *value) {

    buffer = deserialize_int(buffer, &(value->x));
    buffer = deserialize_int(buffer, &(value->y));
    return buffer;
}

unsigned char *deserialize_mapChange(unsigned char *buffer, MapChange *value) {

    buffer = deserialize_coordinates(buffer, &value->coord);
    value->newChar = buffer[0];
    return buffer + 1; // +1 due to character
}

unsigned char *deserialize_mapChangeArray(unsigned char *buffer, MapChange value[], const int size) {

    for (int i = 0; i < size; i++) {
        buffer = deserialize_mapChange(buffer, value + i);
    }
    return buffer;
}