#include "application.h"

#include <stdlib.h>

static b8 initialized = false;
static application_t g_app = {0};

b8 application_init(game_entry_t *game_instance)
{
    if (initialized)
    {
        LOGE("Application already running");
        return false;
    }

    g_app.game = game_instance;

    g_app.is_running = true;
    g_app.is_suspended = false;

    if (!platform_init(&g_app.platform, game_instance->config.name,
                       (i16)game_instance->config.width,
                       (i16)game_instance->config.height))
    {
        return false;
    }

    if (!g_app.game->init(g_app.game))
    {
        LOGF("Game failed to initialized");
        return false;
    }
    g_app.game->resize(g_app.game, g_app.width, g_app.height);

    initialized = true;

    LOGI("Engine Initialized");
    return true;
}

b8 application_run(void)
{
    while (g_app.is_running)
    {
        if (!platform_pump(&g_app.platform))
        {
            g_app.is_running = false;
        }

        if (!g_app.is_suspended)
        {
            if (!g_app.game->update(g_app.game, (f32)0))
            {
                LOGF("Game update failed, shutting down");
                g_app.is_running = false;
                break;
            }

            if (!g_app.game->render(g_app.game, (f32)0))
            {
                LOGF("Game render failed, shutting down");
                g_app.is_running = false;
                break;
            }
        }
    }

    g_app.is_running = false;

    platform_kill(&g_app.platform);

    free(g_app.game->game_state);

    LOGI("Engine Shutdown");
    return true;
}
