#include "tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Tool tools[MAX_TOOLS];
int tool_count = 0;

void register_tool(const char *name, const char *description, int param_count,
                   const char **param_names, const char **param_types, void (*fn)(void **))
{
    Tool *t = &tools[tool_count++];
    t->name = name;
    t->description = description;
    t->param_count = param_count;
    t->fn = fn;
    for (int i = 0; i < param_count; i++)
    {
        t->param_names[i] = param_names[i];
        t->param_types[i] = param_types[i];
    }
}

cJSON *generate_tools_json(void)
{
    cJSON *arr = cJSON_CreateArray();
    for (int i = 0; i < tool_count; i++)
    {
        cJSON *obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "name", tools[i].name);
        cJSON_AddStringToObject(obj, "description", tools[i].description);
        cJSON *params = cJSON_CreateArray();
        for (int j = 0; j < tools[i].param_count; j++)
        {
            cJSON *p = cJSON_CreateObject();
            cJSON_AddStringToObject(p, "name", tools[i].param_names[j]);
            cJSON_AddStringToObject(p, "type", tools[i].param_types[j]);
            cJSON_AddItemToArray(params, p);
        }
        cJSON_AddItemToObject(obj, "parameters", params);
        cJSON_AddItemToArray(arr, obj);
    }
    return arr;
}

void call_tool_from_json(const char *json_str)
{   

printf("[DEBUG] JSON recebido para executar ferramenta: %s\n", json_str);
    cJSON *root = cJSON_Parse(json_str);
    const char *fname = cJSON_GetObjectItem(root, "function")->valuestring;
    cJSON *params = cJSON_GetObjectItem(root, "parameters");

    for (int i = 0; i < tool_count; i++)
    {
        if (strcmp(tools[i].name, fname) == 0)
        {
            void *args[10];
            for (int j = 0; j < tools[i].param_count; j++)
            {
                const char *pname = tools[i].param_names[j];
                cJSON *pval = cJSON_GetObjectItem(params, pname);
                if (strcmp(tools[i].param_types[j], "int") == 0)
                {
                    int *v = malloc(sizeof(int));
                    *v = pval->valueint;
                    args[j] = v;
                }
                else if (strcmp(tools[i].param_types[j], "float") == 0)
                {
                    float *v = malloc(sizeof(float));
                    *v = (float)pval->valuedouble;
                    args[j] = v;
                }
                else // string
                {
                    args[j] = pval->valuestring;
                }
            }

            tools[i].fn(args);

            for (int j = 0; j < tools[i].param_count; j++)
            {
                if (strcmp(tools[i].param_types[j], "int") == 0 || strcmp(tools[i].param_types[j], "float") == 0)
                    free(args[j]);
            }
            break;
        }
    }
    cJSON_Delete(root);
}

cJSON *get_registered_tools_json(void)
{
    return generate_tools_json();
}

char *build_tools_system_prompt(void)
{
    cJSON *tools_json = get_registered_tools_json();
    char *tools_str = cJSON_PrintUnformatted(tools_json);

    size_t len = snprintf(NULL, 0, TOOLS_SYSTEM_PROMPT_TEMPLATE, tools_str) + 1;
    char *prompt = malloc(len);
    snprintf(prompt, len, TOOLS_SYSTEM_PROMPT_TEMPLATE, tools_str);

    cJSON_Delete(tools_json);
    free(tools_str);

    return prompt;
}
