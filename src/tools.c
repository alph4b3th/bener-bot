#include "tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Tool tools[MAX_TOOLS];
int tool_count = 0;

/* ========================================================= */
/* ================= REGISTER =============================== */
/* ========================================================= */

void register_tool(const char *name,
                   const char *description,
                   int param_count,
                   const char **param_names,
                   const char **param_types,
                   char *(*fn)(cJSON *params))
{
    if (tool_count >= MAX_TOOLS)
        return;

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

/* ========================================================= */
/* ================= TOOL JSON EXPORT ====================== */
/* ========================================================= */

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

/* ========================================================= */
/* ================= TOOL DISPATCH ========================= */
/* ========================================================= */

char *call_tool_from_json(const char *json_str)
{
    printf("\033[38;5;214m[DEBUG] JSON recebido para executar ferramenta: %s\033[0m\n", json_str);

    cJSON *root = cJSON_Parse(json_str);
    if (!root)
        return NULL;

    cJSON *fn_item = cJSON_GetObjectItem(root, "function");
    cJSON *params = cJSON_GetObjectItem(root, "parameters");

    if (!cJSON_IsString(fn_item) || !cJSON_IsObject(params))
    {
        cJSON_Delete(root);
        return NULL;
    }

 
    char fname_copy[128];
    strncpy(fname_copy, fn_item->valuestring, sizeof(fname_copy) - 1);
    fname_copy[sizeof(fname_copy) - 1] = '\0';

    char *tool_result = NULL;

    for (int i = 0; i < tool_count; i++)
    {
        if (strcmp(tools[i].name, fname_copy) == 0)
        {
            tool_result = tools[i].fn(params);
            break;
        }
    }

    cJSON_Delete(root);

    if (!tool_result)
        return NULL;

    /* ===================================================== */
    /* ============ WRAP RESULT EM JSON ==================== */
    /* ===================================================== */

    cJSON *json_result = cJSON_CreateObject();
    cJSON_AddStringToObject(json_result, "function", fname_copy);

    cJSON *result_obj = cJSON_CreateObject();

    cJSON *parsed_output = cJSON_Parse(tool_result);
    if (parsed_output)
    {
        cJSON_AddItemToObject(result_obj, "data", parsed_output);
    }
    else
    {
        cJSON_AddStringToObject(result_obj, "output", tool_result);
    }

    cJSON_AddItemToObject(json_result, "result", result_obj);

    char *json_str_result = cJSON_PrintUnformatted(json_result);

    cJSON_Delete(json_result);
    free(tool_result);

    return json_str_result;
}

/* ========================================================= */
/* ================= SYSTEM PROMPT ========================= */
/* ========================================================= */

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
