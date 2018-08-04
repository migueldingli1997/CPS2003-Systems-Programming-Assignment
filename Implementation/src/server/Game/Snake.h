#ifndef SERVER_SNAKE_H
#define SERVER_SNAKE_H

#include <stdbool.h>
#include "../../general/General.h"
#include "../../general/LinkedList/LinkedList.h"

typedef struct Snake Snake;
typedef struct SnakePart SnakePart;

struct Snake {
    int clientSFD;
    int length;
    Direction direction;
    SnakePart *head;
    SnakePart *tail;
    bool playing;
};

struct SnakePart {
    Coordinates coord;
    SnakePart *next;
};

Snake *newSnake(int clientSFD, Direction direction, Coordinates start);

void moveSnakeInCurrDirection(Snake *snake);

void lengthenSnake(Snake *snake);

void freeSnake(Snake *snake);

#endif //SERVER_SNAKE_H
