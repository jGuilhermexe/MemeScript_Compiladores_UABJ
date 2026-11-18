#include "tabela_simbolos.h"
#include "arvore.h"
#include "erro.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Cada celula guarda nome, valor e proxima variavel */
typedef struct Variavel {
    const char *nome;
    Valor valor;
    int inicializada;
    struct Variavel *proxima;
} Variavel;

typedef struct Escopo {
    Variavel *variaveis;
    struct Escopo *anterior;
} Escopo;

typedef struct Funcao {
    No *no;
    struct Funcao *proxima;
} Funcao;

static Variavel *primeira;
static Escopo *escopo_atual;
static Funcao *funcoes;

static Variavel *buscar_em(Variavel *lista, const char *nome) {
    for (Variavel *v = lista; v; v = v->proxima)
        if (strcmp(v->nome,nome) == 0) return v;
    return NULL;
}

static int funcao_existe(const char *nome) {
    for (Funcao *f = funcoes; f; f = f->proxima)
        if (strcmp(f->no->nome,nome) == 0) return 1;
    return 0;
}

static void nome_ausente(const char *nome, int linha) {
    fprintf(stderr,"Erro semântico na linha %d: kd_a_variavel? Nome sem inicialização: %s\n",linha,nome);
    exit(1);
}

Valor copiar_valor(Valor valor) {
    if (valor.tipo == TEXTO_MS && valor.texto) {
        valor.texto = copiar_texto(valor.texto);
    } else if (valor.tipo == LISTA_MS && valor.tamanho > 0) {
        Valor *copia = alocar(valor.tamanho * sizeof(Valor));
        for (int i = 0; i < valor.tamanho; i++) {
            copia[i] = copiar_valor(valor.itens[i]);
        }
        valor.itens = copia;
    }
    return valor;
}

void liberar_valor(Valor *valor) {
    if (!valor) return;
    if (valor->tipo == TEXTO_MS) {
        free(valor->texto);
        valor->texto = NULL;
    } else if (valor->tipo == LISTA_MS && valor->itens) {
        for (int i = 0; i < valor->tamanho; i++) {
            liberar_valor(&valor->itens[i]);
        }
        free(valor->itens);
        valor->itens = NULL;
    }
}

/* Marca variaveis locais antes de executar receita */
static void marcar_locais(No *lista, Escopo *escopo) {
    for (No *no = lista; no; no = no->proximo) {
        if ((no->tipo == NO_LANCEI || no->tipo == NO_PARA_CADA) &&
            !buscar_em(escopo->variaveis, no->nome)) {
            Variavel *v = alocar(sizeof(Variavel));
            v->nome = no->nome;
            v->valor.tipo = NULO_MS;
            v->inicializada = 0;
            v->proxima = escopo->variaveis;
            escopo->variaveis = v;
        }
        marcar_locais(no->bloco_entao, escopo);
        marcar_locais(no->bloco_senao, escopo);
        marcar_locais(no->bloco_while, escopo);
    }
}

void registrar_funcao(No *funcao) {
    if (funcao_existe(funcao->nome))
        erro("semântico", funcao->linha, "função já definida");
    if (buscar_em(primeira, funcao->nome))
        erro("semântico", funcao->linha, "nome já utilizado por variável");
    for (No *p1 = funcao->parametros; p1; p1 = p1->proximo) {
        for (No *p2 = p1->proximo; p2; p2 = p2->proximo) {
            if (strcmp(p1->nome, p2->nome) == 0)
                erro("semântico", funcao->linha, "parâmetros duplicados na função");
        }
    }
    Funcao *f = alocar(sizeof(Funcao));
    f->no = funcao;
    f->proxima = funcoes;
    funcoes = f;
}

No *buscar_funcao(const char *nome, int linha) {
    for (Funcao *f = funcoes; f; f = f->proxima)
        if (strcmp(f->no->nome, nome) == 0) return f->no;
    erro("semântico", linha, "função não definida");
    return NULL;
}

void entrar_escopo(No *funcao, Valor *argumentos, int qtd_argumentos, int linha) {
    int qtd_params = 0;
    for (No *p = funcao->parametros; p; p = p->proximo) qtd_params++;
    if (qtd_params != qtd_argumentos)
        erro("semântico", linha, "quantidade incorreta de argumentos");

    Escopo *novo = alocar(sizeof(Escopo));
    int i = 0;
    for (No *p = funcao->parametros; p; p = p->proximo) {
        Variavel *v = alocar(sizeof(Variavel));
        v->nome = p->nome;
        v->valor = argumentos[i++];
        v->inicializada = 1;
        v->proxima = novo->variaveis;
        novo->variaveis = v;
    }
    marcar_locais(funcao->bloco_entao, novo);
    novo->anterior = escopo_atual;
    escopo_atual = novo;
}

void sair_escopo(void) {
    if (!escopo_atual) return;
    Escopo *topo = escopo_atual;
    escopo_atual = topo->anterior;
    while (topo->variaveis) {
        Variavel *atual = topo->variaveis;
        topo->variaveis = atual->proxima;
        liberar_valor(&atual->valor);
        free(atual);
    }
    free(topo);
}

Valor consultar(const char *nome, int linha) {
    if (escopo_atual) {
        Variavel *v = buscar_em(escopo_atual->variaveis, nome);
        if (v) {
            if (!v->inicializada) nome_ausente(nome, linha);
            return copiar_valor(v->valor);
        }
    }
    Variavel *v = buscar_em(primeira, nome);
    if (!v || !v->inicializada) nome_ausente(nome, linha);
    return copiar_valor(v->valor);
}

void guardar(const char *nome, Valor valor, int declaracao, int linha) {
    if (funcao_existe(nome))
        erro("semântico", linha, "nome já utilizado por função");

    if (escopo_atual) {
        Variavel *v = buscar_em(escopo_atual->variaveis, nome);
        if (!v) {
            if (!declaracao) nome_ausente(nome, linha);
            v = alocar(sizeof(Variavel));
            v->nome = nome;
            v->proxima = escopo_atual->variaveis;
            escopo_atual->variaveis = v;
        } else if (!declaracao && !v->inicializada) {
            nome_ausente(nome, linha);
        }
        liberar_valor(&v->valor);
        v->valor = valor;
        v->inicializada = 1;
        return;
    }

    Variavel *v = buscar_em(primeira, nome);
    if (!v) {
        if (!declaracao) nome_ausente(nome, linha);
        v = alocar(sizeof(Variavel));
        v->nome = nome;
        v->proxima = primeira;
        primeira = v;
    }
    liberar_valor(&v->valor);
    v->valor = valor;
    v->inicializada = 1;
}

void limpar_tabela(void) {
    while (escopo_atual) sair_escopo();
    while (primeira) {
        Variavel *atual = primeira;
        primeira = atual->proxima;
        liberar_valor(&atual->valor);
        free(atual);
    }
    while (funcoes) {
        Funcao *atual = funcoes;
        funcoes = atual->proxima;
        free(atual);
    }
}