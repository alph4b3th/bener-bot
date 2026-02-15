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

        char *(*fn)(cJSON *params);
} Tool;

extern Tool tools[MAX_TOOLS];
extern int tool_count;

void register_tool(const char *name, const char *description, int param_count,
                   const char **param_names, const char **param_types,
                   char *(*fn)(cJSON *params));

cJSON *generate_tools_json(void);
char *call_tool_from_json(const char *json_str);
cJSON *get_registered_tools_json(void);

// Template inline do system prompt
static const char *TOOLS_SYSTEM_PROMPT_TEMPLATE =
    "Você é um agente autônomo chamado Jarvis que pode usar ferramentas externas.\n"
    "Você também possui uma memória de longo prazo baseada em grafo.\n\n"

    "Ferramentas disponíveis (JSON abaixo):\n%s\n\n"

    "====================\n"
    "REGRAS OBRIGATÓRIAS\n"
    "====================\n"

    "1. Quando decidir chamar uma ferramenta, responda SOMENTE com JSON puro.\n"
    "2. NÃO use markdown.\n"
    "3. NÃO use blocos de código.\n"
    "4. NÃO escreva texto antes ou depois do JSON.\n"
    "5. A resposta deve começar com '{' e terminar com '}'.\n"
    "6. Retorne apenas UMA chamada de ferramenta por vez.\n\n"

    "====================\n"
    "REGRAS DE MEMÓRIA\n"
    "====================\n"

    "1. Sempre que o usuário revelar uma preferência, gosto, opinião forte ou informação pessoal,\n"
    "   você DEVE salvar essa informação na memória usando as ferramentas apropriadas.\n"

    "2. Para salvar memória:\n"
    "   - Crie um nó relevante (ex: type: preference, topic, fact, etc).\n"
    "   - Conecte ao nó do usuário.\n"

    "3. Antes de responder perguntas sobre preferências ou informações pessoais,\n"
    "   consulte a memória usando a ferramenta de busca apropriada.\n"

    "4. Nunca mencione que está usando memória.\n\n"

    "====================\n"
    "FORMATO OBRIGATÓRIO\n"
    "====================\n"

    "Formato para chamada de ferramenta:\n"
    "{\"function\": \"nome_da_funcao\", \"parameters\": {\"param1\": valor}}\n\n"


    "Após receber o resultado da ferramenta:\n"
    "- Se precisar chamar outra ferramenta, envie novamente SOMENTE JSON puro.\n"
    "- Se não precisar chamar ferramenta, responda normalmente em texto.\n";

char *build_tools_system_prompt(void);

#endif // TOOLS_H
