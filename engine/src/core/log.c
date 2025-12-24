#include "define.h"

#include <stdio.h>
#include <stdarg.h>

#define MSG_BUFFER 4096
#define FINAL_BUFFER 4096

#if PLATFORM_LINUX
#    define WHITE "38;5;15m"
#    define RED "38;5;196m"
#    define ORANGE "38;5;208m"
#    define YELLOW "38;5;220m"
#    define GREEN "38;5;040m"
#    define CYAN "38;5;050m"
#    define MAGENTA "38;5;219m"
#endif

static const char *lvl_str[6] = {"[FATAL]", "[ERROR]", "[WARN]",
                                 "[INFO]",  "[DEBUG]", "[TRACE]"};

static void log_console(const char *msg, u8 color)
{
    const char *color_string[] = {
        RED,    // FATAL
        ORANGE, // ERROR
        YELLOW, // WARN
        WHITE,  // INFO
        CYAN,   // DEBUG
        MAGENTA // TRACE
    };

    printf("\033[%s%s\033[0m", color_string[color], msg);
}

void log_msg(log_level_t level, const char *msg, ...)
{
    char msg_buffer[MSG_BUFFER];
    char final_buffer[FINAL_BUFFER];

    va_list p_arg;
    va_start(p_arg, msg);
    vsnprintf(msg_buffer, sizeof(msg_buffer), msg, p_arg);
    va_end(p_arg);

    snprintf(final_buffer, sizeof(final_buffer), "%s %s\n", lvl_str[level],
             msg_buffer);

    log_console(final_buffer, (u8)level);
}
