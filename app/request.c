#include "request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool request_parse(const Buffer* in, HttpRequest* req)
{
    char* buf = in->Data;
    int size = in->Count;

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
    size -= method_str_size + 1;

    const int path_str_size = first_index_of(buf, size, ' ');
    if (path_str_size == 0) {
        fprintf(stderr, "Not enough bytes to parse Path from Request!\n");
        return false;
    }

    req->Path = calloc(path_str_size + 1, sizeof(char));
    memcpy((void*)req->Path, buf, path_str_size);

    int new_line;
    while ((new_line = first_index_of(buf, size, '\n')) > 0) {
        buf += new_line + 1;
        size -= new_line + 1;
        if (size <= 0) {
            break;
        }

        const int header_end = first_index_of(buf, size, ':');
        if (header_end == 0 || size <= header_end) {
            break;
        }
        const char* header = buf;
        const int header_len = header_end;

        int value_start = buf[header_end + 1] == ' ' ? header_end + 1 : header_end;
        buf += value_start + 1;
        size -= value_start + 1;
        if (size <= 0) {
            break;
        }

        const char* value = buf;
        const int value_len = first_index_of(buf, size, '\r');
        if (value_len == 0) {
            break;
        }

        set_header_str_len((HeadersArray*)&req->Headers, header, header_len, value, value_len);
    }

    // Advance \r\n corresponding to either line between headers and body or request end
    buf += 2;
    size -= 2;

    if (size > 0) {
        // TODO validate if after the body also comes \r\n\r\n
        req->Body = calloc(size, 1);
        memcpy((char*)req->Body, buf, size);

    }

    return true;
}

void request_destroy(const HttpRequest* req)
{
    free((void*)req->Path);
    free_headers((void*)&req->Headers);
    free((void*)req->Body);
}
