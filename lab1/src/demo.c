#include "../include/query.h"

int main() {
    file_t *file = createFile("file");

    document_t *rootDocument = createDocument("root");
    if (rootDocument) {
        addElement(rootDocument, stringElement("str1", "meow"));
        addElement(rootDocument, intElement("int1", 1234));
        addElement(rootDocument, doubleElement("double1", 999.12));
        addElement(rootDocument, booleanElement("bool1", true));
        addElement(rootDocument, stringElement("str2", "meow"));
    }

    document_t *childDocument = createDocument("child");
    if (childDocument) {
        addElement(childDocument, intElement("int1", 383));
        addElement(childDocument, intElement("int2", 444));
    }

    document_t *grandChildDocument = createDocument("grandChild");
    if (grandChildDocument) {
        addElement(grandChildDocument, intElement("int1", 3));
        addElement(grandChildDocument, intElement("int2", 100));
        addElement(grandChildDocument, booleanElement("bool1", false));
    }

    query_t *insertRootDocument = createInsertQuery(NULL, rootDocument, NULL);
    condition_t *condEquals = condEqual(stringElement("str1", "meow"));
    query_t *insertChildDocument = createInsertQuery("root", childDocument, condEquals);
    query_t *insertGrChildDocument = createInsertQuery("child", grandChildDocument, NULL);

    executeQuery(file, insertRootDocument, NULL);
    executeQuery(file, insertChildDocument, NULL);
    executeQuery(file, insertGrChildDocument, NULL);

    destroyDocument(rootDocument);
    destroyDocument(childDocument);
    destroyDocument(grandChildDocument);
    destroyQuery(insertRootDocument);
    destroyCondition(condEquals);
    destroyQuery(insertChildDocument);
    destroyQuery(insertGrChildDocument);

    printf("%s\n", "INSERT DOCUMENTS");
    printf("%s\n", "DOCUMENT TREE STATUS:");
    printDocumentTree(file, "root");

    queryResult_t *selectChildResult = createQueryResult();
    query_t *selectQueryChild = createSelectQuery("child", NULL);
    executeQuery(file, selectQueryChild, selectChildResult);

    printf("%s\n", "SELECT DOCUMENT \"child\" AND CHILD DOCUMENTS");
    printf("%s\n", "SELECT RESULT:");
    printQueryResult(selectChildResult);

    destroyQuery(selectQueryChild);
    destroyQueryResult(selectChildResult);

    condition_t *conditionOr = condAnd(condLess(intElement("int1", 1000)), condGreater(intElement("int1", 300)));
    queryResult_t *selectResult = createQueryResult();
    query_t *selectQueryWithCondition = createSelectQuery("root", conditionOr);
    executeQuery(file, selectQueryWithCondition, selectResult);

    printf("%s\n", "SELECT DOCUMENT \"root\" AND CHILD DOCUMENTS BY CONDITION");
    printf("%s\n", "SELECT RESULT:");
    printQueryResult(selectResult);

    destroyCondition(conditionOr);
    destroyQuery(selectQueryWithCondition);
    destroyQueryResult(selectResult);

    condition_t *updateCondition = condEqual(booleanElement("bool1", true));
    document_t *updateRootDocument = createDocument("updateDoc");
    addElement(updateRootDocument, stringElement("str2", "updateMeow"));
    query_t *updateRootQuery = createUpdateQuery("root", updateRootDocument, updateCondition);
    executeQuery(file, updateRootQuery, createQueryResult());

    printf("%s\n", "UPDATE DOCUMENT \"root\" AND CHILD DOCUMENTS BY CONDITION");
    printf("%s\n", "DOCUMENT TREE STATUS:");
    printDocumentTree(file, "root");

    destroyCondition(updateCondition);
    destroyDocument(updateRootDocument);
    destroyQuery(updateRootQuery);

    query_t *deleteGrChildQuery = createDeleteQuery("grandChild", NULL);
    executeQuery(file, deleteGrChildQuery, NULL);
    printf("%s\n", "DELETE DOCUMENT \"grandChild\" DOCUMENT");
    printf("%s\n", "DOCUMENT TREE STATUS:");
    printDocumentTree(file, "root");

    destroyQuery(deleteGrChildQuery);

    query_t *deleteRootQuery = createDeleteQuery("root", NULL);
    executeQuery(file, deleteRootQuery, NULL);

    printf("%s\n", "DELETE DOCUMENT \"root\" AND DOCUMENT CHILDREN DOCUMENTS");
    printf("%s\n", "DOCUMENT TREE STATUS:");
    printDocumentTree(file, "root");

    destroyQuery(deleteRootQuery);

    closeFile(file);
}
