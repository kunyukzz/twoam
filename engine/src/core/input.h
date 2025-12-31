#ifndef INPUT_H
#define INPUT_H

#include "define.h"

// clang-format off
typedef enum { 
	MB_LEFT, 
	MB_MIDDLE, 
	MB_RIGHT,
	MB_COUNT,
} mouse_buttons;

typedef enum {
    KEY_UNKNOWN = 0,

	// letters
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F,
    KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L,
    KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X,
    KEY_Y, KEY_Z,

	// numbers (top row)
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
    KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

	// functions
	KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
	KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,

	// controls
	KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_SPACE,

	// modifiers
	KEY_SHIFT, 
    KEY_CTRL, 
    KEY_ALT, 
    KEY_SUPER,

	// arrows
	KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,

	// navigations
	KEY_INSERT, KEY_DELETE,
    KEY_HOME, KEY_END,
    KEY_PAGE_UP, KEY_PAGE_DOWN,

	// numpad
	KEY_NUM_LOCK,
    KEY_KP_0, KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4,
    KEY_KP_5, KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9,
    KEY_KP_ADD,
    KEY_KP_SUBTRACT,
    KEY_KP_MULTIPLY,
    KEY_KP_DIVIDE,
    KEY_KP_ENTER,
    KEY_KP_DECIMAL,

	// symbols
	KEY_MINUS, 
    KEY_EQUALS, 
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_BACKSLASH,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,

    KEY_CAPS_LOCK,

	KEY_COUNT
} keys;
// clang-format on

typedef enum {
    MOD_SHIFT = 1 << 0,
    MOD_CTRL = 1 << 1,
    MOD_ALT = 1 << 2,
    MOD_SUPER = 1 << 3,
    MOD_CAPS = 1 << 4,
} key_mod;

// engine state
void input_sys_init(void);
void input_sys_kill(void);
void input_sys_update(f64 delta);
const char *input_sys_keyname(keys k);

// platform layer
void input_process_key(keys key, b8 pressed);
void input_process_mods(u8 mods);
void input_process_mouse_button(mouse_buttons button, b8 pressed);
void input_process_mouse_move(i16 x, i16 y);
void input_process_mouse_wheel(i8 z_delta);

// input logic
AM2_API b8 input_keydown(keys key);
AM2_API b8 input_keyup(keys key);
AM2_API b8 input_was_keydown(keys key);
AM2_API b8 input_was_keyup(keys key);

AM2_API b8 input_mouse_button_down(mouse_buttons button);
AM2_API b8 input_mouse_button_up(mouse_buttons button);
AM2_API b8 input_mouse_was_button_down(mouse_buttons button);
AM2_API b8 input_mouse_was_button_up(mouse_buttons button);
AM2_API void input_get_mouse_pos(i32 *x, i32 *y);
AM2_API void input_get_mouse_prev_pos(i32 *x, i32 *y);

#endif // INPUT_H
