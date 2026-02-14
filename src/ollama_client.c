#include <stdio.h>
#include <ollama/ollama.h>
#include "json_utils.h"
#include "ollama_client.h"

int generate_text(const char *host, const char *model, const char *prompt, ollama_result_t *result)
{
    ollama_t *client = ollama_init((char *)host, NULL, 0);
    if (!client)
        return 1;

    result->type = OLLAMA_RESULT_TYPE_MEM_DYNAMIC;

    char *param_json = "{\"stream\": false}";
    int status = ollama_chat(client, (char *)model, (char *)prompt, param_json, result);

    if (status != 0)
    {
        printf("falha.\n");
        ollama_close(client);
        return -1;
    }

    ollama_close(client);
    return status;
}
