#ifndef REQUEST_H
#define REQUEST_H

#include <stdbool.h>

#include "common.h"

typedef struct
{
    HttpMethod Method;
    const char* Path;
} HttpRequest;

bool request_parse(const char* buf, int size, HttpRequest* req);
void request_destroy(const HttpRequest* req);

#endif //REQUEST_H
