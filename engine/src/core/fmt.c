#include "fmt.h"
#include <unistd.h>
#include <stddef.h>

typedef enum {
    LEN_NONE = 0, // no length modifier
    LEN_HH,       // char (hh)
    LEN_H,        // short (h)
    LEN_L,        // long (l)
    LEN_LL,       // long long (ll)
    LEN_J,        // intmax_t (j) - optional
    LEN_Z,        // size_t (z)
    LEN_T,        // ptrdiff_t (t)
    LEN_L_CAP,    // long double (L) - for future floats
} len_mods_t;

typedef struct {
    i32 fd;       // file descriptor output (-1 for buffer)
    char *buffer; // buffer output
    u64 size;     // buffer size
    u64 pos;      // current pos in buffer
    i32 error;    // error flag (overflow, etc.)
} ctx_t;

static len_mods_t parse_len_mods(const char **fmt)
{
    const char *p = *fmt;
    len_mods_t len = LEN_NONE;

    switch (*p)
    {
    case 'h':
        p++;
        if (*p == 'h')
        {
            p++;
            len = LEN_HH;
        }
        else
        {
            len = LEN_H;
        }
        break;

    case 'l':
        p++;
        if (*p == 'l')
        {
            p++;
            len = LEN_LL;
        }
        else
        {
            len = LEN_L;
        }
        break;

        // clang-format off
    case 'j': p++; len = LEN_J; break;
    case 'z': p++; len = LEN_Z; break;
    case 't': p++; len = LEN_T; break;
    case 'L': p++; len = LEN_L_CAP; break;

    default: break;
        // clang-format on
    }

    *fmt = p;
    return len;
}

static i32 num_to_str(char *buf, u64 n, i32 base, const char *d)
{
    i32 i = 0;

    if (n == 0)
        buf[i++] = '0';
    else
    {
        while (n > 0)
        {
            buf[i++] = d[n % (u64)base];
            n /= (u64)base;
        }
    }

    return i;
}

static i32 out_char(ctx_t *ctx, char c)
{
    if (ctx->fd >= 0)
    {
        write(1, &c, 1);
        return 1;
    }
    else if (ctx->buffer)
    {
        if (ctx->pos < ctx->size)
        {
            ctx->buffer[ctx->pos++] = c;
            return 1;
        }
        ctx->error = 1; // overflow
        return 0;
    }
    return 0;
}

static i32 out_str(ctx_t *ctx, const char *s)
{
    i32 count = 0;
    while (*s) count += out_char(ctx, *s++);
    return count;
}

static i32 out_buf(ctx_t *ctx, const char *buf, i32 len)
{
    i32 count = 0;
    for (i32 i = 0; i < len; i++) count += out_char(ctx, buf[i]);
    return count;
}

static i32 hdl_signed(ctx_t *ctx, long v, i32 base, const char *d)
{
    char buf[32];
    i32 i = 0;
    i32 count = 0;
    u64 n;

    if (v < 0)
    {
        count += out_char(ctx, '-');
        n = (u64)(-(v + 1)) + 1;
    }
    else
    {
        n = (u64)v;
    }

    i = num_to_str(buf, n, base, d);
    while (i--) out_char(ctx, buf[i]);
    return count;
}

static i32 hdl_unsigned(ctx_t *ctx, u64 v, i32 base, const char *d)
{
    char buf[32];
    i32 i = num_to_str(buf, v, base, d);
    i32 count = 0;

    while (i--) count += out_char(ctx, buf[i]);
    return count;
}

static i32 hdl_ptr(ctx_t *ctx, void *ptr)
{
    i32 count = 0;
    if (!ptr)
    {
        count += out_str(ctx, "<null>");
        return count;
    }

    uptr addr = (uptr)ptr;
    char buf[2 + sizeof(uptr) * 2];
    i32 i = num_to_str(buf, addr, 16, "0123456789abcdef");

    count += out_str(ctx, "0x");
    while (i--) count += out_char(ctx, buf[i]);

    return count;
}

static i32 hdl_binary(ctx_t *ctx, u32 v)
{
    i32 count = out_str(ctx, "0b");
    if (v == 0) return count + out_char(ctx, '0');

    char buf[40];
    i32 i = 0;
    i32 bit_count = 0;

    while (v)
    {
        if (bit_count > 0 && bit_count % 4 == 0) buf[i++] = '_';
        buf[i++] = '0' + (v & 1);
        v >>= 1;
        bit_count++;
    }

    while (i--) count += out_char(ctx, buf[i]);
    return count;
}

static i32 hdl_float(ctx_t *ctx, f64 v, i32 prec)
{
    char buf[64];
    i32 i = 0;
    i32 is_negative = 0;
    i32 count = 0;

    if (prec == -1) prec = 4;

    if (v < 0)
    {
        is_negative = 1;
        v = -v;
    }

    long int_part = (long)v;
    f64 frac = v - (f64)int_part;

    // Calculate rounding factor based on precision
    f64 rounder = 5;
    for (int j = 0; j <= prec; ++j) rounder /= 10.0;
    frac += rounder;

    if (frac >= 1.0)
    {
        int_part++;
        frac -= 1.0;
    }

    // Integer part
    if (int_part == 0)
    {
        buf[i++] = '0';
    }
    else
    {
        char rev[32];
        i32 rev_i = 0;
        while (int_part > 0)
        {
            rev[rev_i++] = '0' + (int_part % 10);
            int_part /= 10;
        }
        while (rev_i--) buf[i++] = rev[rev_i];
    }

    if (prec > 0) buf[i++] = '.';

    // 4 decimal places
    for (int j = 0; j < prec; ++j)
    {
        frac *= 10;
        i32 digit = (i32)frac;
        buf[i++] = '0' + (char)digit;
        frac -= digit;
    }

    if (is_negative) count += out_char(ctx, '-');
    count += out_buf(ctx, buf, i);
    return count;
}

static i32 hdl_signed_spec(ctx_t *ctx, len_mods_t length, va_list args)
{
    const char *d = "0123456789";
    i32 base = 10;

    switch (length)
    {
    case LEN_NONE: return hdl_signed(ctx, va_arg(args, int), base, d);

    // char
    case LEN_HH:
        return hdl_signed(ctx, (signed char)va_arg(args, int), base, d);

    // short
    case LEN_H: return hdl_signed(ctx, (short)va_arg(args, int), base, d);

    // long
    case LEN_L: return hdl_signed(ctx, va_arg(args, long), base, d);

    // long long
    case LEN_LL: return hdl_signed(ctx, va_arg(args, long long), base, d);

    // size_t
    case LEN_Z: return hdl_signed(ctx, va_arg(args, ssize_t), base, d);

    // ptrdiff_t
    case LEN_T: return hdl_signed(ctx, va_arg(args, ptrdiff_t), base, d);

    // intmax_t
    case LEN_J: return hdl_signed(ctx, va_arg(args, long), base, d);

    default: return 0;
    }
}

static i32 hdl_unsigned_spec(ctx_t *ctx, len_mods_t length, va_list args,
                             i32 base, const char *d)
{
    switch (length)
    {
    case LEN_NONE: return hdl_unsigned(ctx, va_arg(args, u32), base, d);

    // unsigned char
    case LEN_HH:
        return hdl_unsigned(ctx, (unsigned char)va_arg(args, u32), base, d);
    // unsigned short
    case LEN_H:
        return hdl_unsigned(ctx, (unsigned short)va_arg(args, u32), base, d);

    // unsigned long
    case LEN_L: return hdl_unsigned(ctx, va_arg(args, unsigned long), base, d);

    // unsigned long long
    case LEN_LL: return hdl_unsigned(ctx, va_arg(args, u64), base, d);

    // size_t
    case LEN_Z: return hdl_unsigned(ctx, va_arg(args, u64), base, d);

    // uintptr_t
    case LEN_T: return hdl_unsigned(ctx, va_arg(args, uptr), base, d);

    // uintmax_t
    case LEN_J: return hdl_unsigned(ctx, va_arg(args, u64), base, d);

    default: return 0;
    }
}

static i32 hdl_specifier(ctx_t *ctx, char specifier, len_mods_t length,
                         i32 prec, va_list args)
{
    switch (specifier)
    {
    case 'd':
    case 'i': return hdl_signed_spec(ctx, length, args);
    case 'u': return hdl_unsigned_spec(ctx, length, args, 10, "0123456789");

    case 'x':
        return hdl_unsigned_spec(ctx, length, args, 16, "0123456789abcdef");
    case 'X':
        return hdl_unsigned_spec(ctx, length, args, 16, "0123456789ABCDEF");

    case 'b':
        if (length == LEN_NONE) return hdl_binary(ctx, va_arg(args, u32));
        break;

    case 'p':
        if (length == LEN_NONE) return hdl_ptr(ctx, va_arg(args, void *));
        break;

    case 'f':
    case 'F':
        if (length == LEN_L_CAP) va_arg(args, long double);
        return hdl_float(ctx, va_arg(args, f64), prec);

    case 's':
        if (length == LEN_NONE)
        {
            const char *str = va_arg(args, const char *);
            if (!str) str = "<null>";
            return out_str(ctx, str);
        }
        break;

    case 'c':
        if (length == LEN_NONE)
        {
            char c = (char)va_arg(args, i32);
            return out_char(ctx, c);
        }
        break;

    case '%': return out_char(ctx, '%');

    default: return -1;
    }

    return -1;
}

static i32 set_print(ctx_t *ctx, const char *fmt, va_list args)
{
    i32 count = 0;

    for (const char *p = fmt; *p; ++p)
    {
        if (*p != '%')
        {
            count += out_char(ctx, *p);
            continue;
        }

        p++;
        if (!*p) break;

        len_mods_t len = parse_len_mods(&p);
        if (!*p) break;

        // Set precision point
        i32 precision = -1;
        if (*p == '.')
        {
            p++;
            precision = 0;
            while (*p >= '0' && *p <= '9')
            {
                precision = precision * 10 + (*p - '0');
                p++;
            }
            if (!*p) break;
        }

        i32 handled = hdl_specifier(ctx, *p, len, precision, args);

        if (handled >= 0)
        {
            count += handled;
        }
        else
        {
            count += out_char(ctx, '%');
            count += out_char(ctx, *p);
        }
    }

    return count;
}

i32 pfmt(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    ctx_t ctx = {.fd = 1}; // stdout is fd 1
    i32 count = set_print(&ctx, fmt, args);

    va_end(args);
    return count;
}

i32 fpfmt(i32 fd, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    ctx_t ctx = {.fd = fd};
    i32 count = set_print(&ctx, fmt, args);

    va_end(args);
    return count;
}

i32 snpfmt(char *str, u64 size, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    ctx_t ctx = {.fd = -1, .buffer = str, .size = size, .pos = 0, .error = 0};

    i32 count = set_print(&ctx, fmt, args);

    // Null terminate
    if (ctx.pos < size)
        str[ctx.pos] = '\0';
    else if (size > 0)
        str[size - 1] = '\0';

    va_end(args);
    return count;
}

i32 vsnpfmt(char *str, u64 size, const char *fmt, va_list args)
{
    ctx_t ctx = {.fd = -1, .buffer = str, .size = size, .pos = 0, .error = 0};
    i32 count = set_print(&ctx, fmt, args);

    // Null terminate
    if (ctx.pos < size)
        str[ctx.pos] = '\0';
    else if (size > 0)
        str[size - 1] = '\0';

    return count;
}

