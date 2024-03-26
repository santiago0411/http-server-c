#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_INIT_CAP 2

#define ARRAY_ENSURE_CAPACITY(arr) \
    if ((arr)->Count >= (arr)->Capacity) { \
        (arr)->Capacity = (arr)->Capacity == 0 ? ARRAY_INIT_CAP : (arr)->Capacity * 2; \
        (arr)->Items = realloc((arr)->Items, (arr)->Capacity * sizeof(*(arr)->Items)); \
        assert((arr)->Items && "Out of RAM"); \
    } \

#define ARRAY_APPEND(arr, item) \
    do { \
        ARRAY_ENSURE_CAPACITY((arr)) \
        (arr)->Items[(arr)->Count++] = (item); \
    } while (0)

#define ARRAY_FREE(arr) \
    do { \
        free((void*)(arr)->Items); \
        (arr)->Items = NULL; \
        (arr)->Count = 0; \
        (arr)->Capacity = 0; \
    } while (0)

#define HTTP_NEW_LINE "\r\n"

typedef enum
{
    GET,
    POST,
} HttpMethod;

typedef struct
{
    const char* Header;
    const char* Value;
} Header;

typedef struct
{
    Header* Items;
    size_t Count;
    size_t Capacity;
} HeadersArray;

int first_index_of(const char* buf, int size, char delim);
void set_header_str(HeadersArray* headers, const char* header, const char* value);
void set_header_i32(HeadersArray* headers, const char* header, int32_t value);

#endif //COMMON_H