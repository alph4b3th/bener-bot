#include <stdio.h>
#include "ollama_client.h"

int main() {
    const char *host = "http://localhost:11434";
    const char *model = "gemma3:1b";
    const char *prompt = "Explique C vs Rust em uma frase.";


    printf("Gerando...\n\n");

    if (generate_text(host, model, prompt) != 0) {
        printf("Erro na geração.\n");
        return 1;
    }

    printf("\n\nConcluído.\n");
    return 0;
}
