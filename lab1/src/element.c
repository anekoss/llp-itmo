#include "../include/element.h"

element_t *createElement(const char *key, element_t elem) {
    if (key && strlen(key) <= 12) {
        element_t *newElement = malloc(sizeof(element_t));
        if (newElement) {
            *newElement = elem;
            memset(newElement->key, 0, 13);
            strncpy(newElement->key, key, 13);
            return newElement;
        }
    }
    return NULL;
}

void destroyElement(element_t *elem) {
    if (elem) {
        free(elem);
    }
}

element_t *intElement(const char *key, int32_t value) {
    return createElement(key, (element_t) {.type =INT, .integerValue = value});
}

element_t *doubleElement(const char *key, double value) {
    return createElement(key, (element_t) {.type =DOUBLE, .doubleValue = value});
}

element_t *booleanElement(const char *key, bool value) {
    return createElement(key, (element_t) {.type =BOOLEAN, .boolValue = (uint8_t) value});
}

element_t *stringElement(const char *key, char *value) {
    if (!value) {
        value = "";
    }
    return createElement(key,
                         (element_t) {.type =STRING, .stringValue = (string_t) {.str=value, .length =
                         strlen(value) +
                         1}});

}

int32_t compareElements(element_t *element1, element_t *element2) {
    switch (element1->type) {
        case INT:
            return (element1->integerValue > element2->integerValue) -
                   (element1->integerValue < element2->integerValue);
        case DOUBLE:
            return (element1->doubleValue > element2->doubleValue) - (element1->doubleValue < element2->doubleValue);
        case BOOLEAN:
            return element1->boolValue - element2->boolValue;
        case STRING:
            return strcmp(element1->stringValue.str, element2->stringValue.str);
    }
    return 0;
}

void printElement(element_t *elem) {
    if (elem) {
        printf("\t %s = ", elem->key);
        switch (elem->type) {
            case INT:
                printf("%d\n", elem->integerValue);
                break;
            case DOUBLE:
                printf("%f\n", elem->doubleValue);
                break;
            case BOOLEAN:
                printf("%s\n", elem->boolValue ? "true" : "false");
                break;
            case STRING:
                printf("\"%s\"\n", elem->stringValue.str);
                break;
        }
    } else {
        printf("Element doesn't exist!\n");
    }
}

uint64_t calcElementSize(element_t *element) {
    switch (element->type) {
        case INT:
            return sizeof(int32_t);
        case DOUBLE:
            return sizeof(double);
        case BOOLEAN:
            return sizeof(int8_t);
        case STRING: {
            return sizeof(uint32_t) + element->stringValue.length * sizeof(char);
        }
    }
    return 0;
}
