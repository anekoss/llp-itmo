#include "../include/list.h"

listNode_t *createNode(int64_t id, uint64_t size) {
    listNode_t *listNode = malloc(sizeof(listNode_t));
    if (listNode) {
        listNode->size = size;
        listNode->index = id;
        listNode->prev = NULL;
        listNode->next = NULL;
        return listNode;
    }
    return NULL;
}

void destroyList(sortedList_t *list) {
    listNode_t *curNode = list->front;
    while (curNode) {
        listNode_t *nextNode = curNode->next;
        free(curNode);
        curNode = nextNode;
    }
}

void insertNode(sortedList_t *list, listNode_t *node) {
    if (list->front) {
        listNode_t *curNode = list->front;
        while (curNode) {
            if (node->size >= curNode->size) {
                if (curNode == list->front) {
                    list->front = node;
                } else {
                    curNode->prev->next = node;
                }
                node->prev = curNode->prev;
                node->next = curNode;
                curNode->prev = node;
                break;
            } else if (curNode == list->back) {
                list->back = node;
                node->next = curNode->next;
                node->prev = curNode;
                curNode->next = node;
                break;
            }
            curNode = curNode->next;
        }
    } else {
        list->front = node;
        list->back = node;
    }
}

listNode_t *popMaxSizeNode(sortedList_t *list) {
    if (list && list->front) {
        listNode_t *popNode = list->front;
        list->front = list->front->next;
        if (list->front) {
            list->front->prev = NULL;
        } else {
            list->back = NULL;
        }
        return popNode;
    }
    return NULL;
}

listNode_t *popMinSizeNode(sortedList_t *list) {
    if (list && list->back) {
        listNode_t *popNode = list->back;
        list->back = popNode->prev;
        if (list->back) {
            list->back->next = NULL;
        } else {
            list->front = NULL;
        }
        return popNode;
    }
    return NULL;
}


bool removeNodeByIndex(sortedList_t *list, uint64_t index) {
    if (list->front) {
        listNode_t *curNode = list->front;
        while (curNode) {
            if (curNode->index == index) {
                listNode_t *prevNode = curNode->prev;
                listNode_t *nextNode = curNode->next;
                if (prevNode) {
                    prevNode->next = nextNode;
                }
                if (nextNode) {
                    nextNode->prev = prevNode;
                }
                if (curNode == list->front) {
                    list->front = nextNode;
                }
                if (nextNode == list->back) {
                    list->back = prevNode;
                }
                free(curNode);
                return true;
            }
        }
    }
    return false;
}