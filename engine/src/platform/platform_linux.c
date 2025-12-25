#include "platform.h"

#if PLATFORM_LINUX

#    include <xcb/xcb.h> // xcb_connection_t, xcb_window_t, xcb_screen_t, xcb_atom_t
#    include <X11/Xlib.h>     // Display, XOpenDisplay
#    include <X11/Xlib-xcb.h> // XGetXCBConnection
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
            // TODO: implement key press & release
        }
        break;

        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
        {
            // TODO: implement button press & release
        }
        break;

        case XCB_MOTION_NOTIFY:
        {
            // TODO: implement mouse cursor
        }
        break;

        case XCB_CONFIGURE_NOTIFY:
        {
            // TODO: implement
        }
        break;

        case XCB_CLIENT_MESSAGE:
        {
            cm = (xcb_client_message_event_t *)event;

            if (cm->data.data32[0] == linux->wm_delete)
            {
                LOGD("WM_DELETE_WINDOW received - quitting");
                quit = true;
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

#endif
