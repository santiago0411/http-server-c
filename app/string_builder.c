#include "string_builder.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

#define APPEND(sb, str, size) \
    memcpy((sb)->Data + (sb)->Size, str, size); \
    (sb)->Size += size; \

void ensure_capacity(StringBuilder* sb, const size_t size)
{
    if (sb->Size + size <= sb->Capacity) {
        return;
    }

    const size_t new_chuck_size = sb->Capacity == 0 ? INITIAL_CAPACITY : sb->Capacity * 2;
    const size_t new_size = new_chuck_size >= size ? new_chuck_size : sb->Capacity + size;
    sb->Data = realloc(sb->Data, new_size);
    assert(sb->Data && "Out of RAM lol");
    sb->Capacity = new_size;
}

StringBuilder sb_create(const size_t capacity)
{
    const StringBuilder sb = {
        .Data = malloc(capacity),
        .Size = 0,
        .Capacity = 10
    };
    return sb;
}

void sb_destroy(StringBuilder *sb)
{
    free(sb->Data);
    sb->Size = 0;
    sb->Capacity = 0;
}

void sb_append_str(StringBuilder *sb, const char *str)
{
    const size_t size = strlen(str);
    ensure_capacity(sb, size);
    APPEND(sb, str, size);
}

void sb_append_char(StringBuilder *sb, char c)
{
    ensure_capacity(sb, 1);
    const char buf[1] = { c };
    APPEND(sb, buf, 1);
}

void sb_append_integer(StringBuilder* sb, const char* format, const uint64_t val)
{
    char buf[21]; // 21 is the longest unsigned 64 bit int
    const int size = snprintf(buf, sizeof(buf), format, val);
    ensure_capacity(sb, size);
    APPEND(sb, buf, size);
}

void sb_append_i8(StringBuilder *sb, const int8_t val)
{
    sb_append_integer(sb, "%d", val);
}

void sb_append_u8(StringBuilder *sb, const uint8_t val)
{
    sb_append_integer(sb, "%u", val);
}

void sb_append_i16(StringBuilder *sb, const int16_t val)
{
    sb_append_integer(sb, "%d", val);
}

void sb_append_u16(StringBuilder *sb, const uint16_t val)
{
    sb_append_integer(sb, "%u", val);
}

void sb_append_i32(StringBuilder *sb, const int32_t val)
{
    sb_append_integer(sb, "%d", val);
}

void sb_append_u32(StringBuilder *sb, const uint32_t val)
{
    sb_append_integer(sb, "%u", val);
}

void sb_append_i64(StringBuilder *sb, const int64_t val)
{
    sb_append_integer(sb, "%lld", val);
}

void sb_append_u64(StringBuilder *sb, const uint64_t val)
{
    sb_append_integer(sb, "%llu", val);
}