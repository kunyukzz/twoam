#include "darray.h"
#include "core/memory.h"

void *_arr_create(u64 length, u64 stride)
{
    u64 header_size = FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;

    // allocate new memory block as bytes
    u8 *block = mem_alloc(header_size + array_size, MEM_ARRAY);
    AM2_ASSERT(block);

    mem_set(block, 0, header_size + array_size);

    // interpret the first part of the block as a u64 header
    u64 *head = (u64 *)block;
    head[CAPACITY] = length;
    head[LENGTH] = 0;
    head[STRIDE] = stride;

    // return pointer to start of element data
    return block + header_size;
}

void _arr_destroy(void *arr)
{
    if (!arr) return;

    // go backwards by the header size (in bytes) to reach the header.
    u64 *header = (u64 *)((u8 *)arr - FIELD_LENGTH * sizeof(u64));

    AM2_ASSERT(header[STRIDE] != 0);
    AM2_ASSERT(header[CAPACITY] < (1ULL << 32));
    AM2_ASSERT(header[LENGTH] <= header[CAPACITY]);

    const u64 total =
        FIELD_LENGTH * sizeof(u64) + header[CAPACITY] * header[STRIDE];

    mem_free(header, total, MEM_ARRAY);
}

u64 _arr_get(void *arr, u64 field)
{
    u64 *header = (u64 *)((u8 *)arr - FIELD_LENGTH * sizeof(u64));
    return header[field];
}

void _arr_set(void *arr, u64 field, u64 value)
{
    u64 *header = (u64 *)((u8 *)arr - FIELD_LENGTH * sizeof(u64));
    header[field] = value;
}

void *_arr_resize(void *arr)
{
    AM2_ASSERT(arr);

    u64 len = da_length(arr);
    u64 stride = da_stride(arr);
    u64 cap = da_capacity(arr);

    AM2_ASSERT(stride != 0);
    AM2_ASSERT(len <= cap);

    u64 new_cap = cap ? cap * DA_FACTOR_RESIZE : DA_DEFAULT_CAPACITY;
    AM2_ASSERT(new_cap >= len);

    void *temp = _arr_create(new_cap, stride);
    mem_copy(temp, arr, len * stride);

    _arr_set(temp, LENGTH, len);
    _arr_destroy(arr);

    return temp;
}

void *_arr_push(void *arr, const void *ptr_value)
{
    AM2_ASSERT(arr);
    AM2_ASSERT(ptr_value);

    u64 len = da_length(arr);
    u64 stride = da_stride(arr);

    if (len >= da_capacity(arr))
    {
        arr = _arr_resize(arr);
    }

    mem_copy((u8 *)arr + len * stride, ptr_value, stride);
    _arr_set(arr, LENGTH, len + 1);

    return arr;
}

void _arr_pop(void *arr, void *dest)
{
    AM2_ASSERT(arr);

    u64 len = da_length(arr);
    if (len == 0) return;

    u64 stride = da_stride(arr);

    if (dest)
    {
        mem_copy(dest, (u8 *)arr + (len - 1) * stride, stride);
    }

    _arr_set(arr, LENGTH, len - 1);
}

void *_arr_pop_at(void *arr, u64 index, void *dest)
{
    AM2_ASSERT(arr);

    u64 len = da_length(arr);
    u64 stride = da_stride(arr);

    if (index >= len)
    {
        return arr;
    }

    u8 *base = (u8 *)arr;
    if (dest)
    {
        mem_copy(dest, base + index * stride, stride);
    }

    // If we're not removing the last element, we need to shift all elements
    // after `index` one slot to the left.
    //
    // Source and destination memory regions OVERLAP here:
    //
    //   [ index+1 ][ index+2 ][ index+3 ]
    //        ↓         ↓         ↓
    //   [  index ][ index+1 ][ index+2 ]
    //
    // Using memcpy() on overlapping regions is undefined behavior.
    // memmove() is REQUIRED because it handles overlap safely
    if (index < len - 1)
    {
        mem_move(base + index * stride, base + (index + 1) * stride,
                 stride * (len - index - 1));
    }

    _arr_set(arr, LENGTH, len - 1);
    return arr;
}

void *_arr_insert_at(void *arr, u64 index, void *ptr_value)
{
    AM2_ASSERT(arr);
    AM2_ASSERT(ptr_value);

    u64 len = da_length(arr);
    u64 stride = da_stride(arr);

    if (index > len)
    {
        return arr;
    }
    if (len >= da_capacity(arr))
    {
        arr = _arr_resize(arr);
    }

    u8 *base = (u8 *)arr;

    // If inserting NOT at the end, we must shift existing elements
    // one slot to the right to make room.
    //
    // Source and destination memory regions OVERLAP:
    //
    //   [ index ][ index+1 ][ index+2 ]
    //        ↓         ↓         ↓
    //   [ index+1 ][ index+2 ][ index+3 ]
    //
    // memcpy() would corrupt data here due to overlap.
    // memmove() is REQUIRED to preserve correctness.
    if (index < len)
    {
        mem_move(base + (index + 1) * stride, base + index * stride,
                 stride * (len - index));
    }

    mem_copy(base + index * stride, ptr_value, stride);
    _arr_set(arr, LENGTH, len + 1);

    return arr;
}
