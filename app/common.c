#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int first_index_of(const char* buf, const int size, const char delim)
{
    char* next_space = memchr(buf, delim, size);
    if (!next_space) {
        return 0;
    }
    return next_space - buf;
}

const char* get_header_value(const HeadersArray* headers, const char* header)
{
    for (size_t i = 0; i < headers->Count; i++) {
        if (strcmp(headers->Data[i].Header, header) == 0) {
            return headers->Data[i].Value;
        }
    }
    return NULL;
}

void set_header_string(HeadersArray* headers, const char *header, const size_t header_len,
                       const char *value, const size_t value_len, const bool copy_value)
{
    Header h = { .Header = calloc(header_len + 1, 1) };
    assert(h.Header && "Out of ram lol");
    memcpy((char*)h.Header, header, header_len);

    if (!copy_value) {
        // This value was allocated by set_header_integer so no need to copy it
        h.Value = value;
        ARRAY_APPEND(headers, h);
        return;
    }

    h.Value = calloc(value_len + 1, 1);
    assert(h.Value && "Out of ram lol");
    memcpy((char*)h.Value, value, value_len);
    ARRAY_APPEND(headers, h);
}

void set_header_str(HeadersArray* headers, const char *header, const char *value)
{
    set_header_string(headers, header, strlen(header), value, strlen(value), true);
}

void set_header_str_len(HeadersArray* headers, const char* header, const size_t header_len,
    const char* value, const size_t value_len)
{
    set_header_string(headers, header, header_len, value, value_len, true);
}

void set_header_integer(HeadersArray *headers, const char *header, const uint64_t value, const char* format)
{
    char buf[21]; // 21 is the longest unsigned 64 bit int
    const int size = snprintf(buf, sizeof(buf), format, value);
    char* value_str = malloc(size + 1);
    assert(value_str && "Out of ram lol");
    memcpy(value_str, buf, size);
    value_str[size] = '\0';
    set_header_string(headers, header, strlen(header), value_str, size, false);
}

void set_header_i32(HeadersArray *headers, const char *header, const int32_t value)
{
    set_header_integer(headers, header, value, "%d");
}

void free_headers(HeadersArray *headers)
{
    for (size_t i = 0; i < headers->Count; i++) {
        free((void*)headers->Data[i].Header);
        free((void*)headers->Data[i].Value);
    }
    ARRAY_FREE(headers);
}


