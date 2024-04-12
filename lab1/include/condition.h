#include "doc.h"

typedef enum {
    OP_EQ_KEY_TYPE,
    OP_EQ,
    OP_NEQ,
    OP_GT,
    OP_GTE,
    OP_LT,
    OP_LTE,
    OP_AND,
    OP_OR,
    OP_NOT
} operationType_t;

typedef struct condition_t {
    operationType_t opType;
    union {
        element_t *element;
        struct condition_t *condition1;
    };
    struct condition_t *condition2;
} condition_t;

condition_t *createCondition(operationType_t type, void *condition1, void *condition2);

void destroyCondition(condition_t *condition);

condition_t *condEqual(element_t *compareElement);

condition_t *condNotEqual(element_t *compareElement);

condition_t *condGreater(element_t *compareElement);

condition_t *condGreaterOrEquals(element_t *compareElement);

condition_t *condLess(element_t *compareElement);

condition_t *condLessOrEqual(element_t *compareElement);

condition_t *condAnd(condition_t *condition1, condition_t *condition2);

condition_t *condNot(condition_t *condition);

condition_t *condOr(condition_t *condition1, condition_t *condition2);

bool checkElementByCondition(element_t *element, condition_t *condition);

int64_t checkDocumentByCondition(document_t *document, condition_t *condition);

