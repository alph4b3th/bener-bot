#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ollama_client.h"
#include <ollama/ollama.h>
#include "json_utils.h"
#include <cjson/cJSON.h>
#include "memory_graph.h"
#include "tools.h"

#define MAX_TOOL_CALLS 10

Graph GLOBAL_GRAPH = {0};
static int NEXT_NODE_ID = 1;

/* ========================================================= */
/* =================== UTILIDADES ========================== */
/* ========================================================= */

void load_env(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
    return;

  char line[512];
  while (fgets(line, sizeof(line), file))
  {
    char *key = strtok(line, "=");
    char *value = strtok(NULL, "\n");
    if (key && value)
      setenv(key, value, 1);
  }
  fclose(file);
}

void print_splash()
{
  printf("========================================\n");
  printf("           BENDER-BOT-JARVIS            \n");
  printf("========================================\n");
  printf("Digite algo para conversar.\n");
  printf("!exit para sair | !graph para ver grafo\n\n");
}

int get_int_param(cJSON *params, const char *key)
{
  cJSON *item = cJSON_GetObjectItem(params, key);
  if (!item)
    return 0;

  if (cJSON_IsNumber(item))
    return item->valueint;

  if (cJSON_IsString(item))
    return atoi(item->valuestring);

  return 0;
}

float get_float_param(cJSON *params, const char *key)
{
  cJSON *item = cJSON_GetObjectItem(params, key);
  if (!item)
    return 0.0f;

  if (cJSON_IsNumber(item))
    return (float)item->valuedouble;

  if (cJSON_IsString(item))
    return atof(item->valuestring);

  return 0.0f;
}

char *get_string_param(cJSON *params, const char *key)
{
  cJSON *item = cJSON_GetObjectItem(params, key);
  if (!item)
    return NULL;

  if (cJSON_IsString(item))
    return item->valuestring;

  return NULL;
}


/* ========================================================= */
/* ==================== TOOLS ============================== */
/* ========================================================= */

char *tool_create_node(cJSON *params)
{
  char *type = get_string_param(params, "type");
  char *property = get_string_param(params, "property");

  if (!type || !property)
    return strdup("Erro: parâmetros inválidos para create_node.");

  int id = NEXT_NODE_ID++;

  Node *node = memory_create_node(id, type, property);
  memory_add_node(&GLOBAL_GRAPH, node);

  char buffer[256];
  snprintf(buffer, sizeof(buffer),
           "Node criado: id=%d, type=%s, property=%s",
           id, type, property);

  return strdup(buffer);
}

void register_create_node_tool()
{
  const char *param_names[] = {"type", "property"};
  const char *param_types[] = {"string", "string"};

  register_tool(
      "create_node",
      "Cria um nó na memória",
      2,
      param_names,
      param_types,
      tool_create_node);
}

/* --------------------------------------------------------- */

char *tool_add_edge(cJSON *params)
{
  int from_id = get_int_param(params, "from_id");
  int to_id = get_int_param(params, "to_id");
  float confidence = get_float_param(params, "confidence");

  if (from_id <= 0)
    return strdup("Erro: from_id inválido.");

  if (to_id <= 0)
    return strdup("Erro: to_id inválido.");

  Node *from = memory_find_node_from_id(&GLOBAL_GRAPH, from_id);
  Node *to = memory_find_node_from_id(&GLOBAL_GRAPH, to_id);

  if (!from)
  {
    char buffer[128];
    snprintf(buffer, sizeof(buffer),
             "Erro: nó from_id=%d não encontrado.", from_id);
    return strdup(buffer);
  }

  if (!to)
  {
    char buffer[128];
    snprintf(buffer, sizeof(buffer),
             "Erro: nó to_id=%d não encontrado.", to_id);
    return strdup(buffer);
  }

  memory_add_edge(from, to, confidence);

  return strdup("Aresta criada com sucesso.");
}

void register_add_edge_tool()
{
  const char *param_names[] = {"from_id", "to_id", "confidence"};
  const char *param_types[] = {"int", "int", "float"};

  register_tool(
      "add_edge",
      "Cria relação entre dois nós",
      3,
      param_names,
      param_types,
      tool_add_edge);
}


/* ========================================================= */
/* ================= AGENT LOOP ============================ */
/* ========================================================= */

char *run_agent_cycle(const char *host,
                      const char *model,
                      cJSON *short_memory)
{
  ollama_result_t result;
  char *short_memory_json = NULL;
  char *response = NULL;
  int tool_calls = 0;

  while (tool_calls < MAX_TOOL_CALLS)
  {
    free(short_memory_json);
    short_memory_json = cJSON_PrintUnformatted(short_memory);

    if (generate_text(host, model, short_memory_json, &result) != 0)
      return NULL;

    response = get_content_from_json(result.buf.base);
    if (!response)
      return NULL;

    cJSON *parsed = cJSON_Parse(response);
    if (!parsed)
      return response;

    cJSON *fn = cJSON_GetObjectItem(parsed, "function");
    cJSON *params = cJSON_GetObjectItem(parsed, "parameters");

    if (!fn || !params)
    {
      cJSON_Delete(parsed);
      return response;
    }

    char *tool_output = call_tool_from_json(response);
    cJSON_Delete(parsed);
    free(response);

    if (!tool_output)
      return NULL;

    printf("[TOOL RESULT] %s\n", tool_output);

    cJSON *tool_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(tool_msg, "role", "tool");
    cJSON_AddStringToObject(tool_msg, "content", tool_output);
    cJSON_AddItemToArray(short_memory, tool_msg);

    free(tool_output);
    tool_calls++;
  }

  return strdup("Erro: limite de chamadas de ferramenta excedido.");
}

/* ========================================================= */
/* ======================= MAIN ============================ */
/* ========================================================= */

int main()
{
  load_env(".env");

  GLOBAL_GRAPH.nodes = NULL;
  GLOBAL_GRAPH.node_count = 0;

  const char *host = getenv("OLLAMA_HOST");
  const char *model = getenv("OLLAMA_MODEL");

  if (!host || !model)
  {
    printf("Erro: configure OLLAMA_HOST e OLLAMA_MODEL\n");
    return -1;
  }

  register_create_node_tool();
  register_add_edge_tool();

  cJSON *short_memory = cJSON_CreateArray();

  char *system_prompt_template = build_tools_system_prompt();

  cJSON *system_prompt = cJSON_CreateObject();
  cJSON_AddStringToObject(system_prompt, "role", "system");
  cJSON_AddStringToObject(system_prompt, "content", system_prompt_template);
  cJSON_AddItemToArray(short_memory, system_prompt);

  print_splash();

  char buffer[512];

  while (1)
  {
    printf("input: ");
    if (!fgets(buffer, sizeof(buffer), stdin))
      continue;

    buffer[strcspn(buffer, "\n")] = 0;

    if (strcmp(buffer, "!exit") == 0)
      break;

    if (strcmp(buffer, "!graph") == 0)
    {
      memory_show(&GLOBAL_GRAPH);
      continue;
    }

    cJSON *msg = cJSON_CreateObject();
    cJSON_AddStringToObject(msg, "role", "user");
    cJSON_AddStringToObject(msg, "content", buffer);
    cJSON_AddItemToArray(short_memory, msg);

    char *response = run_agent_cycle(host, model, short_memory);

    if (!response)
    {
      printf("Erro no agente.\n");
      continue;
    }

    cJSON *assistant_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(assistant_msg, "role", "assistant");
    cJSON_AddStringToObject(assistant_msg, "content", response);
    cJSON_AddItemToArray(short_memory, assistant_msg);

    printf("Jarvis: %s\n", response);
    free(response);
  }

  cJSON_Delete(short_memory);
  return 0;
}
