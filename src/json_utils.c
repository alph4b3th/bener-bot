#include <stdio.h>
#include <cjson/cJSON.h>
#include "json_utils.h"
size_t stream_json_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;

    cJSON *root = cJSON_Parse((char *)contents);
    if (!root) return total;

    cJSON *message = cJSON_GetObjectItem(root, "message");
    if (cJSON_IsObject(message)) {
        cJSON *content = cJSON_GetObjectItem(message, "content");
        if (cJSON_IsString(content)) {
            printf("%s", content->valuestring);
            fflush(stdout);
        }
    }

    cJSON_Delete(root);
    return total;
}
