#ifndef MEMORY_H
#define MEMORY_H

#include "define.h"

typedef enum {
    MEM_UNKNOWN = 0x00,

    MEM_ARENA,
    MEM_ENGINE,
    MEM_GAME,
    MEM_RENDER,
    MEM_AUDIO,
    MEM_STRING,
    MEM_ARRAY,
    MEM_TEXTURE,
    MEM_MESH,
    MEM_SHADER,

    MEM_MAX_TAG
} memtag_t;

b8 memory_sys_init(u64 total_size);

void memory_sys_kill(void);

AM2_API void *mem_alloc(u64 size, memtag_t tag);

AM2_API void mem_free(void *block, u64 size, memtag_t tag);

AM2_API void *mem_zero(void *block, u64 size);

AM2_API void *mem_copy(void *dest, const void *src, u64 size);

AM2_API void *mem_move(void *dest, const void *src, u64 size);

AM2_API void *mem_set(void *dest, i32 value, u64 size);

char *get_usage_mem(void);

#endif // MEMORY_H
