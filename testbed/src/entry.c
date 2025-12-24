#include "game.h"
#include <stdlib.h>

b8 game_set(game_entry_t *out)
{
    out->config.name = "2AM Engine Testbed";
    out->config.width = 800;
    out->config.height = 600;

    out->init = game_init;
    out->update = game_update;
    out->render = game_render;
    out->resize = game_resize;

    out->game_state = malloc(sizeof(game_t));

    return true;
}
