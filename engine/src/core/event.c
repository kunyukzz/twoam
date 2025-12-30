#include "event.h"
#include "memory.h"
#include "container/darray.h"

#define MAX_MSG_CODE 8192

typedef struct {
    void *recipient;
    on_event fn;
} reg_event_t;

typedef struct entry_event {
    reg_event_t *events;
} entry_event_t;

typedef struct {
    // lookup table
    entry_event_t registered[MAX_MSG_CODE];
} event_system_t;

static b8 initialized = false;
static event_system_t g_ev = {0};

b8 event_sys_init(void)
{
    if (initialized == true)
    {
        return false;
    }
    mem_zero(&g_ev, sizeof(g_ev));

    initialized = true;

    LOGI("Event System Init");
    return true;
}

void event_sys_kill(void)
{
    for (u16 i = 0; i < MAX_MSG_CODE; ++i)
    {
        if (g_ev.registered[i].events != 0)
        {
            da_destroy(g_ev.registered[i].events);
            g_ev.registered[i].events = 0;
        }
    }

    initialized = false;
    LOGI("Event System Kill");
}

b8 event_reg(u16 code, void *recipient, on_event on_event)
{
    if (initialized == false)
    {
        return false;
    }
    if (g_ev.registered[code].events == 0)
    {
        g_ev.registered[code].events = da_create(reg_event_t);
    }

    u64 reg_count = da_length(g_ev.registered[code].events);
    for (u64 i = 0; i < reg_count; ++i)
    {
        if (g_ev.registered[code].events[i].recipient == recipient)
        {
            // TODO: maybe gives warning ?
            return false;
        }
    }

    reg_event_t ev = {.recipient = recipient, .fn = on_event};
    da_push(g_ev.registered[code].events, ev);

    return true;
}

b8 event_unreg(u16 code, void *recipient, on_event on_event)
{
    if (initialized == false)
    {
        return false;
    }
    if (g_ev.registered[code].events == 0)
    {
        // TODO: maybe gives warning ?
        return false;
    }

    u64 reg_count = da_length(g_ev.registered[code].events);
    for (u64 i = 0; i < reg_count; ++i)
    {
        reg_event_t ev = g_ev.registered[code].events[i];
        if (ev.recipient == recipient && ev.fn == on_event)
        {
            // found, remove it
            reg_event_t pop_ev;
            da_pop_at(g_ev.registered[code].events, i, &pop_ev);
            return true;
        }
    }

    // not found
    return false;
}

b8 event_emit(u16 code, void *sender, event_ctx_t ctx)
{
    if (initialized == false)
    {
        return false;
    }
    if (g_ev.registered[code].events == 0)
    {
        // TODO: maybe gives warning ?
        return false;
    }

    u64 reg_count = da_length(g_ev.registered[code].events);
    for (u64 i = 0; i < reg_count; ++i)
    {
        reg_event_t ev = g_ev.registered[code].events[i];
        if (ev.fn(code, sender, ev.recipient, ctx))
        {
            // has been handle, not sending to other recipient
            return true;
        }
    }

    return false;
}
