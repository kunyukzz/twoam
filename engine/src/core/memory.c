#include "memory.h"
#include "platform/platform.h"

#include <stdio.h>

#define BUFFER_SIZE 8192

struct mem_stats {
    u64 total_allocated;
    u64 tag_allocations[MEM_MAX_TAG];
    u64 alloc_count[MEM_MAX_TAG];
};

static struct mem_stats g_stats = {0};
static u64 g_mem_size = 0;

// clang-format off
static const char *tag_str[MEM_MAX_TAG] = {
    "MEM_UNKNOWN",
	"MEM_ARENA",
	"MEM_ENGINE",
	"MEM_GAME",
	"MEM_RENDER",
    "MEM_AUDIO",
	"MEM_STRING",
	"MEM_ARRAY",
	"MEM_TEXTURE", 
    "MEM_MESH",
	"MEM_SHADER",
};
// clang-format on

b8 memory_sys_init(u64 total_size)
{
    g_mem_size = total_size;
    // platform_memzero(&g_stats, sizeof(g_stats));

    LOGI("Memory System Init");
    return true;
}

void memory_sys_kill(void) { LOGI("Memory System Kill"); }

void *mem_alloc(u64 size, memtag_t tag)
{
    if (tag == MEM_UNKNOWN)
    {
        LOGW("allocation using MEM_UNKNOWN");
    }

    g_stats.total_allocated += size;
    g_stats.tag_allocations[tag] += size;
    g_stats.alloc_count[tag]++;

    void *block = platform_alloc(size, false);
    platform_memzero(block, size);

    return block;
}

void mem_free(void *block, u64 size, memtag_t tag)
{
    if (tag == MEM_UNKNOWN)
    {
        LOGW("allocation using MEM_UNKNOWN");
    }

    g_stats.total_allocated -= size;
    g_stats.tag_allocations[tag] -= size;
    g_stats.alloc_count[tag]--;

    platform_free(block, false);
}

void *mem_zero(void *block, u64 size) { return platform_memzero(block, size); }

void *mem_copy(void *dest, const void *src, u64 size)
{
    return platform_memcopy(dest, src, size);
}

void *mem_move(void *dest, const void *src, u64 size)
{
    return platform_memmove(dest, src, size);
}

void *mem_set(void *dest, i32 value, u64 size)
{
    return platform_memsets(dest, value, size);
}

char *get_usage_mem(void)
{
    static char buffer[BUFFER_SIZE];
    u64 offset = 0;

    f32 used_mib = (f32)g_stats.total_allocated / (f32)MEBIBYTE;
    f32 reserved_mib = (f32)g_mem_size / (f32)MEBIBYTE;

    offset += (u64)snprintf(buffer + offset, sizeof(buffer) - offset,
                            "Engine Memory Used: %.6f Mib / %.2f Mib\n",
                            used_mib, reserved_mib);

    for (u32 i = 0; i < MEM_MAX_TAG; ++i)
    {
        char *unit = "B";
        u32 count = (u32)g_stats.alloc_count[i];
        f32 amount = (f32)g_stats.tag_allocations[i];

        if (count == 0) continue;

        if (amount >= (f32)GIBIBYTE)
        {
            amount /= (f32)GIBIBYTE;
            unit = "Gib";
        }
        else if (amount >= (f32)MEBIBYTE)
        {
            amount /= (f32)MEBIBYTE;
            unit = "Mib";
        }
        else if (amount >= (f32)KIBIBYTE)
        {
            amount /= (f32)KIBIBYTE;
            unit = "Kib";
        }

        i32 length =
            snprintf(buffer + offset, sizeof(buffer) - offset,
                     "--> %s: [%u] %.2f%s\n", tag_str[i], count, amount, unit);

        if (length > 0 && (offset + (u32)length < BUFFER_SIZE))
        {
            offset += (u32)length;
        }
        else
            break;
    }
    return buffer;
}
