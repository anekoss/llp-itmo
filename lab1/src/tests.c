#include "../include/query.h"
#include "sys/time.h"

void test_create_and_insert_time() {
    printf("INSERT TEST\n");
    // создаем файл
    file_t *file = createFile("file");
    // создаем корневой документ
    document_t *rootDocument = createDocument("root");
    addElement(rootDocument, stringElement("stringAttr1", "meow"));
    addElement(rootDocument, intElement("intAttr1", 1234));
    addElement(rootDocument, doubleElement("doubleAttr1", 999.12));
    addElement(rootDocument, booleanElement("boolAttr1", true));
    addElement(rootDocument, stringElement("stringAttr1", "meowmeowmeow"));
    // вставляем корневой документ
    query_t *insertRootQuery = createInsertQuery(NULL, rootDocument, NULL);
    executeQuery(file, insertRootQuery, NULL);
    // вставляем 50 000 детей
    char childDocumentName[12];
    struct timeval start, end;
    for (int64_t i = 0; i < 500; i++) {
        gettimeofday(&start, NULL);
        for (int64_t j = 0; j < 100; j++) {
            snprintf(childDocumentName, sizeof(childDocumentName), "%d", i * 10 + j);
            // создаем ребенка
            document_t *childDocument = createDocument(childDocumentName);
            addElement(childDocument, booleanElement("boolAttr1", true));
            addElement(childDocument, stringElement("stringAttr1", "meowmeowmeow"));
            addElement(childDocument, stringElement("stringAttr1", "meow"));
            // вставляем ребенка для документа с именем rootDocument
            query_t *insertChildQuery = createInsertQuery("root", childDocument, NULL);
            executeQuery(file, insertChildQuery, NULL);
            destroyDocument(childDocument);
            destroyQuery(insertChildQuery);
        }
        gettimeofday(&end, NULL);
        printf("%d\t%lu\n", (i + 1) * 100, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
    }
    destroyQuery(insertRootQuery);
    destroyDocument(rootDocument);
    closeFile(file);

}

void test_delete_time() {
    printf("DELETE TEST\n");
    // создаем файл
    file_t *file = createFile("file");
    // создаем корневой документ
    document_t *rootDocument = createDocument("root");
    addElement(rootDocument, stringElement("stringAttr1", "meow"));
    addElement(rootDocument, intElement("intAttr1", 1234));
    addElement(rootDocument, doubleElement("doubleAttr1", 999.12));
    addElement(rootDocument, booleanElement("boolAttr1", true));
    addElement(rootDocument, stringElement("stringAttr1", "meowmeowmeow"));
    // вставляем корневой документ
    query_t *insertRootQuery = createInsertQuery(NULL, rootDocument, NULL);
    executeQuery(file, insertRootQuery, NULL);
    char childDocumentName[12];
    // вставляем 5000 детей
    for (int64_t i = 0; i < 5000; i++) {
        snprintf(childDocumentName, sizeof(childDocumentName), "%d", i);
        // создаем ребенка
        document_t *childDocument = createDocument(childDocumentName);
        addElement(childDocument, booleanElement("boolAttr1", true));
        addElement(childDocument, stringElement("stringAttr1", "meowmeowmeow"));
        addElement(childDocument, stringElement("stringAttr1", "meow"));
        // вставляем ребенка для документа с именем rootDocument
        query_t *insertChildQuery = createInsertQuery("root", childDocument, NULL);
        executeQuery(file, insertChildQuery, NULL);
        destroyDocument(childDocument);
        destroyQuery(insertChildQuery);
    }
    struct timeval start, end;
    // удаляем 5000 документов
    for (int64_t i = 0; i < 100; i++) {
        gettimeofday(&start, NULL);
        // удаляем документ с именем (i * 50) + j
        for (int64_t j = 0; j < 50; j++) {
            snprintf(childDocumentName, sizeof(childDocumentName), "%d", (i * 50) + j);
            query_t *deleteQuery = createDeleteQuery(childDocumentName, NULL);
            executeQuery(file, deleteQuery, NULL);
            destroyQuery(deleteQuery);
        }
        gettimeofday(&end, NULL);
        printf("%d\t%lu\n", 5000 - i * 50, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
    }
    destroyQuery(insertRootQuery);
    destroyDocument(rootDocument);
    closeFile(file);

}

void test_insert_update_time() {
    printf("UPDATE TEST\n");
    // создаем файл
    file_t *file = createFile("file");
    // создаем корневой документ
    document_t *rootDocument = createDocument("root");
    addElement(rootDocument, stringElement("stringAttr1", "meow"));
    addElement(rootDocument, intElement("intAttr1", 1234));
    addElement(rootDocument, doubleElement("doubleAttr1", 999.12));
    addElement(rootDocument, booleanElement("boolAttr1", true));
    addElement(rootDocument, stringElement("stringAttr1", "meowmeowmeow"));
    // вставляем корневой документ
    query_t *insertRootQuery = createInsertQuery(NULL, rootDocument, NULL);
    executeQuery(file, insertRootQuery, NULL);

    // создаем документ для хранения ключа, типа и значения обновляемых элементов
    document_t *updateDocument = createDocument("updateDoc");
    addElement(updateDocument, stringElement("stringAttr1", "updateValue"));
    addElement(updateDocument, booleanElement("boolAttr1", true));

    // создаем запрос для обновления всего документного дерева
    query_t *updateQuery = createUpdateQuery(NULL, updateDocument, NULL);

    struct timeval start, end;
    char childDocumentName[12];
    for (int64_t i = 0; i < 100; i++) {
        //вставляем 100 детей корневого документа
        for (int64_t j = 0; j < 100; j++) {
            snprintf(childDocumentName, sizeof(childDocumentName), "%d", i * 100 + j);
            // создаем ребенка
            document_t *childDocument = createDocument(childDocumentName);
            addElement(childDocument, booleanElement("boolAttr1", true));
            addElement(childDocument, stringElement("stringAttr1", "meowmeowmeow"));
            addElement(childDocument, stringElement("stringAttr1", "meow"));
            // вставляем ребенка для документа с именем rootDocument
            query_t *insertChildQuery = createInsertQuery("root", childDocument, NULL);
            executeQuery(file, insertChildQuery, NULL);
            destroyDocument(childDocument);
            destroyQuery(insertChildQuery);
        }
        gettimeofday(&start, NULL);
        // выполняем обновление документного дерева
        executeQuery(file, updateQuery, NULL);
        gettimeofday(&end, NULL);
        printf("%d\t%lu\n", (i + 1) * 100, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
    }
    destroyQuery(insertRootQuery);
    destroyDocument(rootDocument);
    destroyDocument(updateDocument);
    destroyQuery(updateQuery);
    closeFile(file);
}

void test_insert_and_select_time() {
    printf("SELECT TEST\n");
    // создаем файл
    file_t *file = createFile("file");
    // создаем корневой документ
    document_t *rootDocument = createDocument("root");
    addElement(rootDocument, stringElement("stringAttr1", "meow"));
    addElement(rootDocument, intElement("intAttr1", 1234));
    addElement(rootDocument, doubleElement("doubleAttr1", 999.12));
    addElement(rootDocument, booleanElement("boolAttr1", true));
    addElement(rootDocument, stringElement("stringAttr1", "meowmeowmeow"));
    // вставляем корневой документ
    query_t *insertRootQuery = createInsertQuery(NULL, rootDocument, NULL);
    executeQuery(file, insertRootQuery, NULL);
    char childDocumentName[12];
    struct timeval start, end;
    condition_t *condition = createCondition(OP_EQ, booleanElement("boolAttr1", true), NULL);
    query_t *selectQuery = createSelectQuery(NULL, condition);
    for (int64_t i = 0; i < 100; i++) {
        //вставляем 100 детей корневого документа
        for (int64_t j = 0; j < 100; j++) {
            snprintf(childDocumentName, sizeof(childDocumentName), "%d", i * 100 + j);
            // создаем ребенка
            document_t *childDocument = createDocument(childDocumentName);
            addElement(childDocument, booleanElement("boolAttr1", true));
            addElement(childDocument, stringElement("stringAttr1", "meowmeowmeow"));
            addElement(childDocument, stringElement("stringAttr1", "meow"));
            // вставляем ребенка для документа с именем rootDocument
            query_t *insertChildQuery = createInsertQuery("root", childDocument, NULL);
            executeQuery(file, insertChildQuery, NULL);
            destroyDocument(childDocument);
            destroyQuery(insertChildQuery);
        }
        gettimeofday(&start, NULL);
        // выполняем выборку с условием наличия в документе элемента "boolAttr1" = true
        executeQuery(file, selectQuery, NULL);
        gettimeofday(&end, NULL);
        printf("%d\t%lu\n", (i + 1) * 100, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
    }
}

//void test_update_time() {
//    // создаем файл
//    file_t *file = createFile("file");
//    // создаем корневой документ
//    document_t *rootDocument = createDocument("root");
//    addElement(rootDocument, stringElement("stringAttr1", "meow"));
//    addElement(rootDocument, intElement("intAttr1", 1234));
//    addElement(rootDocument, doubleElement("doubleAttr1", 999.12));
//    addElement(rootDocument, booleanElement("boolAttr1", true));
//    addElement(rootDocument, stringElement("stringAttr1", "meowmeowmeow"));
//    // вставляем корневой документ
//    query_t *query = createInsertQuery(NULL, rootDocument, NULL);
//    executeQuery(file, query, NULL);
//    char childDocumentName[12];
//    // вставляем 5000 детей
//    for (uint64_t i = 0; i < 50000; i++) {
//        snprintf(childDocumentName, sizeof(childDocumentName), "%d", i);
//        document_t *childDocument = createDocument(childDocumentName);
//        addElement(childDocument, booleanElement("boolAttr1", true));
//        addElement(childDocument, stringElement("stringAttr1", "meowmeowmeow"));
//        addElement(childDocument, stringElement("stringAttr2", "meow"));
//        query = createInsertQuery("rootDocument", childDocument, NULL);
//        executeQuery(file, query, NULL);
//    }
//    // создаем документ для хранения ключа, типа и значения обновляемых элементов
//    document_t *updateDocument = createDocument("updateDoc");
//    addElement(updateDocument, stringElement("stringAttr2", "updateValue"));
//    addElement(updateDocument, booleanElement("boolAttr1", true));
//    struct timeval start, end;
//    for (int64_t i = 49999; i >= 0; i--) {
//        gettimeofday(&start, NULL);
//        snprintf(childDocumentName, sizeof(childDocumentName), "%d", i);
//        // выполняем обновление документного дерева
//        query = createUpdateQuery(childDocumentName, updateDocument, NULL);
//        executeQuery(file, query, NULL);
//        gettimeofday(&end, NULL);
//        printf("%d\t%lu\n", i, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
//    }
//}

void test_insert_delete_time() {
    printf("INSERT AND DELETE TEST\n");
    // создаем файл
    file_t *file = createFile("file");
    // создаем корневой документ
    document_t *rootDocument = createDocument("root");
    addElement(rootDocument, stringElement("stringAttr1", "meow"));
    addElement(rootDocument, intElement("intAttr1", 1234));
    addElement(rootDocument, doubleElement("doubleAttr1", 999.12));
    addElement(rootDocument, booleanElement("boolAttr1", true));
    addElement(rootDocument, stringElement("stringAttr1", "meowmeowmeow"));
    // вставляем корневой документ
    query_t *query = createInsertQuery(NULL, rootDocument, NULL);
    executeQuery(file, query, NULL);
    char childDocumentName[12];
    // вставляем 5000 детей
    struct timeval start, end;
    int64_t count = 1;
    for (int64_t i = 0; i < 20; i++) {
        gettimeofday(&start, NULL);
        // вставляем 500 * (i+1) детей для документа с именем rootDocument
        for (int j = 0; j < 500 * (i + 1); j++) {
            snprintf(childDocumentName, sizeof(childDocumentName), "%d", 500 * i + j);
            document_t *childDocument = createDocument(childDocumentName);
            addElement(childDocument, booleanElement("boolAttr1", true));
            addElement(childDocument, stringElement("stringAttr1", "meowmeowmeow"));
            addElement(childDocument, stringElement("stringAttr2", "meow"));
            query_t *insertQuery = createInsertQuery("root", childDocument, NULL);
            executeQuery(file, insertQuery, NULL);
            if (j % 100 == 0) {
                gettimeofday(&end, NULL);
                printf("%d\t%lu\n", count * 100, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
                count++;
                gettimeofday(&start, NULL);
            }
        }
        // удаляем 300 * (i + 1) документов по имени 500 * i  + j
        gettimeofday(&start, NULL);
        for (int64_t j = 0; j < 300 * (i + 1); j++) {
            snprintf(childDocumentName, sizeof(childDocumentName), "%d", 500 * i + j);
            query_t *deleteQuery = createDeleteQuery(childDocumentName, NULL);
            executeQuery(file, deleteQuery, NULL);
            if (j % 100 == 0) {
                gettimeofday(&end, NULL);
//                printf("%d\t%lu\n", count * 100, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
//                count++;
                gettimeofday(&start, NULL);
            }
        }

    }
}

void test_file_size() {
    printf("FILE SIZE TEST\n");
    // создаем файл
    file_t *file = createFile("file1");
    // создаем корневой документ
    document_t *rootDocument = createDocument("root");
    addElement(rootDocument, stringElement("stringAttr1", "meow"));
    addElement(rootDocument, intElement("intAttr1", 1234));
    addElement(rootDocument, doubleElement("doubleAttr1", 999.12));
    addElement(rootDocument, booleanElement("boolAttr1", true));
    addElement(rootDocument, stringElement("stringAttr1", "meowmeowmeow"));
    // вставляем корневой документ
    query_t *query = createInsertQuery(NULL, rootDocument, NULL);
    executeQuery(file, query, NULL);
    char childDocumentName[12];
    executeQuery(file, query, NULL);
    for (uint64_t i = 0; i < 50000; i++) {
        snprintf(childDocumentName, sizeof(childDocumentName), "%d", i);
        document_t *childDocument = createDocument(childDocumentName);
        addElement(childDocument, booleanElement("boolAttr1", true));
        addElement(childDocument, stringElement("stringAttr1", "meowmeowmeow"));
        addElement(childDocument, stringElement("stringAttr2", "meow"));
        query = createInsertQuery("root", childDocument, NULL);
        executeQuery(file, query, NULL);
        printf("%d\t", i);
        printf("%llu\n", file->header.endPosition);
    }
}

int main() {
    test_file_size();
//    test_create_and_insert_time();
//    test_delete_time();
//    test_insert_update_time();
//    test_insert_and_select_time();
//    test_insert_delete_time();
//    test_file_size();
}
