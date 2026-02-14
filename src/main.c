#include <stdio.h>
#include "ollama_client.h"
#include <ollama/ollama.h>
#include "json_utils.h"
#include <cjson/cJSON.h>

int main()
{
    const char *host = "http://localhost:11434";
    const char *model = "gemma3:1b";
    const char *prompt = "Explique C vs Rust em uma frase.";

    printf("Gerando...\n\n");

    cJSON *short_memory = cJSON_CreateArray();

    cJSON *msg1 = cJSON_CreateObject();
    cJSON_AddStringToObject(msg1, "role", "system");
    cJSON_AddStringToObject(msg1, "content", "Você é um assistente útil chamado Jarvis.");
    cJSON_AddItemToArray(short_memory, msg1);

    cJSON *msg2 = cJSON_CreateObject();
    cJSON_AddStringToObject(msg2, "role", "user");
    cJSON_AddStringToObject(msg2, "content", "Qual o seu nome?");
    cJSON_AddItemToArray(short_memory, msg2);

    char *short_memory_json = cJSON_PrintUnformatted(short_memory);

    printf("Short Memory: [%s]\n", short_memory_json);

    ollama_result_t result;
    if (generate_text(host, model, short_memory_json, &result) != 0)
    {
        printf("Erro na geração.\n");
        return 1;
    }

    printf("result:\n\t%s", get_content_from_json(result.buf.base));
    printf("\n\nConcluído.\n");

    cJSON_Delete(short_memory);

    return 0;
}
