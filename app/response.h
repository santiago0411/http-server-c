#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdint.h>

#include "common.h"
#include "string_builder.h"

typedef struct
{
    // const char* HttpVersion; Not used for now it's always HTTP/1.1
    uint16_t StatusCode;
    const char* StatusDesc;
    HeadersArray Headers;
    const char* Content;
} HttpResponse;

void response_set_status(HttpResponse* res, uint16_t status);
const char* response_to_str(const HttpResponse* res, size_t* size);
void response_destroy(HttpResponse* res);

#endif //RESPONSE_H
