#include <stdio.h>
#include <ollama/ollama.h>
#include "json_utils.h"
#include "ollama_client.h"

int generate_text(const char *host, const char *model, const char *prompt) {
    ollama_t *client = ollama_init(host, NULL, 0);
    if (!client)
        return 1;

    ollama_result_t result;
    result.buf.type = OLLAMA_RESULT_TYPE_CALLBACK;
    result.call.callback = stream_json_callback;
    result.call.callback_handle = NULL;

    int status = ollama_generate(client, model, prompt, NULL, &result);

    ollama_close(client);
    return status;
}
