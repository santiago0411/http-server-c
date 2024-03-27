#ifndef REQUEST_H
#define REQUEST_H

#include <stdbool.h>

#include "common.h"

typedef struct
{
    HttpMethod Method;
    const char* Path;
    const HeadersArray Headers;
    const char* Body;
} HttpRequest;

bool request_parse(const Buffer* in, HttpRequest* req);
void request_destroy(const HttpRequest* req);

#endif //REQUEST_H
