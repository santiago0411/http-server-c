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

void set_header_string(HeadersArray* headers, const char *header, const char *value, const bool copy_value)
{
    if (!copy_value) {
        ARRAY_APPEND(headers, ((Header){ .Header = header, .Value = value }));
        return;
    }

    const size_t len = strlen(value);
    const Header h = {
        .Header = header,
        .Value = malloc(len + 1)
    };
    assert(h.Value && "Out of ram");
    memcpy((char*)h.Value, value, len + 1);
    ARRAY_APPEND(headers, h);
}

void set_header_str(HeadersArray* headers, const char *header, const char *value)
{
    set_header_string(headers, header, value, true);
}

void set_header_integer(HeadersArray *headers, const char *header, const uint64_t value, const char* format)
{
    char buf[21]; // 21 is the longest unsigned 64 bit int
    const int size = snprintf(buf, sizeof(buf), format, value);
    char* value_str = malloc(size + 1);
    assert(value_str && "Out of ram lol");
    memcpy(value_str, buf, size);
    value_str[size] = '\0';
    set_header_string(headers, header, value_str, false);
}

void set_header_i32(HeadersArray *headers, const char *header, const int32_t value)
{
    set_header_integer(headers, header, value, "%d");
}


