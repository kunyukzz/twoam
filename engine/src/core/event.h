#ifndef EVENT_H
#define EVENT_H

#include "define.h"

typedef struct {
    union
    {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;
} event_ctx_t;

// return true if handled
typedef b8 (*on_event)(u16 code, void *sender, void *recipient,
                       event_ctx_t data);

// application & user code use beyond 255
typedef enum {
    EV_APP_QUIT = 0x01,
    EV_KEY_PRESSED = 0x02,
    EV_KEY_RELEASED = 0x03,
    EV_MB_PRESSED = 0x04,
    EV_MB_RELEASED = 0x05,
    EV_MOUSE_MOVE = 0x06,
    EV_MOUSE_WHEEL = 0x07,
    EV_RESIZED = 0x08,

    MAX_EVENT_CODE = 0xFF
} event_code_t;

b8 event_sys_init(void);

void event_sys_kill(void);

AM2_API b8 event_reg(u16 code, void *recipient, on_event on_event);

AM2_API b8 event_unreg(u16 code, void *recipient, on_event on_event);

AM2_API b8 event_emit(u16 code, void *sender, event_ctx_t ctx);

#endif // EVENT_H
