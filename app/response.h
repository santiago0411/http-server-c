#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdint.h>

#include "common.h"
#include "string_builder.h"

struct HttpResponse_T;
typedef struct HttpResponse_T* HttpResponse;

void response_create(HttpResponse* res);
void response_destroy(HttpResponse res);
void response_set_status(HttpResponse res, uint16_t status);
void response_set_content(HttpResponse res, const char* content, size_t size);
void response_set_header(HttpResponse res, Header header);
const char* response_to_str(HttpResponse res, size_t* size);


#endif //RESPONSE_H
