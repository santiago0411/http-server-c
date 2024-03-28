#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int first_index_of(const char* buf, const int size, const char delim)
{
    char* next_space = memchr(buf, delim, size);
    if (!next_space) {
        return 0;
    }
    return next_space - buf;
}

Buffer read_file_to_end(const char* path)
{
    Buffer buf = {0};
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return buf;
    }

    printf("Reading file '%s'\n", path);

    fseek(file, 0, SEEK_END);
    const size_t size = ftell(file);
    rewind(file);

    ARRAY_INIT(&buf, size);

    if (fread(buf.Data, sizeof(char), buf.Capacity, file) != size) {
        fprintf(stderr, "Failed to read full file to memory. '%s'\n", path);
        fclose(file);
        return buf;
    }

    buf.Count += size;
    fclose(file);
    return buf;
}

bool write_file(const char* path, const Buffer* buf)
{
    FILE* file = fopen(path, "wb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return false;
    }

    printf("Writing file '%s'\n", path);

    if (fwrite(buf->Data, sizeof(char), buf->Count, file) != buf->Count) {
        fprintf(stderr, "Failed to write full file to disk. '%s'\n", path);
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
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

Header create_header_string(const char *header, const size_t header_len,
                       const char *value, const size_t value_len, const bool copy_value)
{
    Header h = { .Header = calloc(header_len + 1, 1) };
    assert(h.Header && "Out of ram lol");
    memcpy((char*)h.Header, header, header_len);

    if (!copy_value) {
        // This value was allocated by set_header_integer so no need to copy it
        h.Value = value;
        return h;
    }

    h.Value = calloc(value_len + 1, 1);
    assert(h.Value && "Out of ram lol");
    memcpy((char*)h.Value, value, value_len);
    return h;
}

Header create_header_str(const char *header, const char *value)
{
    return create_header_string(header, strlen(header), value, strlen(value), true);
}

Header create_header_str_len(const char* header, const size_t header_len,
    const char* value, const size_t value_len)
{
    return create_header_string(header, header_len, value, value_len, true);
}

Header create_header_integer(const char *header, const uint64_t value, const char* format)
{
    char buf[21]; // 21 is the longest unsigned 64 bit int
    const int size = snprintf(buf, sizeof(buf), format, value);
    char* value_str = malloc(size + 1);
    assert(value_str && "Out of ram lol");
    memcpy(value_str, buf, size);
    value_str[size] = '\0';
    return create_header_string(header, strlen(header), value_str, size, false);
}

Header create_header_i32(const char *header, const int32_t value)
{
    return create_header_integer(header, value, "%d");
}

void free_headers(HeadersArray *headers)
{
    for (size_t i = 0; i < headers->Count; i++) {
        free((void*)headers->Data[i].Header);
        free((void*)headers->Data[i].Value);
    }
    ARRAY_FREE(headers);
}


