#include "doc.h"
#include "list.h"

#define C_DEFAULT 10000
#define BUF_SIZE 500000000

typedef struct {
    uint64_t firstPosition;
    uint64_t endPosition;
    uint64_t count;
    int64_t root;
} head_t;

typedef struct {
    head_t header;
    FILE *F;
    sortedList_t list;
} file_t;

#define initFileHeader (head_t){.count=0, .firstPosition=0, .root=-1,.endPosition=sizeof(head_t) + sizeof(inode_t) * C_DEFAULT};


file_t *createFile(char *fileName);

void closeFile(file_t *file);

document_t *readDocument(file_t *file, uint64_t index);

meta_t readMeta(file_t *file, uint64_t index);

element_t *readElement(file_t *file);

int64_t writeDocument(file_t *file, document_t *document, int64_t brotherId);

void writeElement(file_t *file, element_t *elem);

bool writeNewListNode(file_t *file, uint64_t count);

bool writeHeader(file_t *file);

bool reallocateNewInode(file_t *file);

bool move(file_t *file, int64_t *oldPos, int64_t *newPos, uint64_t size);

bool updateInodeStatus(file_t *file, uint64_t id, int8_t status);

bool updateInodePosition(file_t *file, uint64_t id, uint64_t pos);

bool updateInodeSize(file_t *file, uint64_t id, uint64_t size);

inode_t getInodeByIndex(file_t *file, uint64_t index);

uint64_t getDocumentSize(file_t *file);

int64_t getDocumentId(file_t *file);

meta_t findDocumentByParentDocumentAndName(file_t *file, char *documentName, meta_t meta);

meta_t findDocumentMetaByName(file_t *file, char *documentName);
