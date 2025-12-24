#include "entry.h"
#include "application.h"

int main(void)
{
    game_entry_t game_instance;

    if (!game_set(&game_instance))
    {
        LOGF("Cannot initialized game");
        return -1;
    }

    if (!game_instance.init || !game_instance.render ||
        !game_instance.update || !game_instance.resize)
    {
        LOGF("Function pointers must be assigned!");
        return -2;
    }

    if (!application_init(&game_instance))
    {
        LOGF("Engine failed to create");
        return 1;
    }

    if (!application_run())
    {
        LOGF("Engine not shutting down gracefully");
        return 2;
    }

    return 0;
}
