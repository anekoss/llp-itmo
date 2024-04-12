#include "file.h"

bool updateDocument(file_t *file, document_t *document, uint64_t indexelem, element_t *element);

bool deleteDocument(file_t *file, int64_t *parentIndex);

bool updateString(file_t *file, inode_t *node, meta_t *meta, element_t *oldEl, element_t *newEl);

bool insertDocument(file_t *file, int64_t *parentIndex, document_t *document);

