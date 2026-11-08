
/*
 * sintatico.y - ANALISADOR SINTATICO (Bison)
 *
 * Funcao: receber os tokens do Flex, verificar se estao
 * na ordem correta e construir a arvore sintatica (AST).
 *
 * Importante: aqui nao executamos nada.
 * Apenas montamos a arvore.
 * Quem executa e o interpretador (interpretador.c).
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include "arvore.h"

int yylex(void);
extern int yylineno;

/* Chamada automaticamente quando ocorre um erro sintatico */
void yyerror(const char *mensagem) {
    printf("Erro sintatico na linha %d: %s\n", yylineno, mensagem);
}

No *raiz = NULL;   /* Arvore completa do programa */
%}

/* Valores que os tokens e regras podem carregar */
%union {
    int inteiro;
    double real;
    char *texto;
    No *no;
}

/* Palavras reservadas da MemeScript */
%token LANCEI HABLA
%token SE_LIGA OU_SENAO SENAO_FIO ENQUANTO_TANKAR

/* Operadores e simbolos */
%token ATRIBUICAO MAIS MENOS MULT DIV
%token IGUAL DIFERENTE MAIOR_QUE MENOR_QUE MAIOR_IGUAL MENOR_IGUAL
%token DOIS_PONTOS VIRGULA ABRE_PARENTESES FECHA_PARENTESES

/* Tokens de indentacao */
%token NEWLINE INDENT DEDENT

/* Tokens que carregam valores */
%token <inteiro> INTEIRO
%token <real> REAL
%token <texto> TEXTO IDENTIFICADOR

/* Precedencia dos operadores */
%nonassoc IGUAL DIFERENTE MAIOR_QUE MENOR_QUE MAIOR_IGUAL MENOR_IGUAL
%left MAIS MENOS
%left MULT DIV
%right MENOS_UNARIO

/* Regras que produzem nos da arvore */
%type <no> programa lista_comandos comando expressao bloco
%type <no> cauda_condicional

%%

programa
    : lista_comandos                { raiz = $1; }
    ;

lista_comandos
    :                               { $$ = NULL; }
    | lista_comandos comando        { $$ = adicionar_comando($1, $2); }
    ;

comando
    : LANCEI IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_lancei($2, $4); }

    | IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_atribuicao($1, $3); }

    | HABLA expressao NEWLINE       { $$ = criar_habla($2); }

    | SE_LIGA ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco cauda_condicional
                                    { $$ = criar_se_liga($3, $6, $7); }

    | ENQUANTO_TANKAR ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco            { $$ = criar_enquanto_tankar($3, $6); }
    ;

/* Bloco definido pela indentacao de quatro espacos */
bloco
    : NEWLINE INDENT lista_comandos DEDENT
                                    { $$ = $3; }
    ;

/* Partes opcionais da estrutura condicional */
cauda_condicional
    :                               { $$ = NULL; }

    | OU_SENAO ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco cauda_condicional
                                    { $$ = criar_se_liga($3, $6, $7); }

    | SENAO_FIO DOIS_PONTOS bloco   { $$ = $3; }
    ;

/* Expressoes: numeros, textos, variaveis e operacoes */
expressao
    : INTEIRO                       { $$ = criar_numero($1); }
    | REAL                          { $$ = criar_real($1); }
    | TEXTO                         { $$ = criar_texto($1); }
    | IDENTIFICADOR                 { $$ = criar_variavel($1); }
    | ABRE_PARENTESES expressao FECHA_PARENTESES
                                    { $$ = $2; }

    | expressao MAIS expressao      { $$ = criar_operacao(NO_SOMA, $1, $3); }
    | expressao MENOS expressao     { $$ = criar_operacao(NO_SUBTRACAO, $1, $3); }
    | expressao MULT expressao      { $$ = criar_operacao(NO_MULTIPLICACAO, $1, $3); }
    | expressao DIV expressao       { $$ = criar_operacao(NO_DIVISAO, $1, $3); }

    | MENOS expressao %prec MENOS_UNARIO
                                    { $$ = criar_operacao(NO_SUBTRACAO, criar_numero(0), $2); }

    | expressao IGUAL expressao     { $$ = criar_operacao(NO_IGUAL, $1, $3); }
    | expressao DIFERENTE expressao { $$ = criar_operacao(NO_DIFERENTE, $1, $3); }
    | expressao MAIOR_QUE expressao { $$ = criar_operacao(NO_MAIOR_QUE, $1, $3); }
    | expressao MENOR_QUE expressao { $$ = criar_operacao(NO_MENOR_QUE, $1, $3); }
    | expressao MAIOR_IGUAL expressao
                                    { $$ = criar_operacao(NO_MAIOR_IGUAL, $1, $3); }
    | expressao MENOR_IGUAL expressao
                                    { $$ = criar_operacao(NO_MENOR_IGUAL, $1, $3); }
    ;

%%
