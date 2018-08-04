#ifndef GENERAL_H
#define GENERAL_H

#include <stdio.h>
#include <stdbool.h>
#include "LinkedList/LinkedList.h"

#define MILLION_100 100000000L // Server update rate
#define MILLION_50 50000000L   // Snake update rate

#define MAX_SNAKE_LENGTH 15 // Maximum snake length
#define WALLWIDTH 4         // Width of wall as seen from client side

#define min(X, Y) ((X < Y) ? X : Y) // minimum function
#define max(X, Y) ((X > Y) ? X : Y) // maximum function

#define CHAR_BLANK  ' '
#define CHAR_SNAKE  'O'
#define CHAR_BOUND  '#'
#define CHAR_FRUIT  '@'

#define CHAR_UP     'w'
#define CHAR_DOWN   's'
#define CHAR_LEFT   'a'
#define CHAR_RIGHT  'd'

// These were created as a workaround to the 4-byte character
#define SIZEOF_MapChange            (sizeof(Coordinates) + sizeof(char))
#define SIZEOF_GameDetailsFirstPart (sizeof(Coordinates) + (3 * sizeof(int)))
#define SIZEOF_GameUpdateFirstPart  (sizeof(Coordinates) + sizeof(int))

typedef enum Direction Direction;
typedef enum Outcome Outcome;
typedef enum SnakeMoveOutcome SnakeMoveOutcome;
typedef enum PacketType PacketType;
typedef enum GameMessage GameMessage;
typedef struct Coordinates Coordinates;
typedef struct MapChange MapChange;
typedef struct GameDetails GameDetails;
typedef struct GameUpdate GameUpdate;
typedef struct PacketHeader PacketHeader;
typedef struct Packet Packet;

enum Direction {
    DIR_LEFT = 0,
    DIR_UP = 1,
    DIR_RIGHT = 2,
    DIR_DOWN = 3
};

enum Outcome {
    SUCCESSFUL,
    FAILED
};

enum SnakeMoveOutcome {
    MOV_COLLISION,
    MOV_NORMAL
};

enum PacketType {
    PKT_GAMEDETAILS,
    PKT_GAMEUPDATE,
    PKT_DIRECTION,
    PKT_GAMEMESSAGE
};

enum GameMessage {
    MSG_WELCOME,
    MSG_ATEFRUIT,
    MSG_COLLISION,
    MSG_YOUWON,
    MSG_OTHERWON,
    MSG_COUNTDOWN3,
    MSG_COUNTDOWN2,
    MSG_COUNTDOWN1,
    MSG_COUNTDOWN0,
    MSG_SERVERCLOSEDOWN,
    TOTAL_MESSAGES
};

struct Coordinates {
    int x;
    int y;
};

struct MapChange {
    Coordinates coord;
    char newChar;
};

struct GameDetails {
    Coordinates startCoordinates;
    int xMax;
    int yMax;
    int numOfChanges;
    MapChange changes[];
};

struct GameUpdate {
    Coordinates snakeHead;
    int numOfChanges;
    MapChange changes[];
};

struct PacketHeader {
    PacketType type;
    uint size;
};

struct Packet {
    PacketHeader header;
    unsigned char data[];
};

const char *GameMessages[TOTAL_MESSAGES];

void terminationHandler(int n);

Outcome recv_packet(const int sockFD, Packet **packet);

Outcome send_packet(const int sockFD, PacketType packetType, uint packetSize, unsigned char *packetData);

bool coordEqual(const Coordinates *c1, const Coordinates *c2);

#endif //GENERAL_H
