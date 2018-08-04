#ifndef SERVER_CLIENTSLIST_H
#define SERVER_CLIENTSLIST_H

#include <stdlib.h>

typedef struct List List;
typedef struct Node Node;
typedef void (*freeNodeValueFn) (void *); // represents a function used to free nods individually

struct List {
    int nodeCount;
    Node *first;
    Node *last;
};

struct Node {
    void *value;
    Node *next;
};

List *createList(void);

Node *addToList(List *list, void *newValue);

void clearList(List *list, freeNodeValueFn freeNodeValue);

int clearNodeFromList(List *list, freeNodeValueFn freeNodeValue, Node *toRemove);

#endif //SERVER_CLIENTSLIST_H
