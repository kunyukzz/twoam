#ifndef TYPES_H
#define TYPES_H

#include "define.h"

typedef struct {
    u32 width;
    u32 height;
    char *name;
} application_config_t;

typedef struct game_entry_t {
    application_config_t config;

    b8 (*init)(struct game_entry_t *game_inst);
    b8 (*update)(struct game_entry_t *game_inst, f32 delta);
    b8 (*render)(struct game_entry_t *game_inst, f32 delta);
    void (*resize)(struct game_entry_t *game_inst, u32 width, u32 height);

    void *game_state;
} game_entry_t;

#endif // TYPES_H
