#include "application.h"
#include "memory.h"
#include "event.h"
#include "input.h"

#include "container/test_darray.h"

static b8 initialized = false;
static application_t g_app = {0};

b8 application_on_event(u16 code, void *sender, void *recipient,
                        event_ctx_t ctx);

b8 application_on_key(u16 code, void *sender, void *recipient,
                      event_ctx_t ctx);

b8 application_init(game_entry_t *game_instance)
{
    if (initialized)
    {
        LOGE("Application already running");
        return false;
    }

    memory_sys_init(MEBIBYTE);

    g_app.game = game_instance;

    g_app.is_running = true;
    g_app.is_suspended = false;

    input_sys_init();

    if (!event_sys_init())
    {
        LOGE("Event failed to initialized");
        return false;
    }

    if (!platform_init(&g_app.platform, game_instance->config.name,
                       (i16)game_instance->config.width,
                       (i16)game_instance->config.height))
    {
        LOGF("Platform failed to initialized");
        return false;
    }

    if (!g_app.game->init(g_app.game))
    {
        LOGF("Game failed to initialized");
        return false;
    }
    g_app.game->resize(g_app.game, g_app.width, g_app.height);

    event_reg(EV_APP_QUIT, 0, application_on_event);
    event_reg(EV_KEY_PRESSED, 0, application_on_key);
    event_reg(EV_KEY_RELEASED, 0, application_on_key);

    initialized = true;

    // dynamic_array_test();

    LOGI("Engine Initialized");
    return true;
}

b8 application_run(void)
{
    LOGI(get_usage_mem());

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

            input_sys_update(0);
        }
    }

    g_app.is_running = false;

    event_unreg(EV_APP_QUIT, 0, application_on_event);
    event_unreg(EV_KEY_PRESSED, 0, application_on_key);
    event_unreg(EV_KEY_RELEASED, 0, application_on_key);

    platform_kill(&g_app.platform);

    event_sys_kill();
    input_sys_kill();

    // free memory from game side
    platform_free(g_app.game->game_state, false);

    memory_sys_kill();

    LOGI("Engine Shutdown");
    return true;
}

b8 application_on_event(u16 code, void *sender, void *recipient,
                        event_ctx_t ctx)
{
    (void)sender;
    (void)recipient;
    (void)ctx;

    switch (code)
    {
    case EV_APP_QUIT:
    {
        LOGD("EV_APP_QUIT received. Shutdown.");
        g_app.is_running = false;
        return true;
    }
    }

    return false;
}

b8 application_on_key(u16 code, void *sender, void *recipient, event_ctx_t ctx)
{
    (void)sender;
    (void)recipient;

    if (code == EV_KEY_PRESSED)
    {
        u16 kc = ctx.data.u16[0];
        if (kc == KEY_ESCAPE)
        {
            event_ctx_t data = {0};
            event_emit(EV_APP_QUIT, 0, data);
            return true;
        }
        else
        {
            LOGD("'%s' key pressed in window", input_sys_keyname((keys)kc));
        }
    }
    else if (code == EV_KEY_RELEASED)
    {
        u16 kc = ctx.data.u16[0];
        {
            LOGD("'%s' key released in window", input_sys_keyname((keys)kc));
        }
    }

    return false;
}
