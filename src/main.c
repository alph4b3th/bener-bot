#include <stdio.h>
#include "ollama_client.h"
#include <ollama/ollama.h>
#include "json_utils.h"
#include <cjson/cJSON.h>
#include "memory_graph.h"
#include "tools.h"
#include <stdlib.h>

void print_splash()
{
    printf("========================================\n");
    printf("           BENDER-BOT-JARVIS            \n");
    printf("========================================\n");
    printf("Digite algo para conversar com o Jarvis.\n");
    printf("Para sair, digite 'exit' e pressione Enter.\n\n");
}

void fn_weather_fake(void **args)
{
    // args[0] seria cidade, args[1] seria dia, etc.
    const char *city = (const char *)args[0];
    printf("clima %s: Sol, 25°C\n", city);
}

/*
gemma3:270m                                   e7d36fb2c3b3    291 MB    4 weeks ago      
smollm2:135m                                  9077fe9d2ae1    270 MB    6 weeks ago      
qwen3:14b                                     bdbd181c33f2    9.3 GB    6 months ago     
deepseek-r1:latest                            6995872bfe4c    5.2 GB    7 months ago     
llama3.1:latest                               46e0c10c039e    4.9 GB    7 months ago     
llama3.2:latest                               a80c4f17acd5    2.0 GB    7 months ago     
hermes3:3b                                    a8851c5041d4    2.0 GB    7 months ago     
hermes3:latest                                4f6b83f30b62    4.7 GB    7 months ago     
gemma3:latest                                 a2af6cc3eb7f    3.3 GB    7 months ago     
qwen3:30b-a3b                                 0b28110b7a33    18 GB     7 months ago     
qwen3:4b                                      2bfd38a7daaf    2.6 GB    7 months ago     
phi4-mini-reasoning:latest                    3ca8c2865ce9    3.2 GB    7 months ago     
smollm2:latest                                cef4a1e09247    1.8 GB    7 months ago     
gemma3n:e4b                                   15cb39fd9394    7.5 GB    7 months ago     
gemma3n:e2b                                   719372f8c7de    5.6 GB    7 months ago     
phi3:mini                                     4f2222927938    2.2 GB    7 months ago     
minicpm-v:latest                              c92bfad01205    5.5 GB    8 months ago     
llama3.1:8b-instruct-q4_K_M                   46e0c10c039e    4.9 GB    9 months ago     
qwen2.5:latest                                845dbda0ea48    4.7 GB    10 months ago    
deepseek-r1:8b                                28f8fd6cdc67    4.9 GB    10 months ago    
*/

void register_weather_tool()
{
    const char *param_names[] = {"city"};
    const char *param_types[] = {"string"};

    register_tool(
        "get_weather",
        "Retorna a previsão do tempo para uma cidade (mock/fake).",
        1,
        param_names,
        param_types,
        fn_weather_fake);
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
   
    const char *model = "llama3.2:latest";
    const char *prompt = "Explique C vs Rust em uma frase.";

    int running = 1;
    char buffer[300];

    register_weather_tool();

    cJSON *short_memory = cJSON_CreateArray();

    char *system_prompt_template = build_tools_system_prompt();
    cJSON *system_prompt = cJSON_CreateObject();
    cJSON_AddStringToObject(system_prompt, "role", "system");
    cJSON_AddStringToObject(system_prompt, "content", system_prompt_template);
    cJSON_AddItemToArray(short_memory, system_prompt);

    printf("SYSTEM: %s", system_prompt_template);
    // free(system_prompt);

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

        if (strcmp(buffer, "!exit") == 0)
        {
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

        if (cJSON_Parse(response)) // só executa se for JSON válido
        {
            call_tool_from_json(response);
        }

        cJSON *response_message = cJSON_CreateObject();
        cJSON_AddStringToObject(response_message, "role", "system");
        cJSON_AddStringToObject(response_message, "content", response);
        cJSON_AddItemToArray(short_memory, response_message);

        printf("Jarvis: %s\n", response);
    }

    char *short_memory_json = cJSON_PrintUnformatted(short_memory);
    printf("Short Memory: [%s]\n", short_memory_json);

    cJSON_Delete(short_memory);

    return 0;
}
