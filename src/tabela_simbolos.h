#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H
#include <stddef.h>

typedef struct No No;

typedef enum { INTEIRO_MS, REAL_MS, TEXTO_MS, BOOLEANO_MS, NULO_MS, LISTA_MS } TipoValor;

typedef struct Valor {
    TipoValor tipo;
    long long inteiro;
    double real;
    char *texto;
    struct Valor *itens;
    size_t tamanho;
} Valor;

Valor copiar_valor(Valor valor);
void liberar_valor(Valor *valor);

Valor consultar(const char *nome, int linha);
void guardar(const char *nome, Valor valor, int declaracao, int linha);

void registrar_funcao(No *funcao);
No *buscar_funcao(const char *nome, int linha);
void entrar_escopo(No *funcao, Valor *argumentos, size_t qtd_argumentos, int linha);
void sair_escopo(void);

void limpar_tabela(void);
#endif
