#include <stdio.h>
#include <cjson/cJSON.h>
#include "json_utils.h"

size_t stream_json_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;

    cJSON *root = cJSON_Parse((char *)contents);
    if (!root)
        return total;

    cJSON *response = cJSON_GetObjectItem(root, "response");
    if (cJSON_IsString(response))
        printf("%s", response->valuestring);
        fflush(stdout);

    cJSON_Delete(root);
    return total;
}
