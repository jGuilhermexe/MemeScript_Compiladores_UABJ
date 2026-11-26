/* O Bison monta a árvore que o interpretador vai executar. */

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
/* Com barras iguais, o token seguinte pode decidir se a lista está vazia. */
%glr-parser
%expect 3
%locations
%define parse.error detailed
%union {
    long long inteiro;
    double real;
    char *texto;
    No *no;
}

/* Palavras reservadas da MemeScript */
%token LANCEI HABLA ENTRADA NULO VERDADE FALSO MODULO E OU NEM CHEGA PASSA RECEITA DEVOLVE PARA_CADA EM BARRA
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

/* Regras que produzem nos da arvore */
%type <no> conjuncao negacao comparacao soma produto unario primario
%type <no> itens item funcao comandos comando expressao bloco
%type <no> cauda_condicional argumentos entrada chamada parametros nomes argumentos_opcionais

%%

programa
    : %empty   { raiz = NULL; }
    | itens { raiz = $1; }
    ;

itens
    : item
    | itens item { $$ = adicionar_comando($1, $2); }
    ;

item
    : comando
    | funcao
    ;

comandos
    : comando          { $$ = $1; }
    | comandos comando { $$ = adicionar_comando($1, $2); }
    ;

comando
    : LANCEI IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_no_nomeado(NO_LANCEI, $2, $4); }

    | IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_no_nomeado(NO_ATRIBUICAO, $1, $3); }

    | HABLA argumentos NEWLINE       { $$ = criar_habla($2); }

    | SE_LIGA ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco cauda_condicional
                                    { $$ = criar_se_liga($3, $6, $7); $$->linha = @1.first_line; }

    | ENQUANTO_TANKAR ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco            { $$ = criar_enquanto_tankar($3, $6); $$->linha = @1.first_line; }
    | PARA_CADA IDENTIFICADOR EM expressao DOIS_PONTOS bloco
        { $$ = criar_no_nomeado(NO_PARA_CADA, $2, $4); $$->bloco_principal = $6; $$->linha = @1.first_line; }
    | DEVOLVE NEWLINE { $$ = novo_no(NO_DEVOLVE); }
    | DEVOLVE expressao NEWLINE { $$ = novo_no(NO_DEVOLVE); $$->expressao = $2; }
    | chamada NEWLINE { $$ = $1; }
    | CHEGA NEWLINE                 { $$ = novo_no(NO_CHEGA); }
    | PASSA NEWLINE                 { $$ = novo_no(NO_PASSA_RETO); }
    | entrada NEWLINE             { $$ = $1; $$->linha = @2.first_line; }
    ;

funcao
    : RECEITA IDENTIFICADOR ABRE_PARENTESES parametros FECHA_PARENTESES DOIS_PONTOS bloco
        { $$ = criar_no_nomeado(NO_RECEITA, $2, $4); $$->bloco_principal = $7; $$->linha = @1.first_line; }
    ;

parametros
    : %empty { $$ = NULL; }
    | nomes
    ;

nomes
    : IDENTIFICADOR { $$ = criar_variavel($1); }
    | nomes VIRGULA IDENTIFICADOR { $$ = adicionar_comando($1, criar_variavel($3)); }
    ;

chamada
    : IDENTIFICADOR ABRE_PARENTESES argumentos_opcionais FECHA_PARENTESES
        { $$ = criar_no_nomeado(NO_CHAMADA, $1, $3); }
    ;

argumentos_opcionais
    : %empty { $$ = NULL; }
    | argumentos
    ;

argumentos
    : expressao { $$ = $1; }
    | argumentos VIRGULA expressao { $$ = adicionar_comando($1,$3); }
    ;

/* Bloco definido pela indentacao de quatro espacos */
bloco
    : NEWLINE INDENT comandos DEDENT
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

entrada
    : ENTRADA ABRE_PARENTESES FECHA_PARENTESES { $$ = novo_no(NO_ENTRADA); }
    ;

/* Cada regra abaixo tem prioridade sobre a que a utiliza. */
expressao
    : conjuncao
    | expressao OU conjuncao { $$ = criar_operacao(NO_OU, $1, $3); }
    ;

conjuncao
    : negacao
    | conjuncao E negacao { $$ = criar_operacao(NO_E, $1, $3); }
    ;

negacao
    : comparacao
    | NEM negacao { $$ = criar_operacao(NO_NEM, $2, NULL); }
    ;

comparacao
    : soma
    | soma IGUAL soma { $$ = criar_operacao(NO_IGUAL, $1, $3); }
    | soma DIFERENTE soma { $$ = criar_operacao(NO_DIFERENTE, $1, $3); }
    | soma MAIOR_QUE soma { $$ = criar_operacao(NO_MAIOR_QUE, $1, $3); }
    | soma MENOR_QUE soma { $$ = criar_operacao(NO_MENOR_QUE, $1, $3); }
    | soma MAIOR_IGUAL soma { $$ = criar_operacao(NO_MAIOR_IGUAL, $1, $3); }
    | soma MENOR_IGUAL soma { $$ = criar_operacao(NO_MENOR_IGUAL, $1, $3); }
    ;

soma
    : produto
    | soma MAIS produto { $$ = criar_operacao(NO_SOMA, $1, $3); }
    | soma MENOS produto { $$ = criar_operacao(NO_SUBTRACAO, $1, $3); }
    ;

produto
    : unario
    | produto MULT unario { $$ = criar_operacao(NO_MULTIPLICACAO, $1, $3); }
    | produto DIV unario { $$ = criar_operacao(NO_DIVISAO, $1, $3); }
    | produto MODULO unario { $$ = criar_operacao(NO_MODULO, $1, $3); }
    ;

unario
    : primario
    | MAIS unario { $$ = criar_operacao(NO_POSITIVO, $2, NULL); }
    | MENOS unario { $$ = criar_operacao(NO_SUBTRACAO, criar_numero(0), $2); }
    ;

/* Expressoes: numeros, textos, variaveis e operacoes */
primario
    : entrada { $$ = $1; }
    | chamada
    | BARRA BARRA { $$ = novo_no(NO_LISTA); }
    | BARRA argumentos BARRA { $$ = novo_no(NO_LISTA); $$->expressao = $2; }
    | VERDADE { $$ = novo_no(NO_BOOLEANO); $$->valor = 1; }
    | FALSO   { $$ = novo_no(NO_BOOLEANO); }
    | NULO    { $$ = novo_no(NO_NULO); }
    | INTEIRO                       { $$ = criar_numero($1); }
    | REAL                          { $$ = criar_real($1); }
    | TEXTO                         { $$ = criar_texto($1); }
    | IDENTIFICADOR                 { $$ = criar_variavel($1); }
    | ABRE_PARENTESES expressao FECHA_PARENTESES
                                    { $$ = $2; }
    ;

%%
