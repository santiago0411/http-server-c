#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    char* Data;
    size_t Size;
    size_t Capacity;
} StringBuilder;

StringBuilder sb_create(size_t capacity);
void sb_destroy(StringBuilder* sb);

void sb_append_str(StringBuilder* sb, const char* str);
void sb_append_char(StringBuilder* sb, char c);
void sb_append_i8(StringBuilder* sb, int8_t val);
void sb_append_u8(StringBuilder* sb, uint8_t val);
void sb_append_i16(StringBuilder* sb, int16_t val);
void sb_append_u16(StringBuilder* sb, uint16_t val);
void sb_append_i32(StringBuilder* sb, int32_t val);
void sb_append_u32(StringBuilder* sb, uint32_t val);
void sb_append_i64(StringBuilder* sb, int64_t val);
void sb_append_u64(StringBuilder* sb, uint64_t val);

#endif //STRING_BUILDER_H
