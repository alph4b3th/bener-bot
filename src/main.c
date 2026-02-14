#include <stdio.h>
#include "ollama_client.h"
#include <ollama/ollama.h>
#include "json_utils.h"
#include <cjson/cJSON.h>
#include "memory_graph.h"


void print_splash() {
     printf("========================================\n");
    printf("           BENDER-BOT-JARVIS            \n");
    printf("========================================\n");
    printf("Digite algo para conversar com o Jarvis.\n");
    printf("Para sair, digite 'exit' e pressione Enter.\n\n");
}
int main()
{
    Graph graph;
    graph.nodes = NULL;
    graph.node_count = 0;

    Node *user_node = memory_create_node(1, "user", "Alice");
    Node *preference_node = memory_create_node(2, "preference", "Gosta de café");
    Node *topic_node = memory_create_node(3, "topic", "Física Quântica");

    memory_add_node(&graph, user_node);
    memory_add_node(&graph, preference_node);
    memory_add_node(&graph, topic_node);

    memory_add_edge(user_node, preference_node, 0.9f);
    memory_add_edge(user_node, topic_node, 0.7f);

    memory_update_edge_confidence_by_id(user_node, 3, 0.2f);

    Node *search_node = memory_find_node_from_id(&graph, 1);
    if (search_node != NULL)
    {
        printf("Encontrado nó: ID=%d, Type=%s, Property=%s\n", search_node->id, search_node->type, search_node->property);
    }

    memory_show(&graph);

    const char *host = "http://localhost:11434";
    const char *model = "gemma3:1b";
    const char *prompt = "Explique C vs Rust em uma frase.";

    int running = 1;
    char buffer[300];

    cJSON *short_memory = cJSON_CreateArray();

    cJSON *system_prompt = cJSON_CreateObject();
    cJSON_AddStringToObject(system_prompt, "role", "system");
    cJSON_AddStringToObject(system_prompt, "content", "Você é um assistente útil chamado Jarvis.");
    cJSON_AddItemToArray(short_memory, system_prompt);

    ollama_result_t result;
    print_splash();
    while (running)
    {
        printf("input:");
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        {
            continue;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "!exit") == 0) {
            break;
        }

        cJSON *input_message = cJSON_CreateObject();
        cJSON_AddStringToObject(input_message, "role", "user");
        cJSON_AddStringToObject(input_message, "content", buffer);
        cJSON_AddItemToArray(short_memory, input_message);

        char *short_memory_json = cJSON_PrintUnformatted(short_memory);

        if (generate_text(host, model, short_memory_json, &result) != 0)
        {
            printf("Erro na geração. \n");
            return 1;
        }

        char *response = get_content_from_json(result.buf.base);

        cJSON *response_message = cJSON_CreateObject();
        cJSON_AddStringToObject(response_message, "role", "system");
        cJSON_AddStringToObject(response_message, "content", response);
        cJSON_AddItemToArray(short_memory, response_message);

        printf("Jarvis: %s\n", response);
    }

    char *short_memory_json =cJSON_PrintUnformatted(short_memory);
    printf("Short Memory: [%s]\n", short_memory_json);

    cJSON_Delete(short_memory);

    return 0;
}
