#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_INIT_CAP 2

#define ARRAY_ENSURE_CAPACITY(arr, required) \
    if ((arr)->Count + (required) > (arr)->Capacity) { \
        if ((arr)->Capacity == 0) { \
            (arr)->Capacity = ARRAY_INIT_CAP >= required ? ARRAY_INIT_CAP : required; \
        } else { \
            (arr)->Capacity = (arr)->Capacity * 2 >= (arr)->Count + (required) ? (arr)->Capacity * 2 : (arr)->Count + (required); \
        } \
        (arr)->Data = realloc((arr)->Data, (arr)->Capacity * sizeof(*(arr)->Data)); \
        assert((arr)->Data && "Out of ram lol"); \
    } \

#define ARRAY_INIT(arr, capacity) \
    (arr)->Data = malloc(capacity * sizeof(*(arr)->Data)); \
    assert((arr)->Data && "Out of ram lol"); \
    (arr)->Capacity = capacity; \

#define ARRAY_APPEND(arr, item) \
    do { \
        assert(sizeof(*(arr)->Data) == sizeof(item) && "Trying to append data of a different size");  \
        ARRAY_ENSURE_CAPACITY((arr), 1) \
        (arr)->Data[(arr)->Count++] = (item); \
    } while (0)

#define ARRAY_APPEND_MANY(arr, items, count) \
    do { \
        assert(sizeof(*(arr)->Data) == sizeof(*items) && "Trying to append data of a different size");  \
        ARRAY_ENSURE_CAPACITY((arr), count) \
        memcpy((arr)->Data + (arr)->Count, items, sizeof(*(arr)->Data) * count); \
        (arr)->Count += count; \
    } while(0)

#define ARRAY_FREE(arr) \
    do { \
        free((void*)(arr)->Data); \
        (arr)->Data = NULL; \
        (arr)->Count = 0; \
        (arr)->Capacity = 0; \
    } while (0)

#define HTTP_NEW_LINE "\r\n"

typedef enum
{
    METHOD_GET,
    METHOD_HEAD,
    METHOD_POST,
} HttpMethod;

typedef struct
{
    const char* Header;
    const char* Value;
} Header;

typedef struct
{
    Header* Data;
    size_t Count;
    size_t Capacity;
} HeadersArray;

typedef struct
{
    char* Data;
    size_t Count;
    size_t Capacity;
} Buffer;

// Utils
int first_index_of(const char* buf, int size, char delim);
Buffer read_file_to_end(const char* path);

const char* get_header_value(const HeadersArray* headers, const char* header);
Header create_header_str(const char* header, const char* value);
Header create_header_str_len(const char* header, size_t header_len, const char* value, size_t value_len);
Header create_header_i32(const char* header, int32_t value);
void free_headers(HeadersArray* headers);
#endif //COMMON_H