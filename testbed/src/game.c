#include "game.h"

b8 game_init(game_entry_t *game_instance)
{
    (void)game_instance;
    LOGI("Game Init Called");
    return true;
}

b8 game_update(game_entry_t *game_instance, f32 delta)
{
    (void)game_instance;
    (void)delta;
    return true;
}

b8 game_render(game_entry_t *game_instance, f32 delta)
{
    (void)game_instance;
    (void)delta;
    return true;
}

void game_resize(game_entry_t *game_instance, u32 width, u32 height)
{
    (void)game_instance;
    (void)width;
    (void)height;
}
