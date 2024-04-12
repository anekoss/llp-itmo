#include "ast.h"


query_t *createQuery(queryType_t type, char *documentName, condition_t *condition, queryResult_t *documents) {
    query_t *query = malloc(sizeof(query_t));
    if (query) {
        *query = initQuery;
        if (condition) {
            query->condition = condition;
        }
        query->type = type;
        if (documents) {
            query->documents = documents;
        }
        if (documentName && strlen(documentName) <= 12) {
            query->documentName = documentName;
        }
        return query;
    }
    return NULL;
}

query_t *
createManyQuery(queryType_t type, char *documentName, condition_t *condition, queryResult_t *documents, int32_t limit) {
    query_t *query = malloc(sizeof(query_t));
    if (query) {
        *query = initQuery;
        query->limit = limit;
        if (condition) {
            query->condition = condition;
        }
        query->type = type;
        if (documents) {
            query->documents = documents;
        }
        if (documentName && strlen(documentName) <= 12) {
            query->documentName = documentName;
        }
        return query;
    }
    return NULL;
}

#define NULLVALUE NULL


document_t *createDocument(char *name) {
    if (name && strlen(name) <= 12) {
        document_t *doc = malloc(sizeof(document_t));
        if (doc) {
            doc->meta = initDocumentMeta;
            memset(doc->meta.name, 0, 13);
            strncpy(doc->meta.name, name, 13);
            return doc;
        }
    }
    return NULL;
}

bool addElement(document_t *document, element_t *elem) {
    if (elem) {
        element_t **elems = realloc(document->elements, sizeof(element_t *) * (document->meta.count + 1));
        if (elems) {
            document->elements = elems;
            document->elements[document->meta.count++] = elem;
            return true;
        }
    }
    return false;
}


document_t *addElements(document_t *document, element_t *element) {
    element_t *cur = element;
    int64_t count = 0;
    while (cur) {
        addElement(document, cur);
        count++;
        cur = cur->next;
    }
    document->meta.count = count;
    return document;
}

element_t *createElement(const char *key, element_t elem) {
    if (key && strlen(key) <= 12) {
        element_t *newElement = malloc(sizeof(element_t));
        if (newElement) {
            *newElement = elem;
            memset(newElement->key, 0, 13);
            strncpy(newElement->key, key, 13);
            return newElement;
        }
        elem.next = NULL;
    }
    return NULL;
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

void printQueryType(query_t *query) {
    printf("QUERY TYPE: ");
    switch (query->type) {
        case SELECT: {
            printf("SELECT");
            break;
        }
        case DELETE: {
            printf("DELETE");
            break;
        }
        case UPDATE: {
            printf("UPDATE");
            break;
        }
        case INSERT: {
            printf("INSERT");
            break;
        }
    }
    printf("\n");
}


void printDocument(document_t *document) {
    if (document) {
        printf("\t\tDOCUMENT\n");
        printf("\t\t\tNAME: %s\n", document->meta.name);
        printf("\t\t\tELEMENTS : \n");
        for (int64_t i = 0; i < document->meta.count; i++) {
            printElement(document->elements[i]);
        }
    }

}

void printDocuments(queryResult_t *documents) {
    printf("\tDOCUMENTS\n");
    for (int64_t i = 0; i < documents->count; i++) {
        if (documents->documents[i])
            printDocument(documents->documents[i]);
    }
}


void destroyElement(element_t *elem) {
    if (elem) {
        free(elem);
    }
}

void destroyDocument(document_t *document) {
    if (document) {
        if (document->elements) {
            for (uint64_t i = 0; i < document->meta.count; i++) {
                if (document->elements[i])
                    destroyElement(document->elements[i]);
            }
            free(document->elements);
        }

        free(document);
    }
}

void destroyCondition(condition_t *condition) {
    if (condition) {
        if (condition->opType == OP_AND || condition->opType == OP_OR) {
            destroyCondition(condition->condition1);
            destroyCondition(condition->condition2);
        } else {
            destroyElement(condition->element);
        }
        free(condition);
    }
}

void destroyQuery(query_t *query) {
    if (query) {
        if (query->documents) {
            destroyQueryResult(query->documents);
        }
        if (query->condition) {
            destroyCondition(query->condition);
        }
        free(query);
    }
}

void destroyQueryResult(queryResult_t *queryResult) {
    if (queryResult) {
        for (int64_t i = 0; i < queryResult->count; i++) {
            destroyDocument(queryResult->documents[i]);
        }
        if (queryResult->documents) {
            free(queryResult->documents);
        }
        free(queryResult);
    }
}


void printElement(element_t *elem) {
    if (elem) {
        printf("\t\t\t\t %s = ", elem->key);
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

queryResult_t *createQueryResult() {
    queryResult_t *queryResult = malloc(sizeof(queryResult_t));
    queryResult->count = 0;
    queryResult->documents = NULL;
    return queryResult;
}

queryResult_t *addQueryResult(queryResult_t *queryResult, document_t *document) {
    queryResult->documents = realloc(queryResult->documents, ++queryResult->count * sizeof(document_t));
    uint64_t count = queryResult->count - 1;
    queryResult->documents[count] = document;
    return queryResult;
}


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


void printOffset(int64_t level) {
    for (int64_t i = 0; i < level; i++) {
        printf("\t");
    }
}

void printConditionValue(element_t *element) {
    switch (element->type) {
        case INT:
            printf("%d\n", element->integerValue);
            break;
        case DOUBLE:
            printf("%f\n", element->doubleValue);
            break;
        case BOOLEAN:
            printf("%s\n", element->boolValue ? "true" : "false");
            break;
        case STRING:
            printf("\"%s\"\n", element->stringValue.str);
            break;
    }
}

const char *conditionStringName[] = {
        "OP_EQ",
        "OP_NEQ",
        "OP_GT",
        "OP_GTE",
        "OP_LT",
        "OP_LTE",
        "OP_REGEX",
        "OP_AND",
        "OP_OR"
};

void printConditionWithOffset(condition_t *condition, int64_t level) {
    if (condition) {
        if (condition->opType == OP_AND || condition->opType == OP_OR) {
            printOffset(level);
            printf("operator %s \n ", conditionStringName[condition->opType]);
            level++;
            int64_t i = level;
            printConditionWithOffset(condition->condition1, i);
            i = level;
            printConditionWithOffset(condition->condition2, i);
            level--;
        } else if (condition->opType == OP_SAME_LEVEL) {
            int64_t i = level;
            printConditionWithOffset(condition->condition1, i);
            i = level;
            printConditionWithOffset(condition->condition2, i);
            level = i;
        } else if (condition->element) {
            printOffset(level);
            printf("%s ", condition->element->key);
            printf("%s ", conditionStringName[condition->opType]);
            printConditionValue(condition->element);
        }
    }

}

void printUpdateQuery(query_t *query) {
    if (query->type == UPDATE) {
        if (query->limit != -1) {
            printOffset(2);
            printf("LIMIT: %d", query->limit);
            printf("\n");
        }
        if (query->condition) {
            printCondition(query->condition);
        }
        if (query->documents) {
            printOffset(2);
            printf("ELEMENTS:\n");
            if (query->documents->documents[0])
                for (int64_t i = 0; i < query->documents->documents[0]->meta.count; i++) {
                    printElement(query->documents->documents[0]->elements[i]);
                }
        }
    }
}

void printInsertQuery(query_t *query) {
    if (query->type == INSERT) {
        if (query->documents) {
            printDocuments(query->documents);
        }
    }
}

void printSelectOrDelete(query_t *query) {
    if (query->type == SELECT || query->type == DELETE) {
        if (query->limit != -1) {
            printOffset(2);
            printf("LIMIT: %lld", query->limit);
        }
        if (query->condition) {
            printCondition(query->condition);
        }
    }
}

void printCondition(condition_t *condition) {
    printOffset(2);
    printf("CONDITIONS:\n");
    printConditionWithOffset(condition, 3);
}


void printQuery(query_t *query) {
    printQueryType(query);
    printf("\tPARENT:%s\n", query->documentName);
    if (query->type == SELECT || query->type == DELETE) {
        printSelectOrDelete(query);
    }
    if (query->type == INSERT) {
        printInsertQuery(query);
    }
    if (query->type == UPDATE) {
        printUpdateQuery(query);
    }

}

