#include "input.h"
#include "event.h"
#include "memory.h"

#define KEYNAME(k) [KEY_##k] = #k

typedef struct {
    b8 keys[KEY_COUNT];
    u8 mods;
} kbd_state;

typedef struct {
    i16 x, y;
    i8 mouse_wheel_delta;
    u8 buttons[MB_COUNT];
} mouse_state;

typedef struct {
    kbd_state kbd_current;
    kbd_state kbd_prev;
    mouse_state mouse_current;
    mouse_state mouse_prev;
} input_system_t;

static b8 initialized = false;
static input_system_t g_input = {0};

void input_sys_init(void)
{
    initialized = true;
    LOGI("Input System Init");
}

void input_sys_kill(void)
{
    initialized = false;
    LOGI("Input System Kill");
}

void input_sys_update(f64 delta)
{
    (void)delta;
    if (!initialized)
    {
        return;
    }

    mem_copy(&g_input.kbd_prev, &g_input.kbd_current, sizeof(kbd_state));
    mem_copy(&g_input.mouse_prev, &g_input.mouse_current, sizeof(mouse_state));
}

void input_process_key(keys key, b8 pressed)
{
    if (g_input.kbd_current.keys[key] != pressed)
    {
        g_input.kbd_current.keys[key] = pressed;

        event_ctx_t evc;
        evc.data.u16[0] = (u16)key;
        event_emit(pressed ? EV_KEY_PRESSED : EV_KEY_RELEASED, 0, evc);
    }
}

void input_process_mods(u8 mods) { g_input.kbd_current.mods = mods; }

void input_process_mouse_button(mouse_buttons button, b8 pressed)
{
    if (g_input.mouse_current.buttons[button] != pressed)
    {
        g_input.mouse_current.buttons[button] = pressed;

        event_ctx_t evc;
        evc.data.u16[0] = (u16)button;
        event_emit(pressed ? EV_MB_PRESSED : EV_MB_RELEASED, 0, evc);
    }
}

void input_process_mouse_move(i16 x, i16 y)
{
    if (g_input.mouse_current.x != x || g_input.mouse_current.y != y)
    {
        g_input.mouse_current.x = x;
        g_input.mouse_current.y = y;

        event_ctx_t evc;
        evc.data.u16[0] = (u16)x;
        evc.data.u16[1] = (u16)y;
        event_emit(EV_MOUSE_MOVE, 0, evc);
    }
}

void input_process_mouse_wheel(i8 z_delta)
{
    event_ctx_t evc;
    evc.data.u8[0] = (u8)z_delta;
    event_emit(EV_MOUSE_WHEEL, 0, evc);
}

b8 input_keydown(keys key)
{
    if (!initialized)
    {
        return false;
    }

    return g_input.kbd_current.keys[key] == true;
}

b8 input_keyup(keys key)
{
    if (!initialized)
    {
        return true;
    }

    return g_input.kbd_current.keys[key] == false;
}

b8 input_was_keydown(keys key)
{
    if (!initialized)
    {
        return false;
    }

    return g_input.kbd_prev.keys[key] == true;
}

b8 input_was_keyup(keys key)
{
    if (!initialized)
    {
        return true;
    }

    return g_input.kbd_prev.keys[key] == false;
}

b8 input_mouse_button_down(mouse_buttons button)
{
    if (!initialized)
    {
        return false;
    }

    return g_input.mouse_current.buttons[button] == true;
}

b8 input_mouse_button_up(mouse_buttons button)
{
    if (!initialized)
    {
        return true;
    }

    return g_input.mouse_current.buttons[button] == false;
}

b8 input_mouse_was_button_down(mouse_buttons button)
{
    if (!initialized)
    {
        return false;
    }

    return g_input.mouse_prev.buttons[button] == true;
}

b8 input_mouse_was_button_up(mouse_buttons button)
{
    if (!initialized)
    {
        return true;
    }

    return g_input.mouse_prev.buttons[button] == false;
}

void input_get_mouse_pos(i32 *x, i32 *y)
{
    if (!initialized)
    {
        *x = 0;
        *y = 0;
        return;
    }

    *x = g_input.mouse_current.x;
    *y = g_input.mouse_current.y;
}

void input_get_mouse_prev_pos(i32 *x, i32 *y)
{
    if (!initialized)
    {
        *x = 0;
        *y = 0;
        return;
    }

    *x = g_input.mouse_prev.x;
    *y = g_input.mouse_prev.y;
}

// clang-format off
static const char* key_names[KEY_COUNT] = {
	// letters
    KEYNAME(A), KEYNAME(B),KEYNAME(C), KEYNAME(D), KEYNAME(E), KEYNAME(F),
	KEYNAME(G), KEYNAME(H), KEYNAME(I), KEYNAME(J), KEYNAME(K), KEYNAME(L),
	KEYNAME(M), KEYNAME(N), KEYNAME(O), KEYNAME(P), KEYNAME(Q), KEYNAME(R),
	KEYNAME(S), KEYNAME(T), KEYNAME(U), KEYNAME(V), KEYNAME(W), KEYNAME(X),
	KEYNAME(Y), KEYNAME(Z),

	// numbers (top row)
	KEYNAME(0), KEYNAME(1), KEYNAME(2), KEYNAME(3), KEYNAME(4),
	KEYNAME(5), KEYNAME(6), KEYNAME(7), KEYNAME(8), KEYNAME(9),

	// functions
	KEYNAME(F1), KEYNAME(F2), KEYNAME(F3), KEYNAME(F4), KEYNAME(F5), KEYNAME(F6),
	KEYNAME(F7), KEYNAME(F8), KEYNAME(F9), KEYNAME(F10), KEYNAME(F11), KEYNAME(F12),

};
// clang-format on

const char *input_sys_keyname(keys k)
{
    if (k < KEY_COUNT && key_names[k]) return key_names[k];
    return "UNKNOWN";
}
