#ifndef DEFINE_H
#define DEFINE_H

/**********************************
 * Platform Detection (force 64-bit)
 * ********************************/
#if defined(__linux__) || defined(__gnu_linux__)
#    define PLATFORM_LINUX 1
#    ifndef _POSIX_C_SOURCE
#        define _POSIX_C_SOURCE 200809L
#    endif
#else
#    error "2AM Engine requires 64-bit platform"
#endif

/**********************************
 * Build Config
 * ********************************/
#if defined(AM2_DEBUG)
#    define AM2_DEBUG_ENABLED 1
#else
#    define AM2_DEBUG_ENABLED 0
#endif

#ifdef AM2_CORE
#    define AM2_API __attribute__((visibility("default")))
#else
#    define AM2_API
#endif

/**********************************
 * Compiler
 * ********************************/
#if defined(__clang__) || defined(__GNUC__)
#    define INL __attribute__((always_inline)) inline
#    define NOINL __attribute__((noinline))
#    define ALIGN(n) __attribute__((aligned(n)))
#endif

#if defined(__clang__) || defined(__GNUC__)
#    define STATIC_ASSERT(cond, msg)                                          \
        typedef char static_assertion_##msg[(cond) ? 1 : -1]
#else
#    define STATIC_ASSERT static_assert
#endif

/**********************************
 * Type Definiton
 * ********************************/
#define true 1
#define false 0

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;
typedef float f32;
typedef double f64;
typedef _Bool b8;
typedef u64 uptr;

#define KIBIBYTE (1024ULL)
#define MEBIBYTE (1024ULL * KIBIBYTE)
#define GIBIBYTE (1024ULL * MEBIBYTE)
#define TEBIBYTE (1024ULL * GIBIBYTE)

STATIC_ASSERT(sizeof(uptr) == 8, uptr_must_be_64bit);
STATIC_ASSERT(sizeof(void *) == 8, pointers_must_be_64bit);

/**********************************
 * Logging
 * ********************************/
typedef enum {
    LOG_FATAL, // Fatal errors (will crash)
    LOG_ERROR, // Errors that don't crash
    LOG_WARN,  // Warnings
    LOG_INFO,  // General information
    LOG_DEBUG, // For development debugging
    LOG_TRACE, // For deep debugging
} log_level_t;

AM2_API void log_msg(log_level_t level, const char *fmt, ...);

#define LOGF(...) log_msg(LOG_FATAL, __VA_ARGS__)
#define LOGE(...) log_msg(LOG_ERROR, __VA_ARGS__)

#define LOGW(...) log_msg(LOG_WARN, __VA_ARGS__)
#define LOGI(...) log_msg(LOG_INFO, __VA_ARGS__)
#define LOGD(...) log_msg(LOG_DEBUG, __VA_ARGS__)
#define LOGT(...) log_msg(LOG_TRACE, __VA_ARGS__)

#if AM2_DEBUG_ENABLED
#    define AM2_ASSERT(cond)                                                  \
        do                                                                    \
        {                                                                     \
            if (!(cond))                                                      \
            {                                                                 \
                log_msg(LOG_FATAL, "ASSERT FAILED: %s\n  at %s:%d", #cond,    \
                        __FILE__, __LINE__);                                  \
                __builtin_trap();                                             \
            }                                                                 \
        }                                                                     \
        while (0)
#else
#    define AM2_ASSERT(cond) ((void)0)
#endif

#endif // DEFINE_H
