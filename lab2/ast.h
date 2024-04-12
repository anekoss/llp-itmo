#include "string.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"

#define initQuery (query_t){.type=SELECT, .documents=NULL, .condition=NULL, .documentName=NULL, .limit = -1};
#define  initDocumentMeta  (meta_t){.id = 0, .size =0, .child =-1, .brother = -1, .count = 0};

typedef enum elementType_t {
    INT = 1,
    DOUBLE = 2,
    STRING = 3,
    BOOLEAN = 4,
} elementType_t;

typedef struct string_t {
    uint32_t length;
    char *str;
} string_t;

typedef struct element_t {
    uint8_t type;
    char key[13];
    union {
        string_t stringValue;
        int32_t integerValue;
        double doubleValue;
        bool boolValue;
    };
    struct element_t *next;
} element_t;

typedef struct meta_t {
    char name[13];
    int64_t id;
    uint64_t size;
    int64_t child;
    int64_t brother;
    uint64_t count;
} meta_t;

typedef struct {
    meta_t meta;
    element_t **elements;
    struct document_t *next;
} document_t;

typedef struct queryResult {
    uint64_t count;
    document_t **documents;
} queryResult_t;


queryResult_t *createQueryResult();

void destroyQueryResult(queryResult_t *queryResult);

typedef enum {
    OP_EQ,
    OP_NEQ,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE,
    OP_REGEX,
    OP_AND,
    OP_OR,
    OP_SAME_LEVEL
} operationType_t;



typedef struct condition_t {
    operationType_t opType;
    union {
        element_t *element;
        struct condition_t *condition1;
    };
    struct condition_t *condition2;
} condition_t;

typedef enum queryType_t {
    SELECT,
    INSERT,
    UPDATE,
    DELETE
} queryType_t;


typedef struct query_t {
    queryType_t type;
    char *documentName;
    condition_t *condition;
    queryResult_t *documents;
    int32_t limit;
} query_t;

void destroyQuery(query_t *query);

document_t *addElements(document_t *document, element_t *element);

queryResult_t *createQueryResult();

element_t *stringElement(const char *key, char *value);

element_t *doubleElement(const char *key, double value);

element_t *booleanElement(const char *key, bool value);

queryResult_t *addQueryResult(queryResult_t *queryResult, document_t *document);

document_t *createDocument(char *name);

bool addElement(document_t *document, element_t *elem);

element_t *intElement(const char *key, int32_t value);

void printQuery(query_t *query);

void printDocument(document_t *document);

void destroyElement(element_t *elem);

void destroyDocument(document_t *document);

query_t *
createManyQuery(queryType_t type, char *documentName, condition_t *condition, queryResult_t *documents, int32_t limit);

void printDocuments(queryResult_t *documents);

query_t *createQuery(queryType_t type, char *documentName, condition_t *condition, queryResult_t *documents);

void printElement(element_t *elem);

void printCondition(condition_t *condition);

condition_t *createCondition(operationType_t type, void *condition1, void *condition2);


