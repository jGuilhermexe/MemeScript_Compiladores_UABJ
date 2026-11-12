#include "erro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void erro(const char *categoria, int linha, const char *mensagem) {
    fprintf(stderr, "Erro %s na linha %d: %s\n", categoria, linha, mensagem);
    exit(1);
}

void *alocar(size_t tamanho) {
    void *p = calloc(1, tamanho);
    if (!p) { perror("Memória insuficiente"); exit(1); }
    return p;
}

char *copiar_texto(const char *texto) {
    char *copia = alocar(strlen(texto) + 1);
    return strcpy(copia, texto);
}
