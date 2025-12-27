#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "core/define.h"

#define DA_DEFAULT_CAPACITY 1
#define DA_FACTOR_RESIZE 2

enum { CAPACITY, LENGTH, STRIDE, FIELD_LENGTH };

AM2_API void *_arr_create(u64 length, u64 stride);

AM2_API void _arr_destroy(void *arr);

AM2_API u64 _arr_get(void *arr, u64 field);

AM2_API void _arr_set(void *arr, u64 field, u64 value);

AM2_API void *_arr_resize(void *arr);

AM2_API void *_arr_push(void *arr, const void *ptr_value);

AM2_API void _arr_pop(void *arr, void *dest);

AM2_API void *_arr_pop_at(void *arr, u64 index, void *dest);

AM2_API void *_arr_insert_at(void *arr, u64 index, void *ptr_value);

#define da_create(type) _arr_create(DA_DEFAULT_CAPACITY, sizeof(type))

#define da_reserve(type, capacity) _arr_create(capacity, sizeof(type))

#define da_destroy(array) _arr_destroy(array)

#define da_push(array, value)                                                 \
    do                                                                        \
    {                                                                         \
        __typeof__(value) temp = (value);                                     \
        (array) = _arr_push((array), &temp);                                  \
    }                                                                         \
    while (0)

#define da_pop(array, ptr_value) _arr_pop(array, ptr_value)

#define da_insert_at(array, index, value)                                     \
    do                                                                        \
    {                                                                         \
        __typeof__(value) temp = (value);                                     \
        (array) = _arr_insert_at((array), (index), &temp);                    \
    }                                                                         \
    while (0)

#define da_pop_at(array, index, ptr_value) _arr_pop_at(array, index, ptr_value)

#define da_clear(array) _arr_set(array, LENGTH, 0)

#define da_capacity(array) _arr_get(array, CAPACITY)

#define da_length(array) _arr_get(array, LENGTH)

#define da_stride(array) _arr_get(array, STRIDE)

#define da_length_set(array, value) _arr_set(array, LENGTH, value)

#endif // DYNAMIC_ARRAY_H
