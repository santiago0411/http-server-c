#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool request_parse(const char* buf, const int size, HttpRequest* req)
{
    const int method_str_size = first_index_of(buf, size, ' ');
    if (method_str_size == 0) {
        fprintf(stderr, "Not enough bytes to parse Method from Request!\n");
        return false;
    }

    if (strncmp(buf, "GET", method_str_size) == 0) {
        req->Method = GET;
    } else if (strncmp(buf, "POST", method_str_size) == 0) {
        req->Method = POST;
    } else {
        fprintf(stderr, "Unknown request method %.*s", method_str_size, buf);
        return false;
    }

    // We know there is a space after the method str so it's safe to do + 1
    buf += method_str_size + 1;

    const int path_str_size = first_index_of(buf, size, ' ');
    if (path_str_size == 0) {
        fprintf(stderr, "Not enough bytes to parse Path from Request!\n");
        return false;
    }

    req->Path = calloc(path_str_size + 1, sizeof(char));
    memcpy((void*)req->Path, buf, path_str_size);

    // Only parsing method and path for now
    return true;
}

void request_destroy(const HttpRequest* req)
{
    free((void*)req->Path);
}
