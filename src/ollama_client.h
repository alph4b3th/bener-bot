#ifndef OLLAMA_CLIENT_H
#define OLLAMA_CLIENT_H
#include <ollama/ollama.h>
int generate_text(const char *host, const char *model, const char *prompt, ollama_result_t *result);

#endif
