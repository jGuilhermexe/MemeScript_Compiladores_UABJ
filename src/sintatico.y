
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
#include "erro.h"
extern int linha_atual;

/* Chamada automaticamente quando ocorre um erro sintatico */
void yyerror(const char *mensagem) {
    erro("sintático", linha_atual, mensagem);
}

No *raiz = NULL;   /* Arvore completa do programa */
%}

/* Valores que os tokens e regras podem carregar */
%locations
%define parse.error detailed
%union {
    long long inteiro;
    double real;
    char *texto;
    No *no;
}

/* Palavras reservadas da MemeScript */
%token LANCEI HABLA ENTRADA NULO VERDADE FALSO MODULO E OU NEM CHEGA PASSA RESERVADA
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
%left OU
%left E
%precedence NEM
%nonassoc IGUAL DIFERENTE MAIOR_QUE MENOR_QUE MAIOR_IGUAL MENOR_IGUAL
%left MAIS MENOS
%left MULT DIV MODULO
%precedence MENOS_UNARIO

/* Regras que produzem nos da arvore */
%type <no> programa lista_comandos comando expressao bloco
%type <no> cauda_condicional argumentos comandos_bloco

%%

programa
    : lista_comandos                { raiz = $1; }
    ;

lista_comandos
    : %empty                         { $$ = NULL; }
    | lista_comandos comando        { $$ = adicionar_comando($1, $2); }
    ;

comando
    : LANCEI IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_lancei($2, $4); }

    | IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_atribuicao($1, $3); }

    | HABLA argumentos NEWLINE       { $$ = criar_habla($2); }

    | SE_LIGA ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco cauda_condicional
                                    { $$ = criar_se_liga($3, $6, $7); $$->linha = @1.first_line; }

    | ENQUANTO_TANKAR ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco            { $$ = criar_enquanto_tankar($3, $6); $$->linha = @1.first_line; }
    | CHEGA NEWLINE                 { $$ = novo_no(NO_CHEGA); }
    | PASSA NEWLINE                 { $$ = novo_no(NO_PASSA_RETO); }
    | ENTRADA ABRE_PARENTESES FECHA_PARENTESES NEWLINE { $$ = novo_no(NO_ENTRADA); }
    ;

argumentos
    : expressao { $$ = $1; }
    | argumentos VIRGULA expressao { $$ = adicionar_comando($1,$3); }
    ;

comandos_bloco
    : comando { $$ = $1; }
    | comandos_bloco comando { $$ = adicionar_comando($1,$2); }
    ;

/* Bloco definido pela indentacao de quatro espacos */
bloco
    : NEWLINE INDENT comandos_bloco DEDENT
                                    { $$ = $3; }
    ;

/* Partes opcionais da estrutura condicional */
cauda_condicional
    : %empty                         { $$ = NULL; }

    | OU_SENAO ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco cauda_condicional
                                    { $$ = criar_se_liga($3, $6, $7); $$->linha = @1.first_line; }

    | SENAO_FIO DOIS_PONTOS bloco   { $$ = $3; }
    ;

/* Expressoes: numeros, textos, variaveis e operacoes */
expressao
    : ENTRADA ABRE_PARENTESES FECHA_PARENTESES { $$ = novo_no(NO_ENTRADA); }
    | VERDADE { $$ = novo_no(NO_BOOLEANO); $$->valor = 1; }
    | FALSO   { $$ = novo_no(NO_BOOLEANO); }
    | NULO    { $$ = novo_no(NO_NULO); }
    | MAIS expressao %prec MENOS_UNARIO { $$ = criar_operacao(NO_POSITIVO,$2,NULL); }
    | NEM expressao { $$ = criar_operacao(NO_NEM,$2,NULL); }
    | expressao MODULO expressao { $$ = criar_operacao(NO_MODULO,$1,$3); }
    | expressao E expressao      { $$ = criar_operacao(NO_E,$1,$3); }
    | expressao OU expressao     { $$ = criar_operacao(NO_OU,$1,$3); }
    | INTEIRO                       { $$ = criar_numero($1); }
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
