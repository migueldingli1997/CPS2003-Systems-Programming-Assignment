#include <stdlib.h>
#include "../server/Game/Snake.h"

Coordinates c[4] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 4}
};
Snake *s[4] = {
    NULL, NULL, NULL, NULL
};
char ch[4] = {
    'a', 'b', 'c', 'd'
};
MapChange *mc[4] = {
    NULL, NULL, NULL, NULL
};

static void snakesListTest1(void) {

    List *sl = createList();
    for (int i = 0; i < 4; i++) {
        s[i] = newSnake(i, (Direction) i, c[i]);
        lengthenSnake(s[i]);
        lengthenSnake(s[i]);
        lengthenSnake(s[i]);
        addToList(sl, s[i]);
    }
    printf("Snakes list size before remove: %d\n", sl->nodeCount);
    clearList(sl, (freeNodeValueFn) &freeSnake);
    printf("Snakes list size after remove:  %d\n", sl->nodeCount);
    free(sl);
}

static void snakesListTest2(void) {

    List *sl = createList();
    Node *n[4];
    
    for (int i = 0; i < 4; i++) {
        s[i] = newSnake(i, (Direction) i, c[i]);
        lengthenSnake(s[i]);
        lengthenSnake(s[i]);
        lengthenSnake(s[i]);
        n[i] = addToList(sl, s[i]);
    }
    printf("Snakes list size before remove: %d\n", sl->nodeCount);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n[3]);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n[2]);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n[1]);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n[0]);
    printf("Snakes list size after remove:  %d\n", sl->nodeCount);
    free(sl);
}

static void snakesListTest3(void) {

    List *sl = createList();
    for (int i = 0; i < 4; i++) {
        s[i] = newSnake(i, (Direction) i, c[i]);
        lengthenSnake(s[i]);
        lengthenSnake(s[i]);
        lengthenSnake(s[i]);
    }
    
    Node *n;
    n = addToList(sl, s[3]);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n);
    n = addToList(sl, s[2]);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n);
    n = addToList(sl, s[1]);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n);
    n = addToList(sl, s[0]);
    clearNodeFromList(sl, (freeNodeValueFn) &freeSnake, n);
    printf("Snakes list size after last remove:  %d\n", sl->nodeCount);
    free(sl);
}

static void mapChangesListTest1(void) {

    List *cl = createList();
    for (int i = 0; i < 4; i++) {
        mc[i] = malloc(sizeof(MapChange));
        mc[i]->coord = c[i];
        mc[i]->newChar = ch[i];
        addToList(cl, mc[i]);
    }
    printf("Changes list size before remove: %d\n", cl->nodeCount);
    clearList(cl, &free);
    printf("Changes list size after remove:  %d\n", cl->nodeCount);
    free(cl);
}

static void mapChangesListTest2(void) {

    List *cl = createList();
    Node *n[4];
    
    for (int i = 0; i < 4; i++) {
        mc[i] = malloc(sizeof(MapChange));
        mc[i]->coord = c[i];
        mc[i]->newChar = ch[i];
        n[i] = addToList(cl, mc[i]);
    }
    printf("Changes list size before remove: %d\n", cl->nodeCount);
    clearNodeFromList(cl, &free, n[3]);
    clearNodeFromList(cl, &free, n[2]);
    clearNodeFromList(cl, &free, n[1]);
    clearNodeFromList(cl, &free, n[0]);
    printf("Changes list size after remove:  %d\n", cl->nodeCount);
    free(cl);
}

static void mapChangesListTest3(void) {

    List *cl = createList();
    for (int i = 0; i < 4; i++) {
        mc[i] = malloc(sizeof(MapChange));
        mc[i]->coord = c[i];
        mc[i]->newChar = ch[i];
    }
    
    Node *n;
    n = addToList(cl, mc[3]);
    clearNodeFromList(cl, &free, n);
    n = addToList(cl, mc[2]);
    clearNodeFromList(cl, &free, n);
    n = addToList(cl, mc[1]);
    clearNodeFromList(cl, &free, n);
    n = addToList(cl, mc[0]);
    clearNodeFromList(cl, &free, n);
    printf("Changes list size after last remove:  %d\n", cl->nodeCount);
    free(cl);
}

int main(int argc, char *argv[]) {

    snakesListTest1();
    snakesListTest2();
    snakesListTest3();
    
    mapChangesListTest1();
    mapChangesListTest2();
    mapChangesListTest3();
    return 0;
}
