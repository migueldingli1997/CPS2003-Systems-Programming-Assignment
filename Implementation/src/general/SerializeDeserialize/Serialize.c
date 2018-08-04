#include <netinet/in.h>
#include "Serialize.h"


unsigned char *serialize_int(unsigned char *buffer, int value) {

    // Convert to network byte order
    uint32_t network_order = htonl((uint32_t) value);

    // Writing big-endian int value into buffer; assumes 32-bit int and 8-bit char.
    buffer[0] = network_order >> 24;
    buffer[1] = network_order >> 16;
    buffer[2] = network_order >> 8;
    buffer[3] = network_order;

    // Return advanced buffer
    return buffer + 4;
}

unsigned char *serialize_coordinates(unsigned char *buffer, const Coordinates value) {

    buffer = serialize_int(buffer, value.x);
    buffer = serialize_int(buffer, value.y);
    return buffer;
}

unsigned char *serialize_mapChange(unsigned char *buffer, const MapChange *value) {

    buffer = serialize_coordinates(buffer, value->coord);
    buffer[0] = value->newChar;
    return buffer + 1; // +1 due to character
}

unsigned char *serialize_gameDetails(unsigned char *buffer, const GameDetails *value, const List *snakesList, const Coordinates fruit) {

    buffer = serialize_coordinates(buffer, value->startCoordinates);
    buffer = serialize_int(buffer, value->xMax);
    buffer = serialize_int(buffer, value->yMax);
    buffer = serialize_int(buffer, value->numOfChanges);

    // Snakes to map changes
    Node *node = snakesList->first;
    while (node != NULL) {
        const Snake *snake = node->value;
        if (snake->playing) {
            SnakePart *part = snake->head;
            while (part != NULL) {
                const MapChange mc = {{part->coord.x, part->coord.y}, CHAR_SNAKE};
                buffer = serialize_mapChange(buffer, &mc);
                part = part->next;
            }
        }
        node = node->next;
    }

    // Fruit to map change
    const MapChange mc = {{fruit.x, fruit.y}, CHAR_FRUIT};
    buffer = serialize_mapChange(buffer, &mc);

    return buffer;
}

// Note: client will read this data as GameUpdate
unsigned char *serialize_gameUpdate(unsigned char *buffer, const List *value) {

    const Coordinates BLANK = {0, 0}; // will be personalized later
    buffer = serialize_coordinates(buffer, BLANK);
    buffer = serialize_int(buffer, value->nodeCount);

    Node *change = value->first;
    while (change != NULL) {
        buffer = serialize_mapChange(buffer, change->value);
        change = change->next;
    }

    return buffer;
}