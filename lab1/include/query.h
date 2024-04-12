#include "condition.h"
#include "operation.h"

#define initQuery (query_t){.type=SELECT, .newDocument=NULL, .condition=NULL, .documentName=NULL};

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
    document_t *newDocument;
} query_t;

typedef struct queryResult {
    uint64_t count;
    document_t **documents;
} queryResult_t;


query_t *createInsertQuery(char *parentDocumentName, document_t *newDocument, condition_t *condition);

query_t *createUpdateQuery(char *documentName, document_t *newDocument, condition_t *condition);

query_t *createDeleteQuery(char *documentName, condition_t *condition);

query_t *createSelectQuery(char *documentName, condition_t *condition);

void destroyQuery(query_t *query);

queryResult_t *createQueryResult();

void destroyQueryResult(queryResult_t *queryResult);

bool executeQuery(file_t *file, query_t *query, queryResult_t *queryResult);

void printQueryResult(queryResult_t *queryResult);

bool printDocumentTree(file_t *file, char *documentName);



