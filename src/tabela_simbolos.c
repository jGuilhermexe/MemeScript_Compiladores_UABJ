#include "tabela_simbolos.h"
#include "erro.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Cada célula guarda o nome, o valor atual e o caminho até a próxima variável. */
typedef struct Variavel {
    const char *nome;
    Valor valor;
    struct Variavel *proxima;
} Variavel;
static Variavel *primeira;

static Variavel *buscar(const char *nome) {
    for (Variavel *v = primeira; v; v = v->proxima)
        if (strcmp(v->nome,nome) == 0) return v;
    return NULL;
}
static void nome_ausente(const char *nome, int linha) {
    fprintf(stderr,"Erro semântico na linha %d: kd_a_variavel? Nome sem inicialização: %s\n",linha,nome);
    exit(1);
}
Valor consultar(const char *nome, int linha) {
    Variavel *v = buscar(nome);
    if (!v) nome_ausente(nome,linha);
    Valor copia = v->valor;
    if (copia.texto) copia.texto = copiar_texto(copia.texto);
    return copia;
}
void guardar(const char *nome, Valor valor, int declaracao, int linha) {
    Variavel *v = buscar(nome);
    if (!v) {
        if (!declaracao) nome_ausente(nome,linha);
        v = alocar(sizeof(Variavel));
        v->nome = nome;
        v->proxima = primeira;
        primeira = v;
    }
    free(v->valor.texto);
    v->valor = valor;
}
void limpar_tabela(void) {
    while (primeira) {
        Variavel *atual = primeira;
        primeira = atual->proxima;
        free(atual->valor.texto);
        free(atual);
    }
}
