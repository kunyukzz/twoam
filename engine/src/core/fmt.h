#ifndef FMT_H
#define FMT_H

#include "define.h"
#include <stdarg.h>

i32 pfmt(const char *fmt, ...);

i32 fpfmt(i32 fd, const char *fmt, ...);

i32 snpfmt(char *str, u64 size, const char *fmt, ...);

i32 vsnpfmt(char *str, u64 size, const char *fmt, va_list args);

#endif // FMT_H
