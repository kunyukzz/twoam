#include "platform.h"

#if PLATFORM_LINUX
#    include "core/event.h"
#    include "core/input.h"

#    include <xcb/xcb.h> // xcb_connection_t, xcb_window_t, xcb_screen_t, xcb_atom_t
#    include <X11/Xlib.h>     // Display, XOpenDisplay
#    include <X11/Xlib-xcb.h> // XGetXCBConnection
#    include <X11/XKBlib.h>
#    include <X11/keysym.h>
#    include <sys/time.h>

#    include <stdlib.h>
#    include <string.h>

typedef struct {
    Display *display;
    xcb_connection_t *conn;
    xcb_screen_t *screen;
    xcb_window_t window;
    xcb_atom_t wm_proto;
    xcb_atom_t wm_delete;
} linux_system_t;

static keys translate_keycode(u32 keycode);

b8 platform_init(platform_system_t *ps, const char *name, i32 width,
                 i32 height)
{
    ps->internal_data = malloc(sizeof(linux_system_t));
    linux_system_t *linux = (linux_system_t *)ps->internal_data;

    // setup for X11 server
    linux->display = XOpenDisplay(NULL);
    XAutoRepeatOff(linux->display);

    // setup for xcb
    linux->conn = XGetXCBConnection(linux->display);
    if (xcb_connection_has_error(linux->conn))
    {
        LOGF("Failed to connect to X server via xcb");
        return false;
    }

    const struct xcb_setup_t *setup = xcb_get_setup(linux->conn);

    int screen_num = 0;
    xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
    for (i32 i = screen_num; i < screen_num; i++)
    {
        xcb_screen_next(&it);
    }
    linux->screen = it.data;
    linux->window = xcb_generate_id(linux->conn);

    // all mask event for xcb window
    u32 values = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
                 XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
                 XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
                 XCB_EVENT_MASK_KEY_RELEASE;
    u32 value_list[] = {linux->screen->black_pixel, values};
    u32 event_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;

    // window setup creation
    xcb_void_cookie_t cookie = xcb_create_window(
        linux->conn, XCB_COPY_FROM_PARENT, linux->window, linux->screen->root,
        100,         // TODO: make center to screen for position x
        100,         // TODO: make center to screen for position y
        (u16)width,  // width
        (u16)height, // height
        0,           // border
        XCB_WINDOW_CLASS_INPUT_OUTPUT, linux->screen->root_visual, event_mask,
        value_list);

    // check window creation immediately
    xcb_generic_error_t *error = xcb_request_check(linux->conn, cookie);
    if (error)
    {
        LOGF("Failed to create window. Error code: %d", error->error_code);
        free(error);
        return false;
    }

    // set window name
    xcb_change_property(linux->conn, XCB_PROP_MODE_REPLACE, linux->window,
                        XCB_ATOM_WM_NAME, XCB_ATOM_STRING,
                        8, // has to be 8-bit??
                        (u32)strlen(name), name);

    // clang-format off
	xcb_intern_atom_cookie_t wm_cookie_del = xcb_intern_atom(linux->conn, 0, strlen("WM_DELETE_WINDOW"), "WM_DELETE_WINDOW");
	xcb_intern_atom_cookie_t wm_cookie_proto = xcb_intern_atom(linux->conn, 0, strlen("WM_PROTOCOLS"), "WM_PROTOCOLS");
	xcb_intern_atom_reply_t *wm_reply_del = xcb_intern_atom_reply(linux->conn, wm_cookie_del, NULL);
	xcb_intern_atom_reply_t *wm_reply_proto = xcb_intern_atom_reply(linux->conn, wm_cookie_proto, NULL);
    // clang-format on

    // check if atoms were retrieved successfully
    if (!wm_reply_del || !wm_reply_proto)
    {
        LOGF("Failed to get WM protocol atoms");
        if (wm_reply_del) free(wm_reply_del);
        if (wm_reply_proto) free(wm_reply_proto);
        return false;
    }

    linux->wm_delete = wm_reply_del->atom;
    linux->wm_proto = wm_reply_proto->atom;

    // format: 32 for atoms (which are 32-bit values)
    // data_len: 1 (setting 1 atom)
    xcb_change_property(linux->conn, XCB_PROP_MODE_REPLACE, linux->window,
                        wm_reply_proto->atom, XCB_ATOM_ATOM, 32, 1,
                        &wm_reply_del->atom);

    free(wm_reply_del);
    free(wm_reply_proto);

    // map the window
    xcb_map_window(linux->conn, linux->window);
    i32 result = xcb_flush(linux->conn);
    if (result <= 0)
    {
        LOGF("Failed to flush connection. XCB error");
        return false;
    }

    LOGI("Platform Linux Init");
    return true;
}

void platform_kill(platform_system_t *ps)
{
    linux_system_t *linux = (linux_system_t *)ps->internal_data;

    XAutoRepeatOn(linux->display);
    xcb_destroy_window(linux->conn, linux->window);
    XCloseDisplay(linux->display);
    free(linux);
    ps->internal_data = NULL;

    LOGI("Platform Linux Kill");
}

b8 platform_pump(platform_system_t *ps)
{
    linux_system_t *linux = (linux_system_t *)ps->internal_data;
    xcb_generic_event_t *event;
    b8 quit = false;

    while ((event = xcb_poll_for_event(linux->conn)) != NULL)
    {
        xcb_client_message_event_t *cm;

        switch (event->response_type & ~0x80)
        {
        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE:
        {
            xcb_key_press_event_t *kbd_ev = (xcb_key_press_event_t *)event;
            b8 pressed = event->response_type == XCB_KEY_PRESS;
            xcb_keycode_t code = kbd_ev->detail;
            KeySym key_sym = XkbKeycodeToKeysym(linux->display, code, 0, 0);

            keys key = translate_keycode((u32)key_sym);
            input_process_key(key, pressed);

            u8 mods = 0;
            if (kbd_ev->state & XCB_MOD_MASK_SHIFT) mods |= MOD_SHIFT;
            if (kbd_ev->state & XCB_MOD_MASK_CONTROL) mods |= MOD_CTRL;
            if (kbd_ev->state & XCB_MOD_MASK_1) mods |= MOD_ALT;
            if (kbd_ev->state & XCB_MOD_MASK_4) mods |= MOD_SUPER;
            if (kbd_ev->state & XCB_MOD_MASK_LOCK) mods |= MOD_CAPS;

            input_process_mods(mods);
        }
        break;

        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
        {
            xcb_button_press_event_t *m_ev = (xcb_button_press_event_t *)event;
            b8 pressed = m_ev->response_type == XCB_BUTTON_PRESS;
            mouse_buttons mouse_button = MB_COUNT;
            switch (m_ev->detail)
            {
            case XCB_BUTTON_INDEX_1: mouse_button = MB_LEFT; break;
            case XCB_BUTTON_INDEX_2: mouse_button = MB_MIDDLE; break;
            case XCB_BUTTON_INDEX_3: mouse_button = MB_RIGHT; break;
            }

            if (mouse_button != MB_COUNT)
                input_process_mouse_button(mouse_button, pressed);
        }
        break;

        case XCB_MOTION_NOTIFY:
        {
            xcb_motion_notify_event_t *move_ev =
                (xcb_motion_notify_event_t *)event;
            input_process_mouse_move(move_ev->event_x, move_ev->event_y);
        }
        break;

        case XCB_CONFIGURE_NOTIFY:
        {
            xcb_configure_notify_event_t *cfg_ev =
                (xcb_configure_notify_event_t *)event;

            event_ctx_t c;
            c.data.u16[0] = cfg_ev->width;
            c.data.u16[1] = cfg_ev->height;

            event_emit(EV_RESIZED, 0, c);
        }
        break;

        case XCB_CLIENT_MESSAGE:
        {
            cm = (xcb_client_message_event_t *)event;

            if (cm->data.data32[0] == linux->wm_delete)
            {
                quit = true;

                event_ctx_t c;
                c.data.u8[0] = (u8)cm->window;
                event_emit(EV_APP_QUIT, 0, c);
            }
        }
        break;

        default: break;
        }
        free(event);
    }
    return !quit;
}

f64 platform_get_time(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (f64)now.tv_sec + (f64)now.tv_nsec * 1e-9;
}

void platform_sleep(u64 ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, 0);
}

void *platform_alloc(u64 size, b8 aligned)
{
    (void)aligned;
    return malloc(size);
}

void platform_free(void *block, b8 aligned)
{
    (void)aligned;
    free(block);
}

void *platform_memzero(void *block, u64 size)
{
    return memset(block, 0, size);
}

void *platform_memcopy(void *dest, const void *src, u64 size)
{
    return memcpy(dest, src, size);
}

void *platform_memmove(void *dest, const void *src, u64 size)
{
    return memmove(dest, src, size);
}

void *platform_memsets(void *dest, i32 value, u64 size)
{
    return memset(dest, value, size);
}

keys translate_keycode(u32 keycode)
{
    switch (keycode)
    {
    // letters
    case XK_a:
    case XK_A: return KEY_A;
    case XK_b:
    case XK_B: return KEY_B;
    case XK_c:
    case XK_C: return KEY_C;
    case XK_d:
    case XK_D: return KEY_D;
    case XK_e:
    case XK_E: return KEY_E;
    case XK_f:
    case XK_F: return KEY_F;
    case XK_g:
    case XK_G: return KEY_G;
    case XK_h:
    case XK_H: return KEY_H;
    case XK_i:
    case XK_I: return KEY_I;
    case XK_j:
    case XK_J: return KEY_J;
    case XK_k:
    case XK_K: return KEY_K;
    case XK_l:
    case XK_L: return KEY_L;
    case XK_m:
    case XK_M: return KEY_M;
    case XK_n:
    case XK_N: return KEY_N;
    case XK_o:
    case XK_O: return KEY_O;
    case XK_p:
    case XK_P: return KEY_P;
    case XK_q:
    case XK_Q: return KEY_Q;
    case XK_r:
    case XK_R: return KEY_R;
    case XK_s:
    case XK_S: return KEY_S;
    case XK_t:
    case XK_T: return KEY_T;
    case XK_u:
    case XK_U: return KEY_U;
    case XK_v:
    case XK_V: return KEY_V;
    case XK_w:
    case XK_W: return KEY_W;
    case XK_x:
    case XK_X: return KEY_X;
    case XK_y:
    case XK_Y: return KEY_Y;
    case XK_z:
    case XK_Z: return KEY_Z;

    // numbers (top row)
    case XK_0:
    case XK_parenright: return KEY_0;
    case XK_1:
    case XK_exclam: return KEY_1;
    case XK_2:
    case XK_at: return KEY_2;
    case XK_3:
    case XK_numbersign: return KEY_3;
    case XK_4:
    case XK_dollar: return KEY_4;
    case XK_5:
    case XK_percent: return KEY_5;
    case XK_6:
    case XK_asciicircum: return KEY_6;
    case XK_7:
    case XK_ampersand: return KEY_7;
    case XK_8:
    case XK_asterisk: return KEY_8;
    case XK_9:
    case XK_parenleft: return KEY_9;

    // functions
    case XK_F1: return KEY_F1;
    case XK_F2: return KEY_F2;
    case XK_F3: return KEY_F3;
    case XK_F4: return KEY_F4;
    case XK_F5: return KEY_F5;
    case XK_F6: return KEY_F6;
    case XK_F7: return KEY_F7;
    case XK_F8: return KEY_F8;
    case XK_F9: return KEY_F9;
    case XK_F10: return KEY_F10;
    case XK_F11: return KEY_F11;
    case XK_F12: return KEY_F12;

    // controls
    case XK_Escape: return KEY_ESCAPE;
    case XK_Return: return KEY_ENTER;
    case XK_Tab: return KEY_TAB;
    case XK_BackSpace: return KEY_BACKSPACE;
    case XK_space: return KEY_SPACE;

    // modifiers
    case XK_Shift_L:
    case XK_Shift_R: return KEY_SHIFT;
    case XK_Control_L:
    case XK_Control_R: return KEY_CTRL;
    case XK_Alt_L:
    case XK_Alt_R: return KEY_ALT;
    case XK_Super_L:
    case XK_Super_R: return KEY_SUPER;

    // arrows
    case XK_Up: return KEY_UP;
    case XK_Down: return KEY_DOWN;
    case XK_Left: return KEY_LEFT;
    case XK_Right: return KEY_RIGHT;

    // navigations
    case XK_Insert: return KEY_INSERT;
    case XK_Delete: return KEY_DELETE;
    case XK_Home: return KEY_HOME;
    case XK_End: return KEY_END;
    case XK_Page_Up: return KEY_PAGE_UP;
    case XK_Page_Down: return KEY_PAGE_DOWN;

    // numpad
    case XK_Num_Lock: return KEY_NUM_LOCK;
    case XK_KP_0: return KEY_KP_0;
    case XK_KP_1: return KEY_KP_1;
    case XK_KP_2: return KEY_KP_2;
    case XK_KP_3: return KEY_KP_3;
    case XK_KP_4: return KEY_KP_4;
    case XK_KP_5: return KEY_KP_5;
    case XK_KP_6: return KEY_KP_6;
    case XK_KP_7: return KEY_KP_7;
    case XK_KP_8: return KEY_KP_8;
    case XK_KP_9: return KEY_KP_9;
    case XK_KP_Add: return KEY_KP_ADD;
    case XK_KP_Subtract: return KEY_KP_SUBTRACT;
    case XK_KP_Multiply: return KEY_KP_MULTIPLY;
    case XK_KP_Divide: return KEY_KP_DIVIDE;
    case XK_KP_Enter: return KEY_KP_ENTER;
    case XK_KP_Decimal: return KEY_KP_DECIMAL;

    // symbols
    case XK_minus:
    case XK_underscore: return KEY_MINUS;
    case XK_equal:
    case XK_plus: return KEY_EQUALS;
    case XK_bracketleft:
    case XK_braceleft: return KEY_LEFT_BRACKET;
    case XK_bracketright:
    case XK_braceright: return KEY_RIGHT_BRACKET;
    case XK_backslash:
    case XK_bar: return KEY_BACKSLASH;
    case XK_semicolon:
    case XK_colon: return KEY_SEMICOLON;
    case XK_apostrophe:
    case XK_quotedbl: return KEY_APOSTROPHE;
    case XK_grave:
    case XK_asciitilde: return KEY_GRAVE;
    case XK_comma:
    case XK_less: return KEY_COMMA;
    case XK_period:
    case XK_greater: return KEY_PERIOD;
    case XK_slash:
    case XK_question: return KEY_SLASH;

    default: return KEY_UNKNOWN;
    }
}

#endif
