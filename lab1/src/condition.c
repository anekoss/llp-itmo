#include "../include/condition.h"

condition_t *createCondition(operationType_t type, void *condition1, void *condition2) {
    condition_t *condition = malloc(sizeof(condition_t));
    if (condition) {
        condition->opType = type;
        condition->condition1 = condition1;
        condition->condition2 = condition2;
        return condition;
    }
    return NULL;
}

void destroyCondition(condition_t *condition) {
    if (condition) {
        if (condition->opType == OP_AND || condition->opType == OP_OR || condition->opType == OP_NOT) {
            destroyCondition(condition->condition1);
            destroyCondition(condition->condition2);
        } else {
            destroyElement(condition->element);
        }
        free(condition);
    }
}

condition_t *condEqual(element_t *compareElement) {
    if (compareElement) {
        return createCondition(OP_EQ, compareElement, NULL);
    }
    return NULL;
}

condition_t *condNotEqual(element_t *compareElement) {
    if (compareElement) {
        return createCondition(OP_NEQ, compareElement, NULL);
    }
    return NULL;
}

condition_t *condGreater(element_t *compareElement) {
    if (compareElement) {
        return createCondition(OP_GT, compareElement, NULL);
    }
    return NULL;
}

condition_t *condGreaterOrEquals(element_t *compareElement) {
    if (compareElement) {
        return createCondition(OP_GTE, compareElement, NULL);
    }
    return NULL;
}

condition_t *condLess(element_t *compareElement) {
    if (compareElement) {
        return createCondition(OP_LT, compareElement, NULL);
    }
    return NULL;
}

condition_t *condLessOrEqual(element_t *compareElement) {
    if (compareElement) {
        return createCondition(OP_LTE, compareElement, NULL);
    }
    return NULL;
}

condition_t *condAnd(condition_t *condition1, condition_t *condition2) {
    if (condition1 && condition2) {
        return createCondition(OP_AND, condition1, condition2);
    }
    return NULL;
}

condition_t *condNot(condition_t *condition) {
    if (condition) {
        return createCondition(OP_NOT, condition, NULL);
    }
    return NULL;
}

condition_t *condOr(condition_t *condition1, condition_t *condition2) {
    if (condition1 && condition2) {
        return createCondition(OP_OR, condition1, condition2);
    }
    return NULL;
}


int64_t checkDocumentByCondition(document_t *document, condition_t *condition) {
    bool result = false;
    if (!condition) {
        return 1;
    }
    for (int64_t i = 0; i < document->meta.count; i++) {
        result = checkElementByCondition(document->elements[i], condition);
        if (result) {
            return i;
        }
    }
    return -1;
}

bool checkElementByCondition(element_t *element, condition_t *condition) {
    if (condition->opType != OP_AND && condition->opType != OP_OR && condition->opType != OP_NOT)
        if (strcmp(element->key, condition->element->key) || element->type != condition->element->type) {
            return false;
        }
    switch (condition->opType) {
        case OP_EQ_KEY_TYPE: {
            return true;
        }
        case OP_EQ: {
            return compareElements(element, condition->element) == 0;
        }
        case OP_NEQ: {
            return compareElements(element, condition->element) != 0;
        }
        case OP_GT: {
            return compareElements(element, condition->element) > 0;
        }
        case OP_GTE: {
            return compareElements(element, condition->element) >= 0;
        }
        case OP_LT: {
            return compareElements(element, condition->element) < 0;
        }
        case OP_LTE: {
            return compareElements(element, condition->element) <= 0;
        }
        case OP_AND: {
            return checkElementByCondition(element, condition->condition1) &
                   checkElementByCondition(element, condition->condition2);
        }
        case OP_OR: {
            return checkElementByCondition(element, condition->condition1) |
                   checkElementByCondition(element, condition->condition2);
        }
        case OP_NOT: {
            return !checkElementByCondition(element, condition->condition1);
        }
    }
    return false;
}
