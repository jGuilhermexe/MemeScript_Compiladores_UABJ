
/*
 * arvore.c
 * Funcoes para CRIAR os nos da arvore
 * e para IMPRIMIR a arvore do MemeScript.
 */

#include <stdio.h>
#include <stdlib.h>
#include "arvore.h"


/* Cria um no vazio do tipo pedido.
 * O calloc inicializa todos os campos com zero ou NULL. */
No *novo_no(TipoNo tipo) {

    No *no = calloc(1, sizeof(No));

    if (no == NULL) {
        printf("Erro ao alocar memoria!\n");
        exit(1);
    }

    no->tipo = tipo;

    return no;
}


/* Cria um no que representa um numero inteiro */
No *criar_numero(int valor) {

    No *no = novo_no(NO_NUMERO);

    no->valor = valor;

    return no;
}


/* Cria um no que representa um numero decimal */
No *criar_real(double valor) {

    No *no = novo_no(NO_REAL);

    no->valor_real = valor;

    return no;
}


/* Cria um no que representa um texto */
No *criar_texto(char *texto) {

    No *no = novo_no(NO_TEXTO);

    no->texto = texto;

    return no;
}


/* Cria um no que representa uma variavel */
No *criar_variavel(char *nome) {

    No *no = novo_no(NO_VARIAVEL);

    no->nome = nome;

    return no;
}


/* Cria um no para uma operacao matematica.
 * O tipo informa se e soma, subtracao, multiplicacao ou divisao. */
No *criar_operacao(TipoNo tipo, No *esquerda, No *direita) {

    No *no = novo_no(tipo);

    no->esquerda = esquerda;
    no->direita = direita;

    return no;
}


/* Cria um no para o comando LANCEI */
No *criar_lancei(char *nome, No *expressao) {

    No *no = novo_no(NO_LANCEI);

    no->nome = nome;
    no->expressao = expressao;

    return no;
}


/* Cria um no para atribuir um novo valor a uma variavel */
No *criar_atribuicao(char *nome, No *expressao) {

    No *no = novo_no(NO_ATRIBUICAO);

    no->nome = nome;
    no->expressao = expressao;

    return no;
}


/* Cria um no para o comando HABLA */
No *criar_habla(No *expressao) {

    No *no = novo_no(NO_HABLA);

    no->expressao = expressao;

    return no;
}


/* Cria um no para uma estrutura condicional */
No *criar_se_liga(No *condicao, No *bloco_entao, No *bloco_senao) {

    No *no = novo_no(NO_SE_LIGA);

    no->expressao = condicao;
    no->bloco_entao = bloco_entao;
    no->bloco_senao = bloco_senao;

    return no;
}


/* Cria um no para uma estrutura de repeticao */
No *criar_enquanto_tankar(No *condicao, No *bloco) {

    No *no = novo_no(NO_ENQUANTO_TANKAR);

    no->expressao = condicao;
    no->bloco_while = bloco;

    return no;
}


/* Cria um no para interromper uma repeticao */
No *criar_passar_reto(void) {

    No *no = novo_no(NO_PASSA_RETO);

    return no;
}


/* Coloca um comando no FINAL da lista de comandos */
No *adicionar_comando(No *lista, No *comando) {

    /* Se a lista estiver vazia, o comando sera o primeiro */
    if (lista == NULL)
        return comando;

    /* Percorre a lista ate encontrar o ultimo comando */
    No *atual = lista;

    while (atual->proximo != NULL)
        atual = atual->proximo;

    /* Liga o ultimo comando ao novo comando */
    atual->proximo = comando;

    return lista;
}


/* ===================== IMPRESSAO DA ARVORE ===================== */


/* Imprime uma expressao, como numero, variavel ou operacao */
void imprimir_expressao(No *no) {

    /* Se nao existir expressao, nao ha nada para imprimir */
    if (no == NULL)
        return;

    if (no->tipo == NO_NUMERO)
        printf("    NUMERO %d\n", no->valor);

    else if (no->tipo == NO_REAL)
        printf("    REAL %.2f\n", no->valor_real);

    else if (no->tipo == NO_TEXTO)
        printf("    TEXTO %s\n", no->texto);

    else if (no->tipo == NO_VARIAVEL)
        printf("    VARIAVEL %s\n", no->nome);

    else if (no->tipo == NO_SOMA) {
        printf("    SOMA\n");
        imprimir_expressao(no->esquerda);
        imprimir_expressao(no->direita);
    }

    else if (no->tipo == NO_SUBTRACAO) {
        printf("    SUBTRACAO\n");
        imprimir_expressao(no->esquerda);
        imprimir_expressao(no->direita);
    }

    else if (no->tipo == NO_MULTIPLICACAO) {
        printf("    MULTIPLICACAO\n");
        imprimir_expressao(no->esquerda);
        imprimir_expressao(no->direita);
    }

    else if (no->tipo == NO_DIVISAO) {
        printf("    DIVISAO\n");
        imprimir_expressao(no->esquerda);
        imprimir_expressao(no->direita);
    }

    else if (no->tipo == NO_MAIOR_QUE ||
             no->tipo == NO_MENOR_QUE ||
             no->tipo == NO_IGUAL ||
             no->tipo == NO_DIFERENTE ||
             no->tipo == NO_MAIOR_IGUAL ||
             no->tipo == NO_MENOR_IGUAL) {

        printf("    COMPARACAO\n");
        imprimir_expressao(no->esquerda);
        imprimir_expressao(no->direita);
    }

    else {
        printf("    EXPRESSAO DESCONHECIDA\n");
    }
}


/* Percorre a lista de comandos e imprime cada um */
void imprimir_arvore(No *programa) {

    for (No *cmd = programa; cmd != NULL; cmd = cmd->proximo) {

        if (cmd->tipo == NO_LANCEI) {
            printf("LANCEI %s\n", cmd->nome);
            imprimir_expressao(cmd->expressao);
        }

        else if (cmd->tipo == NO_ATRIBUICAO) {
            printf("ATRIBUICAO %s\n", cmd->nome);
            imprimir_expressao(cmd->expressao);
        }

        else if (cmd->tipo == NO_HABLA) {
            printf("HABLA\n");
            imprimir_expressao(cmd->expressao);
        }

        else if (cmd->tipo == NO_SE_LIGA) {
            printf("SE_LIGA\n");
            imprimir_expressao(cmd->expressao);

            printf("  BLOCO ENTAO:\n");
            imprimir_arvore(cmd->bloco_entao);

            if (cmd->bloco_senao != NULL) {
                printf("  BLOCO SENAO:\n");
                imprimir_arvore(cmd->bloco_senao);
            }
        }

        else if (cmd->tipo == NO_ENQUANTO_TANKAR) {
            printf("ENQUANTO_TANKAR\n");
            imprimir_expressao(cmd->expressao);

            printf("  BLOCO:\n");
            imprimir_arvore(cmd->bloco_while);
        }

        else if (cmd->tipo == NO_PASSA_RETO) {
            printf("PASSA_RETO\n");
        }
    }
}
