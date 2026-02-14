#include <stdio.h>
#include <cjson/cJSON.h>
#include "json_utils.h"
#include <string.h>
#include <stdlib.h>

char *get_content_from_json(const char *json_str)
{
    if (!json_str)
        return NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root)
        return NULL;

    char *result = NULL;

    cJSON *message = cJSON_GetObjectItem(root, "message");
    if (cJSON_IsObject(message))
    {
        cJSON *content = cJSON_GetObjectItem(message, "content");
        if (cJSON_IsString(content))
        {
            // aloca memória suficiente e copia o conteúdo
            result = malloc(strlen(content->valuestring) + 1);
            if (result)
                strcpy(result, content->valuestring);
        }
    }

    cJSON_Delete(root);
    return result; // quem chamar precisa fazer free(result) depois
}

size_t stream_json_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t total = size * nmemb;

    cJSON *root = cJSON_Parse((char *)contents);
    if (!root)
        return total;

    cJSON *message = cJSON_GetObjectItem(root, "message");
    if (cJSON_IsObject(message))
    {
        cJSON *content = cJSON_GetObjectItem(message, "content");
        if (cJSON_IsString(content))
        {
            printf("%s", content->valuestring);
            fflush(stdout);
        }
    }

    cJSON_Delete(root);
    return total;
}
