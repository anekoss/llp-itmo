#include "../include/doc.h"

document_t *createDocument(char *name) {
    if (name && strlen(name) <= 12) {
        document_t *doc = malloc(sizeof(document_t));
        if (doc) {
            doc->elements = NULL;
            doc->meta = initDocumentMeta;
            memset(doc->meta.name, 0, 13);
            strncpy(doc->meta.name, name, 13);
            return doc;
        }
    }
    return NULL;
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

bool addElement(document_t *document, element_t *elem) {
    element_t **elems = realloc(document->elements, sizeof(element_t *) * (document->meta.count + 1));
    if (elems) {
        document->elements = elems;
        document->elements[document->meta.count++] = elem;
        return true;
    }
    return false;
}

void printDoc(document_t *doc) {
    if (!doc) {
        printf("NULL");
        return;
    }
    printf("%s", doc->meta.name);
    printf("{\n");
#ifdef META
    printMeta(doc);
#endif
    for (int64_t i = 0; i < doc->meta.count; ++i) {
        printElement(doc->elements[i]);
    }
    printf("}\n");
}

void printMeta(document_t *doc) {
    printf("\t meta_t.name %s \n", doc->meta.name);
    printf("\t meta_t.id %llu \n", doc->meta.id);
    printf("\t meta_t.size %llu \n", doc->meta.size);
    printf("\t meta_t.count %llu \n", doc->meta.count);
    printf("\t meta_t.brother %lld \n", doc->meta.brother);
    printf("\t meta_t.child %lld \n", doc->meta.child);
}


uint64_t calcDocumentSize(document_t *doc) {
    uint64_t size = sizeof(meta_t);
    for (uint64_t i = 0; i < doc->meta.count; i++) {
        size += sizeof(uint8_t) + 13 * sizeof(int8_t); // type ? key
        element_t el = *doc->elements[i];
        switch (el.type) {
            case INT:
                size += sizeof(int32_t);
                break;
            case DOUBLE:
                size += sizeof(double);
                break;
            case BOOLEAN:
                size += sizeof(uint8_t);
                break;
            case STRING:
                size += sizeof(uint32_t);
                size += sizeof(char) * el.stringValue.length;
                break;
        }
    }
    return size;
}
