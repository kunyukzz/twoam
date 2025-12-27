#include "entry.h"
#include "application.h"

b8 engine_entry(void)
{
    game_entry_t game_instance = {0};

    if (!game_set(&game_instance))
    {
        LOGF("Cannot initialized game");
        return false;
    }

    if (!game_instance.init || !game_instance.render ||
        !game_instance.update || !game_instance.resize)
    {
        LOGF("Function pointers must be assigned!");
        return false;
    }

    if (!application_init(&game_instance))
    {
        LOGF("Engine failed to create");
        return false;
    }

    application_run();
    return true;
}

// clang-format off
__attribute__((constructor))
static void engine_auto_run(void)
{
    (void)engine_entry();
}
