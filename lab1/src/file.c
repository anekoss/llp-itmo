#include "../include/file.h"

file_t *createFile(char *fileName) {
    file_t *file = malloc(sizeof(file_t));
    if (file) {
        file->F = fopen(fileName, "w+b");
        file->header = initFileHeader;
        file->list = initSortedList;
        writeHeader(file);
        writeNewListNode(file, C_DEFAULT);
        return file;
    }
    return NULL;
}

void closeFile(file_t *file) {
    if (file) {
        if (file->F) {
            fclose(file->F);
        }
        destroyList(&file->list);
        free(file);
    }
}

document_t *readDocument(file_t *file, uint64_t index) {
    inode_t inode = getInodeByIndex(file, index);
    if (inode.status == CLOSE) {
        fseek(file->F, (long) inode.pos, SEEK_SET);
        meta_t meta;
        fread(&meta, sizeof(meta_t), 1, file->F);
        document_t *doc = createDocument(meta.name);
        if (doc) {
            doc->meta = meta;
            doc->elements = malloc(sizeof(element_t *) * meta.count);
            if (doc->elements) {
                for (uint64_t i = 0; i < meta.count; i++) {
                    doc->elements[i] = readElement(file);
                }
                return doc;
            }
            destroyDocument(doc);
            return NULL;
        }
    }
    return NULL;
}

meta_t readMeta(file_t *file, uint64_t index) {
    inode_t inode = getInodeByIndex(file, index);
    fseek(file->F, inode.pos, SEEK_SET);
    meta_t meta;
    fread(&meta, sizeof(meta_t), 1, file->F);
    return meta;
}

element_t *readElement(file_t *file) {
    element_t *elem = malloc(sizeof(element_t));
    if (elem) {
        fread(&elem->type, sizeof(uint8_t), 1, file->F);
        if (fread(&elem->key, sizeof(char), 13, file->F) == 13) {
            switch (elem->type) {
                case INT: {
                    fread(&elem->integerValue, sizeof(int32_t), 1, file->F);
                    break;
                }
                case DOUBLE: {
                    fread(&elem->doubleValue, sizeof(double), 1, file->F);
                    break;
                }
                case BOOLEAN: {
                    fread(&elem->boolValue, sizeof(uint8_t), 1, file->F);
                    break;
                }
                case STRING: {
                    fread(&elem->stringValue.length, sizeof(uint32_t), 1, file->F);
                    elem->stringValue.str = malloc(sizeof(uint8_t) * elem->stringValue.length);
                    if (elem->stringValue.str) {
                        fread(elem->stringValue.str, sizeof(uint8_t), elem->stringValue.length, file->F);
                    } else {
                        destroyElement(elem);
                        return NULL;
                    }
                    break;
                }
            }
            return elem;
        }
    }
    return NULL;
}

int64_t writeDocument(file_t *file, document_t *document, int64_t brotherId) {
    uint64_t pos = ftello(file->F);
    if (document) {
        document->meta.size = calcDocumentSize(document);
        document->meta.child = -1;
        document->meta.brother = brotherId;
        if (!file->list.front && !reallocateNewInode(file)) {
            return false;
        }
        int64_t diff = (int64_t) file->list.front->size - (int64_t) document->meta.size;
        if (diff >= 0) {
            inode_t node = getInodeByIndex(file, file->list.front->index);
            if (node.status != REMOVE ||
                !updateInodeStatus(file, file->list.front->index, CLOSE)) {
                return false;
            }
            // Заполняем заголовок документа:
            document->meta.id = file->list.front->index;
            free(popMaxSizeNode(&file->list));
        } else {
            if (file->list.back->size != 0 && !reallocateNewInode(file) ||
                !updateInodeStatus(file, file->list.back->index, CLOSE) ||
                !updateInodePosition(file, file->list.back->index, file->header.endPosition) ||
                !updateInodeSize(file, file->list.back->index, document->meta.size)) {
                return 0;
            }
            document->meta.id = file->list.back->index;
            free(popMinSizeNode(&file->list));
            // Обновляем размер файла
            file->header.endPosition += (int64_t) document->meta.size;
            writeHeader(file);
        }
        inode_t inode = getInodeByIndex(file, document->meta.id);
        fseek(file->F, (long) inode.pos, SEEK_SET);
        fwrite(&document->meta, sizeof(meta_t), 1, file->F);
        for (uint64_t i = 0; i < document->meta.count; i++) {
            writeElement(file, document->elements[i]);
        }
        if (file->header.root == -1) {
            file->header.root = document->meta.id;
            if (!writeHeader(file)) {
                return false;
            }
        }
        fseek(file->F, (long) pos, SEEK_SET);
        return document->meta.id;
    }
    return -1;
}

void writeElement(file_t *file, element_t *elem) {
    fwrite(&elem->type, sizeof(uint8_t), 1, file->F);
    fwrite(&elem->key, sizeof(char), 13, file->F);
    switch (elem->type) {
        case INT: {
            fwrite(&elem->integerValue, sizeof(int32_t), 1, file->F);
            return;
        }
        case DOUBLE: {
            fwrite(&elem->doubleValue, sizeof(double), 1, file->F);
            return;
        }
        case BOOLEAN: {
            fwrite(&elem->boolValue, sizeof(uint8_t), 1, file->F);
            return;
        }
        case STRING: {
            fwrite(&elem->stringValue.length, sizeof(uint32_t), 1, file->F);
            fwrite(elem->stringValue.str, sizeof(uint8_t), elem->stringValue.length, file->F);
            return;
        }
    }
}

bool writeNewListNode(file_t *file, uint64_t count) {
    fseek(file->F, (long) (sizeof(head_t) + sizeof(inode_t) * file->header.count), SEEK_SET);
    inode_t inode = initInode;
    uint64_t writtenCount = 0;
    for (uint64_t i = 0; i < count; i++) {
        fwrite(&inode, sizeof(inode_t), 1, file->F);
        writtenCount++;
        insertNode(&file->list, createNode(file->header.count++, inode.size));
    }
    return writtenCount == count;
}

bool writeHeader(file_t *file) {
    uint64_t pos = ftell(file->F);
    fseek(file->F, 0, SEEK_SET);
    fwrite(&file->header, sizeof(head_t), 1, file->F);
    fseek(file->F, (long) pos, SEEK_SET);
    return true;
}

bool reallocateNewInode(file_t *file) {
    int64_t newPos;
    int64_t oldPos = (int64_t) (file->header.firstPosition + sizeof(head_t) + sizeof(inode_t) * file->header.count);
    int64_t neededSpace = sizeof(inode_t) * C_DEFAULT;
    uint64_t availableSpace = file->header.firstPosition;
    while (availableSpace < neededSpace) {
        fseek(file->F, (long) oldPos, SEEK_SET);
        int64_t docId = getDocumentId(file);
        uint64_t docSize = getDocumentSize(file);
        inode_t inode = getInodeByIndex(file, docId);
        if (inode.status == REMOVE) {
            if (!removeNodeByIndex(&file->list, docId) ||
                !updateInodeStatus(file, docId, OPEN) ||
                !updateInodePosition(file, docId, 0) ||
                !updateInodeSize(file, docId, 0)) {
                return false;
            }
            insertNode(&file->list, createNode(docId, 0));
            oldPos += (int64_t) docSize;
        } else if (inode.status == CLOSE) {
            if (file->list.front && file->list.front->size >= docSize) {
                inode_t gapIndex = getInodeByIndex(file, file->list.front->index);
                if (gapIndex.status != REMOVE ||
                    !updateInodeStatus(file, file->list.front->index, OPEN) ||
                    !updateInodePosition(file, file->list.front->index, 0) ||
                    !updateInodeSize(file, file->list.front->index, 0)) {
                    return false;
                }
                newPos = gapIndex.pos;
                // Записываем дырку обратно в список, но уже с размером 0:
                listNode_t *node = popMaxSizeNode(&file->list);
                node->size = 0;
                insertNode(&file->list, node);
            } else {
                newPos = file->header.endPosition;
                // Обновляем размер файла:
                file->header.endPosition += (uint64_t) docSize;
                writeHeader(file);
            }
            // Перемещаем документ, обновляем смещение в его индексе:
            if (!updateInodePosition(file, docId, newPos) ||
                !move(file, &oldPos, &newPos, docSize)) {
                return false;
            }
        } else {
            return false;
        }
        availableSpace += docSize;
    }
    // Записываем новые ноды и сохраняем остаток места:
    file->header.firstPosition = availableSpace % sizeof(inode_t);
    if (!writeNewListNode(file, availableSpace / sizeof(inode_t))) {
        return false;
    }
    writeHeader(file);
    return true;
}


bool move(file_t *file, int64_t *oldPos, int64_t *newPos, uint64_t size) {
    while (size) {
        // Определяем размер буфера и аллоцируем его:
        int64_t bufSize;
        if (size > BUF_SIZE) {
            bufSize = BUF_SIZE;
        } else {
            bufSize = (int64_t) size;
        }
        size -= bufSize;
        uint8_t *buf = malloc(bufSize);

        // Перемещаемся на прошлый адрес и заполняем буфер:
        fseek(file->F, (long) *oldPos, SEEK_SET);
        if (!fread(buf, bufSize, 1, file->F)) {
            free(buf);
            return false;
        }
        *oldPos += bufSize;

        // Перемещаемся на новый адрес и пишем из буфера:
        fseek(file->F, (long) *newPos, SEEK_SET);
        if (!fwrite(buf, bufSize, 1, file->F)) {
            free(buf);
            return false;
        }
        *newPos += bufSize;
        free(buf);
    }
    return true;
}

bool updateInodeStatus(file_t *file, uint64_t id, int8_t status) {
    bool writtenCount = 0;
    if (id < file->header.count) {
        int64_t pos = ftello(file->F);
        inode_t inode;
        fseek(file->F, (sizeof(head_t) + sizeof(inode) * id), SEEK_SET);
        fread(&inode, sizeof(inode_t), 1, file->F);
        inode.status = (inodeStatus_t) status;
        fseek(file->F, -sizeof(inode_t), SEEK_CUR);
        writtenCount = fwrite(&inode, sizeof(inode_t), 1, file->F);
        fseek(file->F, pos, SEEK_SET);
    }
    return writtenCount;
}

bool updateInodePosition(file_t *file, uint64_t id, uint64_t pos) {
    bool writtenCount = 0;
    if (id < file->header.count) {
        uint64_t oldPos = ftello(file->F);
        inode_t inode;
        fseek(file->F, (sizeof(head_t) + sizeof(inode) * id), SEEK_SET);
        fread(&inode, sizeof(inode_t), 1, file->F);
        inode.pos = pos;
        fseek(file->F, -sizeof(inode_t), SEEK_CUR);
        writtenCount = fwrite(&inode, sizeof(inode_t), 1, file->F);
        fseek(file->F, oldPos, SEEK_SET);
    }
    return writtenCount;
}

bool updateInodeSize(file_t *file, uint64_t id, uint64_t size) {
    bool writtenCount = 0;
    if (id < file->header.count) {
        int64_t pos = ftello(file->F);
        inode_t inode;
        fseek(file->F, (long) (sizeof(head_t) + sizeof(inode) * id), SEEK_SET);
        fread(&inode, sizeof(inode_t), 1, file->F);
        inode.size = size;
        fseek(file->F, (long) -sizeof(inode_t), SEEK_CUR);
        writtenCount = fwrite(&inode, sizeof(inode_t), 1, file->F);
        fseek(file->F, (long) pos, SEEK_SET);
    }
    return writtenCount;
};

inode_t getInodeByIndex(file_t *file, uint64_t index) {
    uint64_t pos = ftello(file->F);
    inode_t inode = initInode;
    if (index < file->header.count) {
        fseek(file->F, (long) (sizeof(head_t) + sizeof(inode_t) * index), SEEK_SET);
        fread(&inode, sizeof(inode), 1, file->F);
    }
    fseek(file->F, (long) pos, SEEK_CUR);
    return inode;
}


uint64_t getDocumentSize(file_t *file) {
    uint64_t offset = ftell(file->F);
    meta_t meta;
    fread(&meta, sizeof(meta_t), 1, file->F);
    fseek(file->F, (long) offset, SEEK_SET);
    return meta.size;
}


int64_t getDocumentId(file_t *file) {
    uint64_t pos = ftell(file->F);
    meta_t meta;
    fread(&meta, sizeof(meta_t), 1, file->F);
    fseek(file->F, (long) pos, SEEK_SET);
    return meta.id;
}


meta_t findDocumentByParentDocumentAndName(file_t *file, char *documentName, meta_t meta) {
    if (!strcmp(documentName, meta.name)) {
        return meta;
    }
    int64_t child = meta.child;
    if (child != -1) {
        meta_t childMeta = readMeta(file, child);
        return findDocumentByParentDocumentAndName(file, documentName, childMeta);
    }
    int64_t brother = meta.brother;
    if (brother != -1) {
        meta_t brotherMeta = readMeta(file, brother);
        return findDocumentByParentDocumentAndName(file, documentName, brotherMeta);
    }
    return (meta_t) {0};
}

meta_t findDocumentMetaByName(file_t *file, char *documentName) {
    int64_t rootIndex =
            file->header.root;
    if (rootIndex != -1) {
        inode_t rootInode = getInodeByIndex(file, rootIndex);
        if (rootInode.status == CLOSE) {
            meta_t meta = readMeta(file, rootIndex);
            if (!strcmp(meta.name, documentName)) {
                return meta;
            }
            return findDocumentByParentDocumentAndName(file, documentName, meta);
        }
    }
    return (meta_t) {0};
}


