#ifndef ERRO_H
#define ERRO_H
#include <stddef.h>
void erro(const char *categoria, int linha, const char *mensagem);
void *alocar(size_t tamanho);
char *copiar_texto(const char *texto);
#endif
