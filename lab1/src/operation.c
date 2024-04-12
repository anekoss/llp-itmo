#include "../include/operation.h"

bool insertDocument(file_t *file, int64_t *parentId, document_t *document) {
    int64_t documentId;
    if (*parentId != -1) {
        inode_t parentInode = getInodeByIndex(file, *parentId);
        meta_t parentMeta;
        if (parentInode.status != CLOSE) {
            return false;
        }
        fseek(file->F, parentInode.pos, SEEK_SET);
        if (!fread(&parentMeta, sizeof(meta_t), 1, file->F)) {
            return false;
        }
        documentId = writeDocument(file, document, parentMeta.child);
        parentInode = getInodeByIndex(file, *parentId);
        if (parentInode.status != CLOSE) {
            return false;
        }
        // Перезаписываем последнего ребёнка в заголовке родителя:
        parentMeta.child = documentId;
        fseek(file->F, parentInode.pos, SEEK_SET);
        if (!fwrite(&parentMeta, sizeof(meta_t), 1, file->F)) {
            return false;
        }
    } else {
        // Если не указан родитель, то сразу добавляем:
        documentId = writeDocument(file, document, -1);
    }
    *parentId = documentId; // передаём номер индекса для того, чтобы потом добавить детей
    return true;
}

bool deleteSameLevelDocuments(file_t *file, int64_t *documentId) {
    inode_t node = getInodeByIndex(file, *documentId);
    if (node.status == CLOSE) {
        fseek(file->F, node.pos, SEEK_SET);
        meta_t meta;
        if (fread(&meta, sizeof(meta_t), 1, file->F)) {
            int64_t child = meta.child;
            int64_t brother = meta.brother;
            if (child != -1 && !deleteDocument(file, &child)) {
                if (brother != -1 && !deleteDocument(file, &child))
                    return false;
            }
            // Добавляем дырку в список:
            insertNode(&file->list, createNode(*documentId, meta.size));
        }
        // Обновляем индекс:
        if (!updateInodeStatus(file, *documentId, REMOVE)) {
            return false;
        }
        *documentId = -1;
        return true;
    }
    return false;
}

bool deleteDocument(file_t *file, int64_t *documentId) {
    inode_t node = getInodeByIndex(file, *documentId);
    if (node.status == CLOSE) {
        fseek(file->F, node.pos, SEEK_SET);
        meta_t meta;
        if (fread(&meta, sizeof(meta_t), 1, file->F)) {
            // Удаляем детей:
            int64_t child = meta.child;
            deleteSameLevelDocuments(file, &child);
            // Добавляем дырку в список:
            insertNode(&file->list, createNode(*documentId, meta.size));
        }
        // Обновляем индекс:
        if (!updateInodeStatus(file, *documentId, REMOVE)) {
            return false;
        }
//        *documentId = -1;
        return true;
    }
    return false;
}

bool updateDocument(file_t *file, document_t *document, uint64_t indexelem, element_t *element) {
    uint64_t oldPos = ftello(file->F);
    inode_t inode = getInodeByIndex(file, document->meta.id);
    if (inode.status == CLOSE) {
        uint64_t offset = inode.pos + sizeof(meta_t) + (sizeof(uint8_t) + sizeof(char) * 13) * (indexelem + 1);
        for (uint64_t i = 0; i < indexelem; i++) {
            offset += calcElementSize(document->elements[i]);
        }
        fseek(file->F, offset, SEEK_SET);
        switch (element->type) {
            case INT:
                fwrite(&element->integerValue, sizeof(int32_t), 1, file->F);
                break;
            case DOUBLE:
                fwrite(&element->doubleValue, sizeof(double), 1, file->F);
                break;
            case BOOLEAN:
                fwrite(&element->boolValue, sizeof(uint8_t), 1, file->F);
                break;
            case STRING: {
                if (!strcmp(document->elements[indexelem]->key, element->key))
                    updateString(file, &inode, &document->meta, document->elements[indexelem], element);
            }
        }
        fseek(file->F, oldPos, SEEK_SET);
        return true;
    }
    return false;
}


bool updateString(file_t *file, inode_t *node, meta_t *meta, element_t *oldEl, element_t *newEl) {
    uint64_t newHeaderSize = 0;
    int64_t offsetOfElement = ftello(file->F) - node->pos;
    // смещение строки относительно начала документа
    int64_t delta =
            (int64_t) newEl->stringValue.length - (int64_t) oldEl->stringValue.length; // изменение размера строки
    // Если строка стала больше, то ей нужно найти новое место:
    if (delta > 0) {
        // Если документ не в конце файла, ищем дырку и переносим его:
        if (node->pos + meta->size != file->header.endPosition) {
            int64_t oldPos = node->pos;
            int64_t newPos = 0;
            struct meta_t gapMeta = *meta;
            int64_t diff = file->list.front ? (int64_t) file->list.front->size - (int64_t) meta->size - delta : -1;
            if (diff >= 0) {
                // Считываем индекс дырки и обновляем в нём смещение:
                inode_t gapinode = getInodeByIndex(file, file->list.front->index);
                if (gapinode.status != CLOSE ||
                    !updateInodePosition(file, file->list.front->index, node->pos)) {
                    return false;
                }
                gapMeta.id = file->list.front->index;
                newPos = node->pos = gapinode.pos;
                newHeaderSize = meta->size + delta;
                listNode_t *node = popMinSizeNode(&file->list);
                node->size = meta->size;
                insertNode(&file->list, node);
            } else {
                // На предыдущем месте образуется дырка, следовательно, нужны индексы:
                if ((!file->list.back || file->list.back->size) &&
                    !reallocateNewInode(file)) {
                    return false;
                }
                // Заново считываем индекс документа, поскольку он мог быть перемещён:
                *node = getInodeByIndex(file, meta->id);
                if (node->status != CLOSE) {
                    return false;
                }
                // Если документ не переместился в конец, нужно его туда переместить:
                if (node->pos + meta->size != file->header.endPosition) {
                    // Считываем INDEX_NEW индекс, делаем его INDEX_DEAD и записываем в
                    // него текущее смещение документа:
                    inode_t gapinode = getInodeByIndex(file, file->list.back->index);
                    if (gapinode.status != OPEN ||
                        !updateInodeStatus(file, file->list.back->index, REMOVE) ||
                        !updateInodePosition(file, file->list.back->index, node->pos)) {
                        return false;
                    }
                    gapMeta.id = file->list.back->index; // записываем в хедер для будущей
                    // дырки номер индекса текущей
                    listNode_t *node1 = popMinSizeNode(&file->list);
                    node1->size = meta->size;
                    insertNode(&file->list, node1);
                    oldPos = node->pos;
                    newPos = node->pos = file->header.endPosition;
                    file->header.endPosition += (int64_t) meta->size + delta;
                } else {
                    newPos = oldPos; // условие для того, чтобы не перемещать документ
                    file->header.endPosition += delta;
                }
                // Обновляем fileSize:
                if (!writeHeader(file)) {
                    return false;
                }
            }
            // Перемещаем документ, если он не оказался в конце файла,
            //  обновляем смещение в его индексе и записываем на его месте хедер
            //  дырки:
            if (newPos != oldPos) {
                if (!updateInodePosition(file, meta->id, newPos) ||
                    !move(file, &oldPos, &newPos, meta->size)) {
                    return false;
                }
                fseek(file->F, oldPos - (int64_t) meta->size, SEEK_SET);
                if (!fwrite(&gapMeta, sizeof(meta_t), 1, file->F)) {
                    return false;
                }
            }
        } else {
            file->header.endPosition += delta;
            if (!writeHeader(file)) {
                return false;
            }
        }
    }
    // Перемещаем кусок документа после строки, чтобы не перекрыть его новой
    // строкой и чтобы не было дырок:
    int64_t oldPos = (int64_t) (node->pos + offsetOfElement + sizeof(uint32_t) + oldEl->stringValue.length);
    int64_t newPos = oldPos + delta;
    if (!move(file, &oldPos, &newPos, node->pos + meta->size - oldPos)) {
        return false;
    }
    // Перезаписываем размер документа:
    meta->size = newHeaderSize ? newHeaderSize : meta->size + delta;
    fseek(file->F, node->pos, SEEK_SET);
    if (!fwrite(meta, sizeof(meta_t), 1, file->F)) {
        return false;
    }
    // Возвращаемся к началу элемента, перезаписываем его:
    fseek(file->F, node->pos + offsetOfElement, SEEK_SET);
    if (!fwrite(&newEl->stringValue.length, sizeof(uint32_t), 1, file->F) ||
        !fwrite(newEl->stringValue.str, sizeof(int8_t), newEl->stringValue.length, file->F)) {
        return false;
    }
    return true;
}

