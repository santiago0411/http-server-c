#include "response.h"

#include <stdio.h>

void response_set_status(HttpResponse* res, const uint16_t status)
{
    res->StatusCode = status;
    switch (status) {
        case 200:
            res->StatusDesc = "OK";
            break;
        case 400:
            res->StatusDesc = "Bad Request";
            break;
        case 404:
            res->StatusDesc = "Not Found";
            break;
        default:
            res->StatusDesc = "OK";
            fprintf(stderr, "Unsupported status code %d\n", status);
            break;
    }
}

const char* response_to_str(const HttpResponse* res, size_t* size)
{
    StringBuilder sb = sb_create(20);

    sb_append_str(&sb, "HTTP/1.1 ");
    sb_append_u16(&sb, res->StatusCode);
    sb_append_char(&sb, ' ');
    sb_append_str(&sb, res->StatusDesc);
    sb_append_str(&sb, HTTP_NEW_LINE);

    for (size_t i = 0; i < res->Headers.Count; i++) {
        const Header* h = &res->Headers.Data[i];
        sb_append_str(&sb, h->Header);
        sb_append_str(&sb, ": ");
        sb_append_str(&sb, h->Value);
        sb_append_str(&sb, HTTP_NEW_LINE);
    }

    if (res->Content) {
        sb_append_str(&sb, HTTP_NEW_LINE);
        sb_append_str(&sb, res->Content);
        sb_append_str(&sb, HTTP_NEW_LINE);
    }

    // End
    sb_append_str(&sb, HTTP_NEW_LINE);

    *size = sb.Count;
    return sb.Data;
}

void response_destroy(HttpResponse* res)
{
    free_headers(&res->Headers);
    free((void*)res->Content);
}
