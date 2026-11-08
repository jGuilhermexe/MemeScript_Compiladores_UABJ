/*
 * main.c - Junta as etapas do MemeScript
 *
 * codigo-fonte --(Flex)--> tokens
 *             --(Bison)--> arvore sintatica
 *             --(Interpretador)--> resultado
 */

#include <stdio.h>
#include "arvore.h"

/* Arquivo que o Flex vai ler */
extern FILE *yyin;

/* Arvore construida pelo analisador sintatico */
extern No *raiz;

/* Funcao gerada pelo Bison */
int yyparse(void);

int main(int argc, char *argv[]) {

    /* Verifica se o usuario informou um arquivo */
    if (argc < 2) {
        printf("Uso: %s arquivo.ms\n", argv[0]);
        return 1;
    }

    /* Abre o arquivo com o codigo MemeScript */
    yyin = fopen(argv[1], "r");

    if (yyin == NULL) {
        printf("Nao foi possivel abrir %s\n", argv[1]);
        return 1;
    }

    /*
     * Etapas 1 e 2: analise lexica e sintatica.
     *
     * O yyparse() solicita tokens ao Flex e verifica
     * se eles seguem as regras da gramatica.
     *
     * Ao final, a arvore fica armazenada em "raiz".
     */
    if (yyparse() != 0) {
        fclose(yyin);
        return 1;
    }

    fclose(yyin);

    /* Exibe a arvore sintatica construida */
    printf("\n===== ARVORE SINTATICA =====\n");

    if (raiz != NULL) {
        imprimir_arvore(raiz);
    } else {
        printf("Programa vazio.\n");
    }

    /*
     * Etapa 3: interpretacao.
     *
     * Quando o interpretador estiver implementado,
     * descomente a chamada abaixo:
     *
     * interpretar(raiz);
     */

    return 0;
}
