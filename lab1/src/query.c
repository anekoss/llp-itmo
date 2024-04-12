#include "../include/query.h"

query_t *createQuery(queryType_t type, char *documentName, condition_t *condition, document_t *newDocument) {
    query_t *query = malloc(sizeof(query_t));
    if (query) {
        *query = initQuery;
        if (condition) {
            query->condition = condition;
        }
        query->type = type;
        if (newDocument) {
            query->newDocument = newDocument;
        }
        if (documentName && strlen(documentName) <= 12) {
            query->documentName = documentName;
        }
        return query;
    }
    return NULL;
}

void destroyQuery(query_t *query) {
    if (query) {
        free(query);
    }
}

query_t *createInsertQuery(char *parentDocumentName, document_t *newDocument, condition_t *condition) {
    return createQuery(INSERT, parentDocumentName, condition, newDocument);
}

query_t *createUpdateQuery(char *documentName, document_t *newDocument, condition_t *condition) {
    return createQuery(UPDATE, documentName, condition, newDocument);
}

query_t *createDeleteQuery(char *documentName, condition_t *condition) {
    return createQuery(DELETE, documentName, condition, NULL);
}

query_t *createSelectQuery(char *documentName, condition_t *condition) {
    return createQuery(SELECT, documentName, condition, NULL);
}

queryResult_t *createQueryResult() {
    queryResult_t *queryResult = malloc(sizeof(queryResult_t));
    queryResult->count = 0;
    queryResult->documents = NULL;
    return queryResult;
}

void addQueryResult(queryResult_t *queryResult, document_t *document) {
    queryResult->documents = realloc(queryResult->documents, ++queryResult->count * sizeof(document_t));
    uint64_t count = queryResult->count - 1;
    queryResult->documents[count] = document;
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

int64_t *getIndexForDocumentUpdate(document_t *newDocument, document_t *oldDocument) {
    int64_t *indexElements = malloc(sizeof(int64_t) * newDocument->meta.count);
    for (int64_t i = 0; i < newDocument->meta.count; i++) {
        indexElements[i] = checkDocumentByCondition(oldDocument, createCondition(OP_EQ_KEY_TYPE,
                                                                                 newDocument->elements[i], NULL));
        if (indexElements[i] == -1) {
            free(indexElements);
            return NULL;
        }
    }
    return indexElements;
}

bool treeQueryExecute(file_t *file, int64_t parentIndex, int64_t index, queryResult_t *queryResult, query_t *query) {
    inode_t inode = getInodeByIndex(file, index);
    if (inode.status == CLOSE) {
        document_t *document = readDocument(file, index);
        bool result = true;
        if (document) {
            int64_t indexElement = checkDocumentByCondition(document, query->condition);
            if (indexElement != -1) {
                switch (query->type) {
                    case DELETE: {
                        if (queryResult)
                            addQueryResult(queryResult, document);
                        result = deleteDocument(file, &document->meta.id);
                        break;
                    }
                    case UPDATE: {
                        int64_t *indexElements = getIndexForDocumentUpdate(query->newDocument, document);
                        if (indexElements) {
                            for (int64_t i = 0; i < query->newDocument->meta.count; i++) {
                                result = result & updateDocument(file, document, indexElements[i],
                                                                 query->newDocument->elements[i]);
                            }
                            if (queryResult) {
                                document_t *updateDocument = readDocument(file, document->meta.id);
                                addQueryResult(queryResult, updateDocument);
                            }
                        }
                        break;
                    }
                    case SELECT: {
                        result = true;
                        if (queryResult)
                            addQueryResult(queryResult, document);
                        break;
                    }
                }
            }
            int64_t childId = document->meta.child;
            int64_t brotherId = document->meta.brother;
            if (!queryResult)
                destroyDocument(document);
            if (query->type == DELETE && result || childId == -1 ||
                !treeQueryExecute(file, parentIndex, childId, queryResult, query)) {
                if (brotherId == -1 ||
                    (parentIndex != index &&
                     !treeQueryExecute(file, parentIndex, brotherId, queryResult, query))) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool insertExecute(file_t *file, int64_t index, queryResult_t *queryResult, query_t *query) {
    if (query->type == INSERT) {
        if (index == -1) {
            if (query->condition) {
                return false;
            }
        }
        if (index != -1&&query->condition) {
            inode_t inode = getInodeByIndex(file, index);
            if (inode.status == CLOSE) {
                document_t *document = readDocument(file, index);
                int64_t indexElement = checkDocumentByCondition(document, query->condition);
                destroyDocument(document);
                if (indexElement == -1) {
                    return false;
                }
            }
        }
        bool result = insertDocument(file, &index, query->newDocument);
        if (queryResult) {
            addQueryResult(queryResult, readDocument(file, index));
        }
        return result;
    }
    return false;
}


bool executeQuery(file_t *file, query_t *query, queryResult_t *queryResult) {
    int64_t parentIndex = file->header.root;
    if (query->documentName) {
        meta_t meta = findDocumentMetaByName(file, query->documentName);
        parentIndex = meta.id;
    }
    if (query->type == INSERT) {
        return insertExecute(file, parentIndex, queryResult, query);
    }
    return treeQueryExecute(file, parentIndex, parentIndex, queryResult, query);
}


bool findDocumentByCondition(file_t *file, int64_t index, queryResult_t *queryResult, query_t *query) {
    inode_t inode = getInodeByIndex(file, index);
    if (inode.status == CLOSE) {
        document_t *document = readDocument(file, index);
        bool result = true;
        if (document) {
            int64_t indexElement = checkDocumentByCondition(document, query->condition);
            if (indexElement != -1) {
                switch (query->type) {
                    case DELETE: {
                        if (queryResult)
                            addQueryResult(queryResult, document);
                        result = deleteDocument(file, &document->meta.id);
                        break;
                    }
                    case UPDATE: {
                        int64_t *indexElements = getIndexForDocumentUpdate(query->newDocument, document);
                        if (indexElements) {
                            for (int64_t i = 0; i < query->newDocument->meta.count; i++) {
                                result = result & updateDocument(file, document, indexElements[i],
                                                                 query->newDocument->elements[i]);
                            }
                            if (queryResult) {
                                document_t *updateDocument = readDocument(file, document->meta.id);
                                addQueryResult(queryResult, updateDocument);
                            }
                        }
                        break;
                    }
                    case SELECT: {
                        result = true;
                        if (queryResult)
                            addQueryResult(queryResult, document);
                        break;
                    }
                }
            }
            int64_t documentId = document->meta.id;
            int64_t childId = document->meta.child;
            int64_t brotherId = document->meta.brother;
            destroyDocument(document);
            if (query->type == DELETE && result || childId == -1 ||
                !findDocumentByCondition(file, childId, queryResult, query)) {
                if (brotherId == -1 ||
                    (documentId != index &&
                     !findDocumentByCondition(file, brotherId, queryResult, query))) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

void printQueryResult(queryResult_t *queryResult) {
    if (queryResult->count == 0) {
        printf("EMPTY\n");
    }
    for (int64_t i = 0; i < queryResult->count; i++) {
        printDoc(queryResult->documents[i]);
    }
}

bool printTree(file_t *file, int64_t *parentId, int64_t level, queryResult_t *result) {
    if (parentId == NULL || *parentId == -1) {
        int64_t rootIndex = file->header.root;
        return printTree(file, &rootIndex, level, result);
    }
    inode_t node = getInodeByIndex(file, *parentId);
    if (node.status == CLOSE) {
        fseek(file->F, node.pos, SEEK_SET);
        document_t *document = readDocument(file, *parentId);
        if (document) {
            printf("%s", document->meta.name);
            printf("%s\n", "{");
            ++level;
            for (uint64_t i = 0; i < level; i++) {
                printf("%c", '\t');
            }
            int64_t child = document->meta.child;
            if (child != -1) {
                printTree(file, &child, level, result);
            }
            --level;
            printf("%s\n", "}");
            for (uint64_t i = 0; i < level; i++) {
                printf("%c", '\t');
            }
            int64_t brother = document->meta.brother;
            if (brother != -1) {
                printTree(file, &brother, level, result);
                printf("%s\n", "}");
            }
            if (result)
                addQueryResult(result, document);
            if (!result)
                destroyDocument(document);
        }
        *parentId = -1;
        return true;
    }

    return false;
}

bool printDocumentTree(file_t *file, char *documentName) {
    int64_t parentIndex = file->header.root;
    if (documentName) {
        meta_t meta = findDocumentMetaByName(file, documentName);

        parentIndex = meta.id;
    }
    int64_t level = 0;
    queryResult_t *queryResult = createQueryResult();
    bool result = printTree(file, &parentIndex, level, queryResult);
    printQueryResult(queryResult);
    destroyQueryResult(queryResult);
    return result;
}