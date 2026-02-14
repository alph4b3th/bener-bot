#ifndef TOOLS_H
#define TOOLS_H

#include <cjson/cJSON.h>

#define MAX_TOOLS 100
#define MAX_PARAMS 10

typedef struct
{
    const char *name;
    const char *description;
    const char *param_names[MAX_PARAMS];
    const char *param_types[MAX_PARAMS];
    int param_count;
    char *(*fn)(void **args);
} Tool;

extern Tool tools[MAX_TOOLS];
extern int tool_count;

void register_tool(const char *name, const char *description, int param_count,
                   const char **param_names, const char **param_types,
                   char *(*fn)(void **args));

cJSON *generate_tools_json(void);
char *call_tool_from_json(const char *json_str);
cJSON *get_registered_tools_json(void);

// Template inline do system prompt
static const char *TOOLS_SYSTEM_PROMPT_TEMPLATE =
    "Você é um agente autônomo chamado Jarvis que pode usar ferramentas externas.\n"
    "Ferramentas disponíveis (JSON abaixo):\n%s\n\n"
    "Quando quiser usar uma ferramenta, gere SOMENTE JSON no formato:\n"
    "{\"function\": \"nome_da_funcao\", \"parameters\": {\"param1\": valor, ...}}\n"
    "Exemplo para obter clima:\n"
    "{\"function\": \"get_weather\", \"parameters\": {\"city\": \"São Paulo\"}}\n"
    "Observação: envie somente o json, porque o seu interpretador de ferramentas nao consegue identificar se você mandar mensagem e json juntos\n"
    "Resposta da tool:\n"
    "\tQuando a tool retornar com o resultado, será algo como:\n"
    "{\"function\": \"get_weather\", \"result\": {\"temperatura\": \"21\", \"status\": \"Ensolarado\" }}\n";

char *build_tools_system_prompt(void);

#endif // TOOLS_H
