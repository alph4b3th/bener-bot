#include <stdio.h>
#include "ollama_client.h"
#include <cjson/cJSON.h>

int main() {
    const char *host = "http://localhost:11434";
    const char *model = "gemma3:1b";
    const char *prompt = "Explique C vs Rust em uma frase.";


    printf("Gerando...\n\n");

    cJSON *messages = cJSON_CreateArray();
   
    cJSON *msg1 = cJSON_CreateObject();
    cJSON_AddStringToObject(msg1, "role", "system");
    cJSON_AddStringToObject(msg1, "content", "Você é um assistente útil chamado Jarvis.");
    cJSON_AddItemToArray(messages, msg1);


    cJSON *msg2 = cJSON_CreateObject();
    cJSON_AddStringToObject(msg2, "role", "user");
    cJSON_AddStringToObject(msg2, "content", "Qual o seu nome?");
    cJSON_AddItemToArray(messages, msg2);

    char *msg_json = cJSON_PrintUnformatted(messages);

    printf("%s\n",msg_json);

    if (generate_text(host, model, msg_json) != 0) {
        printf("Erro na geração.\n");
        return 1;
    }

    printf("\n\nConcluído.\n");
    return 0;
}
