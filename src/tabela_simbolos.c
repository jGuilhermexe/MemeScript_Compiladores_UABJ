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

static Escopo global;
static Escopo *escopo_atual = &global;
static Funcao *funcoes;

static Variavel *buscar_em(Variavel *lista, const char *nome) {
    for (Variavel *v = lista; v; v = v->proxima) {
        if (strcmp(v->nome, nome) == 0) {
            return v;
        }
    }
    return NULL;
}

static No *procurar_receita(const char *nome) {
    for (Funcao *f = funcoes; f; f = f->proxima) {
        if (strcmp(f->no->nome, nome) == 0) {
            return f->no;
        }
    }
    return NULL;
}

static void nome_ausente(const char *nome, int linha) {
    fprintf(stderr, "Erro semântico na linha %d: kd_a_variavel? Nome sem inicialização: %s\n",
            linha, nome);
    exit(1);
}

Valor copiar_valor(Valor valor) {
    if (valor.tipo == TEXTO_MS && valor.texto) {
        valor.texto = copiar_texto(valor.texto);
    } else if (valor.tipo == LISTA_MS && valor.tamanho > 0) {
        Valor *copia = alocar(valor.tamanho * sizeof(Valor));
        for (size_t i = 0; i < valor.tamanho; i++) {
            copia[i] = copiar_valor(valor.itens[i]);
        }
        valor.itens = copia;
    }
    return valor;
}

void liberar_valor(Valor *valor) {
    if (!valor) {
        return;
    }
    if (valor->tipo == TEXTO_MS) {
        free(valor->texto);
        valor->texto = NULL;
    } else if (valor->tipo == LISTA_MS && valor->itens) {
        for (size_t i = 0; i < valor->tamanho; i++) {
            liberar_valor(&valor->itens[i]);
        }
        free(valor->itens);
        valor->itens = NULL;
    }
}

/* Marca variaveis locais antes de executar receita */
static void marcar_locais(No *lista, Escopo *escopo) {
    for (No *no = lista; no; no = no->proximo) {
        if (no->tipo == NO_LANCEI && !buscar_em(escopo->variaveis, no->nome)) {
            Variavel *v = alocar(sizeof(Variavel));
            v->nome = no->nome;
            v->valor.tipo = NULO_MS;
            v->inicializada = 0;
            v->proxima = escopo->variaveis;
            escopo->variaveis = v;
        }
        marcar_locais(no->bloco_principal, escopo);
        marcar_locais(no->bloco_senao, escopo);
    }
}

void registrar_funcao(No *funcao) {
    if (procurar_receita(funcao->nome)) {
        erro("semântico", funcao->linha, "função já definida");
    }
    if (buscar_em(global.variaveis, funcao->nome)) {
        erro("semântico", funcao->linha, "nome já utilizado por variável");
    }
    for (No *p1 = funcao->expressao; p1; p1 = p1->proximo) {
        for (No *p2 = p1->proximo; p2; p2 = p2->proximo) {
            if (strcmp(p1->nome, p2->nome) == 0) {
                erro("semântico", funcao->linha, "parâmetros duplicados na função");
            }
        }
    }
    Funcao *f = alocar(sizeof(Funcao));
    f->no = funcao;
    f->proxima = funcoes;
    funcoes = f;
}

No *buscar_funcao(const char *nome, int linha) {
    No *funcao = procurar_receita(nome);
    if (!funcao) {
        erro("semântico", linha, "função não definida");
    }
    return funcao;
}

void entrar_escopo(No *funcao, Valor *argumentos, size_t qtd_argumentos, int linha) {
    size_t qtd_params = 0;
    for (No *p = funcao->expressao; p; p = p->proximo) {
        qtd_params++;
    }
    if (qtd_params != qtd_argumentos) {
        erro("semântico", linha, "quantidade incorreta de argumentos");
    }

    Escopo *novo = alocar(sizeof(Escopo));
    size_t i = 0;
    for (No *p = funcao->expressao; p; p = p->proximo) {
        if (procurar_receita(p->nome)) {
            erro("semântico", linha, "nome já utilizado por função");
        }
        Variavel *v = alocar(sizeof(Variavel));
        v->nome = p->nome;
        v->valor = argumentos[i++];
        v->inicializada = 1;
        v->proxima = novo->variaveis;
        novo->variaveis = v;
    }
    marcar_locais(funcao->bloco_principal, novo);
    novo->anterior = escopo_atual;
    escopo_atual = novo;
}

static void liberar_variaveis(Variavel *lista) {
    while (lista) {
        Variavel *atual = lista;
        lista = atual->proxima;
        liberar_valor(&atual->valor);
        free(atual);
    }
}

void sair_escopo(void) {
    Escopo *topo = escopo_atual;
    escopo_atual = topo->anterior;
    liberar_variaveis(topo->variaveis);
    free(topo);
}

Valor consultar(const char *nome, int linha) {
    Variavel *v = buscar_em(escopo_atual->variaveis, nome);
    if (!v && escopo_atual != &global) {
        v = buscar_em(global.variaveis, nome);
    }
    if (!v || !v->inicializada) {
        nome_ausente(nome, linha);
    }
    return copiar_valor(v->valor);
}

void guardar(const char *nome, Valor valor, int declaracao, int linha) {
    if (procurar_receita(nome)) {
        erro("semântico", linha, "nome já utilizado por função");
    }

    Variavel *v = buscar_em(escopo_atual->variaveis, nome);
    if (!declaracao && (!v || !v->inicializada)) {
        nome_ausente(nome, linha);
    }
    if (!v) {
        v = alocar(sizeof *v);
        v->nome = nome;
        v->proxima = escopo_atual->variaveis;
        escopo_atual->variaveis = v;
    }
    liberar_valor(&v->valor);
    v->valor = valor;
    v->inicializada = 1;
}

void limpar_tabela(void) {
    liberar_variaveis(global.variaveis);
    global.variaveis = NULL;
    while (funcoes) {
        Funcao *atual = funcoes;
        funcoes = atual->proxima;
        free(atual);
    }
}
