#ifndef DOC
#define DOC

#include "element.h"

#define  initDocumentMeta  (meta_t){.id = 0, .size =0, .child =-1, .brother = -1, .count = 0};

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
} document_t;


document_t *createDocument(char *schemaName);

void destroyDocument(document_t *document);

bool addElement(document_t *document, element_t *element);

void printDoc(document_t *doc);

void printMeta(document_t *doc);

uint64_t calcDocumentSize(document_t *doc);

#endif