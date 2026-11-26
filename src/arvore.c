#include <stdio.h>
#include <stdlib.h>
#include "arvore.h"
#include "erro.h"
extern int linha_atual;

/* alocar inicia os campos com zero. */
No *novo_no(TipoNo tipo) {
    No *no = alocar(sizeof *no);
    no->tipo = tipo;
    no->linha = linha_atual;
    return no;
}

No *criar_numero(long long valor) {
    No *no = novo_no(NO_NUMERO);
    no->valor = valor;
    return no;
}

No *criar_real(double valor) {
    No *no = novo_no(NO_REAL);
    no->valor_real = valor;
    return no;
}

No *criar_texto(char *texto) {
    No *no = novo_no(NO_TEXTO);
    no->texto = texto;
    return no;
}

No *criar_variavel(char *nome) {
    No *no = novo_no(NO_VARIAVEL);
    no->nome = nome;
    return no;
}

No *criar_operacao(TipoNo tipo, No *esquerda, No *direita) {
    No *no = novo_no(tipo);
    no->esquerda = esquerda;
    no->direita = direita;
    return no;
}

No *criar_no_nomeado(TipoNo tipo, char *nome, No *expressao) {
    No *no = novo_no(tipo);
    no->nome = nome;
    no->expressao = expressao;
    return no;
}

No *criar_habla(No *expressao) {
    No *no = novo_no(NO_HABLA);
    no->expressao = expressao;
    return no;
}

No *criar_se_liga(No *condicao, No *bloco_entao, No *bloco_senao) {
    No *no = novo_no(NO_SE_LIGA);
    no->expressao = condicao;
    no->bloco_principal = bloco_entao;
    no->bloco_senao = bloco_senao;
    return no;
}

No *criar_enquanto_tankar(No *condicao, No *bloco) {
    No *no = novo_no(NO_ENQUANTO_TANKAR);
    no->expressao = condicao;
    no->bloco_principal = bloco;
    return no;
}

/* Coloca um comando no FINAL da lista de comandos */
No *adicionar_comando(No *lista, No *comando) {
    /* Se a lista estiver vazia, o comando sera o primeiro */
    if (lista == NULL) {
        return comando;
    }
    /* Percorre a lista ate encontrar o ultimo comando */
    No *atual = lista;
    while (atual->proximo != NULL) {
        atual = atual->proximo;
    }
    /* Liga o ultimo comando ao novo comando */
    atual->proximo = comando;
    return lista;
}

/* Os nomes seguem a ordem de TipoNo em arvore.h. */
static const char *nomes[] = {"LANCEI",
                              "HABLA",
                              "INTEIRO",
                              "REAL",
                              "TEXTO",
                              "VARIAVEL",
                              "SOMA",
                              "SUBTRACAO",
                              "MULTIPLICACAO",
                              "DIVISAO",
                              "ATRIBUICAO",
                              "MAIOR",
                              "MENOR",
                              "IGUAL",
                              "DIFERENTE",
                              "MAIOR_IGUAL",
                              "MENOR_IGUAL",
                              "SE_LIGA",
                              "ENQUANTO_TANKAR",
                              "PASSA_RETO",
                              "CHEGA",
                              "ENTRADA",
                              "BOOLEANO",
                              "NULO",
                              "MODULO",
                              "E",
                              "OU",
                              "NEM",
                              "POSITIVO",
                              "RECEITA",
                              "CHAMADA",
                              "DEVOLVE",
                              "LISTA",
                              "PARA_CADA"};

static void mostrar(No *lista, int nivel) {
    for (No *no = lista; no; no = no->proximo) {
        printf("%*s%s", nivel * 2, "", nomes[no->tipo]);
        if (no->nome) {
            printf(" %s", no->nome);
        }
        if (no->texto) {
            printf(" %s", no->texto);
        }
        if (no->tipo == NO_NUMERO) {
            printf(" %lld", no->valor);
        }
        if (no->tipo == NO_REAL) {
            printf(" %g", no->valor_real);
        }
        if (no->tipo == NO_BOOLEANO) {
            printf(" %s", no->valor ? "tankavel" : "intankavel");
        }
        printf(" [linha %d]\n", no->linha);
        mostrar(no->expressao, nivel + 1);
        mostrar(no->esquerda, nivel + 1);
        mostrar(no->direita, nivel + 1);
        mostrar(no->bloco_principal, nivel + 1);
        mostrar(no->bloco_senao, nivel + 1);
    }
}

void imprimir_arvore(No *programa) {
    mostrar(programa, 0);
}

void liberar_arvore(No *lista) {
    while (lista != NULL) {
        No *proximo = lista->proximo;
        liberar_arvore(lista->esquerda);
        liberar_arvore(lista->direita);
        liberar_arvore(lista->expressao);
        liberar_arvore(lista->bloco_principal);
        liberar_arvore(lista->bloco_senao);
        free(lista->nome);
        free(lista->texto);
        free(lista);
        lista = proximo;
    }
}
