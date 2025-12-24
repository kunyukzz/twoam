#ifndef APPLICATION_H
#define APPLICATION_H

#include "define.h"
#include "types.h"

// temp
#include "platform/platform.h"

typedef struct {
    game_entry_t *game;

    b8 is_running;
    b8 is_suspended;

    u32 width;
    u32 height;
    f64 last_time;

    platform_system_t platform;
} application_t;

AM2_API b8 application_init(game_entry_t *game_instance);

AM2_API b8 application_run(void);

#endif // APPLICATION_H
