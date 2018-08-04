#include <stdio.h>
#include "LinkedList.h"

List *createList(void) {

    List *newList = malloc(sizeof(List));
    newList->nodeCount = 0;
    newList->first = NULL;
    newList->last = NULL;
    return newList;
}

Node *addToList(List *list, void *newValue) {

    // Initialize new node
    Node *newNode = malloc(sizeof(Node));
    newNode->value = newValue;
    newNode->next = NULL;

    if (list->first == NULL) { // Check if list is empty
        list->first = newNode;
    } else { // List not empty; Find first NULL and set as new node
        Node *node = list->first;
        while (node->next != NULL) {
            node = node->next;
        }
        node->next = newNode;
    }

    // Finalize
    list->last = newNode;
    list->nodeCount++;
    return newNode;
}

void clearList(List *list, freeNodeValueFn freeNodeValue) {

    // Free all nodes and node vlaues
    Node *node = list->first;
    while(node != NULL) {
        Node *toRemove = node;
        node = node->next;
        freeNodeValue(toRemove->value);
        free(toRemove);
    }

    // Reset pointers and count
    list->first = NULL;
    list->last = NULL;
    list->nodeCount = 0;
}

int clearNodeFromList(List *list, freeNodeValueFn freeNodeValue, Node *toRemove) {

    if (list->first == NULL) { // Check if list is empty
        printf("Attempted to remove node from empty list.\n");
        return -1;
    } else if (list->first == toRemove) { // Check if toRemove is first node
        if (list->nodeCount == 1) { // Check if toRemove is the only node in the list
            list->first = NULL;
            list->last = NULL;
        } else {
            list->first = list->first->next;
        }
    } else {

        // Find first 'next' node that matches with toRemove
        Node *node = list->first;
        while (node != NULL && node->next != toRemove) {
            node = node->next;
        }

        if (node == NULL) { // Check if node was found
            printf("Attempted to remove inexistent node.\n");
            return -1;
        } else { // It was found; Now shift pointers (and shift last, if need be)
            if (list->last == toRemove) {
                list->last = node;
            }
            node->next = node->next->next;
        }
    }

    // Finalize
    freeNodeValue(toRemove->value);
    free(toRemove);
    list->nodeCount--;
    return 1;
}