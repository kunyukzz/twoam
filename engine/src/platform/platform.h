#ifndef PLATFORM_H
#define PLATFORM_H

#include "core/define.h"

typedef struct {
    void *internal_data;
} platform_system_t;

b8 platform_init(platform_system_t *ps, const char *name, i32 width,
                 i32 height);

void platform_kill(platform_system_t *ps);

b8 platform_pump(platform_system_t *ps);

f64 platform_get_time(void);

void platform_sleep(u64 ms);

#endif // PLATFORM_H
