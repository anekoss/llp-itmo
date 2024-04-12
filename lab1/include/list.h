#include "stdint.h"
#include "inode.h"
#include "stdlib.h"
#include "stdbool.h"

#define initSortedList (sortedList_t){.front=NULL, .back=NULL};

typedef struct listNode_t {
    uint64_t size;
    int64_t index;
    struct listNode_t *prev;
    struct listNode_t *next;
} listNode_t;

typedef struct sortedList_t {
    listNode_t *front;
    listNode_t *back;
} sortedList_t;

listNode_t *createNode(int64_t id, uint64_t size);

void destroyList(sortedList_t *list);

void insertNode(sortedList_t *list, listNode_t *node);

listNode_t *popMaxSizeNode(sortedList_t *list);

listNode_t *popMinSizeNode(sortedList_t *list);

bool removeNodeByIndex(sortedList_t *list, uint64_t index);



