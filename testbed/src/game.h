#ifndef GAME_H
#define GAME_H

#include <twoam.h>

typedef struct {
    f32 delta;
} game_t;

b8 game_init(game_entry_t *game_instance);

b8 game_update(game_entry_t *game_instance, f32 delta);

b8 game_render(game_entry_t *game_instance, f32 delta);

void game_resize(game_entry_t *game_instance, u32 width, u32 height);

#endif // GAME_H
