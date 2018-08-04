#include "../general/SerializeDeserialize/Serialize.h"
#include "../general/SerializeDeserialize/Deserialize.h"
#include <stdio.h>
#include <stdlib.h>

static void test_integer(void) {

    unsigned char buffer[4]; // storage for serialized int
    int value = 123456789, result;

    serialize_int(buffer, value);
    deserialize_int(buffer, &result);

    if (value != result) {
        fprintf(stderr, "Tests failed at integer.\n");
        exit(1);
    }
}

static void test_coordinates(void) {

    unsigned char buffer[8]; // storage for serialized coordinates
    Coordinates value = { 123456, 654321 }, result;

    serialize_coordinates(buffer, value);
    deserialize_coordinates(buffer, &result);

    if (value.x != result.x || value.y != result.y) {
        fprintf(stderr, "Tests failed at coordinates.\n");
        exit(1);
    }
}

static void test_mapChange(void) {

    unsigned char buffer[9]; // storage for coordinates and character
    MapChange value = { { 123456, 654321 }, 'O' }, result;

    serialize_mapChange(buffer, &value);
    deserialize_mapChange(buffer, &result);

    if (value.coord.x != result.coord.x || value.coord.y != result.coord.y || value.newChar != result.newChar) {
        fprintf(stderr, "Tests failed at map change.\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {

    printf("Performing test 1...");
    test_integer();
    printf("Success.\nPerforming test 2...");
    test_coordinates();
    printf("Success.\nPerforming test 3...");
    test_mapChange();
    printf("Success.\n");
}
