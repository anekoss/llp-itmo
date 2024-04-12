#include "stdint.h"

#define initInode {.size = 0, .id = 0, .pos = 0, .status = OPEN};
typedef enum {
    CLOSE = 0,
    OPEN = 1,
    REMOVE = 2
} inodeStatus_t;

typedef struct {
    uint64_t id;
    inodeStatus_t status;
    uint64_t size;
    uint64_t pos;
} inode_t;
