#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

typedef enum elementType_t {
    INT = 1,
    DOUBLE = 2,
    STRING = 3,
    BOOLEAN = 4,
} elementType_t;

typedef struct string {
    uint32_t length;
    char *str;
} string_t;

typedef struct element {
    uint8_t type;
    char key[13];
    union {
        string_t stringValue;
        int32_t integerValue;
        double doubleValue;
        bool boolValue;
    };
} element_t;

element_t *createElement(const char *key, element_t elem);

void destroyElement(element_t *elem);

element_t *doubleElement(const char *key, double value);

element_t *intElement(const char *key, int32_t value);

element_t *stringElement(const char *key, char *value);

element_t *booleanElement(const char *key, bool value);

int32_t compareElements(element_t *element1, element_t *element2);

void printElement(element_t *elem);

uint64_t calcElementSize(element_t *element);

