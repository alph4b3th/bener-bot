#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <stddef.h>

size_t stream_json_callback(void *contents, size_t size, size_t nmemb, void *userp);
char *get_content_from_json(const char *json_str);
#endif
