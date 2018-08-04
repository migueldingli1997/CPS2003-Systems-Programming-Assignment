#include <stddef.h>
#include <stdlib.h>
#include "Snake.h"

static SnakePart *newPart(Coordinates coord) {

    // Initialize new snake part
    SnakePart *newPart = malloc(sizeof(SnakePart));
    newPart->coord = coord;
    newPart->next = NULL;
    return newPart;
}

Snake *newSnake(int clientSFD, Direction direction, Coordinates start) {

    // Initialize new snake
    Snake *snake = malloc(sizeof(Snake));
    snake->clientSFD = clientSFD;
    snake->length = 1;
    snake->direction = direction;
    snake->head = newPart(start);
    snake->tail = snake->head;
    snake->playing = true;
    return snake;
}

static void shiftParts(SnakePart *part) {

    if (part->next == NULL) {
        // Tail does not need to pass on coordinates
        return;
    } else {
        // For each part, pass on coordinates to next part
        shiftParts(part->next);
        part->next->coord = part->coord;
    }
}

void moveSnakeInCurrDirection(Snake *snake) {

    // Shift all parts
    shiftParts(snake->head);

    // Move head by one step
    switch (snake->direction) {
        case DIR_UP:
            snake->head->coord.y -= 1;
            break;
        case DIR_DOWN:
            snake->head->coord.y += 1;
            break;
        case DIR_LEFT:
            snake->head->coord.x -= 1;
            break;
        case DIR_RIGHT:
            snake->head->coord.x += 1;
            break;
    }
}

void lengthenSnake(Snake *snake) {

    snake->tail->next = newPart(snake->tail->coord); // Give new tail coordinates of current tail
    snake->tail = snake->tail->next; // Set new tail as snake tail
    snake->length++; // Length has increased
}

static void freeSnakeParts(SnakePart *head) {

    // Free parts recursively in reverse
    if (head == NULL) {
        return;
    } else {
        freeSnakeParts(head->next);
        free(head);
    }
}

void freeSnake(Snake *snake) {

    freeSnakeParts(snake->head);    // Free parts
    free(snake);                    // Free main pointer
}