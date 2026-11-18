/* sintatico.y - Analisador sintatico (Bison) */

%{
#include <stdio.h>
#include <stdlib.h>
#include "arvore.h"

int yylex(void);
#include "erro.h"
extern int linha_atual;

void yyerror(const char *mensagem) {
    erro("sintático", linha_atual, mensagem);
}

No *raiz = NULL;
%}

/* Valores que tokens e regras podem carregar */
%locations
%define parse.error detailed
%union {
    long long inteiro;
    double real;
    char *texto;
    No *no;
}

/* Palavras reservadas da MemeScript */
%token LANCEI HABLA ENTRADA NULO VERDADE FALSO MODULO E OU NEM CHEGA PASSA
%token RECEITA DEVOLVE PARA_CADA EM
%token SE_LIGA OU_SENAO SENAO_FIO ENQUANTO_TANKAR

/* Operadores e simbolos */
%token ATRIBUICAO MAIS MENOS MULT DIV
%token IGUAL DIFERENTE MAIOR_QUE MENOR_QUE MAIOR_IGUAL MENOR_IGUAL
%token DOIS_PONTOS VIRGULA ABRE_PARENTESES FECHA_PARENTESES BARRA

/* Tokens de indentacao */
%token NEWLINE INDENT DEDENT

/* Tokens com valores */
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
%type <no> programa lista_comandos comando expressao expressao_base lista bloco
%type <no> cauda_condicional argumentos argumentos_lista comandos_bloco
%type <no> parametros lista_parametros args_chamada

%%

programa
    : lista_comandos                { raiz = $1; }
    ;

lista_comandos
    : %empty                        { $$ = NULL; }
    | lista_comandos comando        { $$ = adicionar_comando($1, $2); }
    ;

comando
    : LANCEI IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_lancei($2, $4); }

    | IDENTIFICADOR ATRIBUICAO expressao NEWLINE
                                    { $$ = criar_atribuicao($1, $3); }

    | HABLA argumentos NEWLINE      { $$ = criar_habla($2); }

    | SE_LIGA ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco cauda_condicional
                                    { $$ = criar_se_liga($3, $6, $7); $$->linha = @1.first_line; }

    | ENQUANTO_TANKAR ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco             { $$ = criar_enquanto_tankar($3, $6); $$->linha = @1.first_line; }

    | PARA_CADA IDENTIFICADOR EM expressao DOIS_PONTOS bloco
                                    { $$ = criar_para_cada($2, $4, $6); $$->linha = @1.first_line; }

    | RECEITA IDENTIFICADOR ABRE_PARENTESES parametros FECHA_PARENTESES DOIS_PONTOS bloco
                                    { $$ = criar_receita($2, $4, $7); $$->linha = @1.first_line; }

    | DEVOLVE NEWLINE               { $$ = criar_devolve(NULL); $$->linha = @1.first_line; }
    | DEVOLVE expressao NEWLINE     { $$ = criar_devolve($2); $$->linha = @1.first_line; }

    | IDENTIFICADOR ABRE_PARENTESES args_chamada FECHA_PARENTESES NEWLINE
                                    { $$ = criar_chamada($1, $3); $$->linha = @1.first_line; }

    | CHEGA NEWLINE                 { $$ = novo_no(NO_CHEGA); }
    | PASSA NEWLINE                 { $$ = novo_no(NO_PASSA_RETO); }
    | ENTRADA ABRE_PARENTESES FECHA_PARENTESES NEWLINE { $$ = novo_no(NO_ENTRADA); }
    ;

parametros
    : %empty                        { $$ = NULL; }
    | lista_parametros              { $$ = $1; }
    ;

lista_parametros
    : IDENTIFICADOR                 { $$ = criar_variavel($1); }
    | lista_parametros VIRGULA IDENTIFICADOR
                                    { $$ = adicionar_comando($1, criar_variavel($3)); }
    ;

args_chamada
    : %empty                        { $$ = NULL; }
    | argumentos                    { $$ = $1; }
    ;

argumentos
    : expressao                     { $$ = $1; }
    | argumentos VIRGULA expressao  { $$ = adicionar_comando($1, $3); }
    ;

argumentos_lista
    : expressao_base                { $$ = $1; }
    | argumentos_lista VIRGULA expressao_base
                                    { $$ = adicionar_comando($1, $3); }
    ;

comandos_bloco
    : comando                       { $$ = $1; }
    | comandos_bloco comando        { $$ = adicionar_comando($1, $2); }
    ;

/* Bloco por indentacao de 4 espacos */
bloco
    : NEWLINE INDENT comandos_bloco DEDENT
                                    { $$ = $3; }
    ;

/* Partes opcionais de estrutura condicional */
cauda_condicional
    : %empty                        { $$ = NULL; }

    | OU_SENAO ABRE_PARENTESES expressao FECHA_PARENTESES
      DOIS_PONTOS bloco cauda_condicional
                                    { $$ = criar_se_liga($3, $6, $7); $$->linha = @1.first_line; }

    | SENAO_FIO DOIS_PONTOS bloco   { $$ = $3; }
    ;

lista
    : BARRA BARRA                   { $$ = criar_lista(NULL); }
    | BARRA argumentos_lista BARRA  { $$ = criar_lista($2); }
    ;

expressao
    : expressao_base                { $$ = $1; }
    | lista                         { $$ = $1; }
    | lista IGUAL lista             { $$ = criar_operacao(NO_IGUAL, $1, $3); }
    | lista DIFERENTE lista         { $$ = criar_operacao(NO_DIFERENTE, $1, $3); }
    | lista MAIOR_QUE lista         { $$ = criar_operacao(NO_MAIOR_QUE, $1, $3); }
    | lista MENOR_QUE lista         { $$ = criar_operacao(NO_MENOR_QUE, $1, $3); }
    | lista MAIOR_IGUAL lista       { $$ = criar_operacao(NO_MAIOR_IGUAL, $1, $3); }
    | lista MENOR_IGUAL lista       { $$ = criar_operacao(NO_MENOR_IGUAL, $1, $3); }
    | lista IGUAL expressao_base    { $$ = criar_operacao(NO_IGUAL, $1, $3); }
    | lista DIFERENTE expressao_base
                                    { $$ = criar_operacao(NO_DIFERENTE, $1, $3); }
    | lista MAIOR_QUE expressao_base
                                    { $$ = criar_operacao(NO_MAIOR_QUE, $1, $3); }
    | lista MENOR_QUE expressao_base
                                    { $$ = criar_operacao(NO_MENOR_QUE, $1, $3); }
    | lista MAIOR_IGUAL expressao_base
                                    { $$ = criar_operacao(NO_MAIOR_IGUAL, $1, $3); }
    | lista MENOR_IGUAL expressao_base
                                    { $$ = criar_operacao(NO_MENOR_IGUAL, $1, $3); }
    | expressao_base IGUAL lista    { $$ = criar_operacao(NO_IGUAL, $1, $3); }
    | expressao_base DIFERENTE lista
                                    { $$ = criar_operacao(NO_DIFERENTE, $1, $3); }
    | expressao_base MAIOR_QUE lista
                                    { $$ = criar_operacao(NO_MAIOR_QUE, $1, $3); }
    | expressao_base MENOR_QUE lista
                                    { $$ = criar_operacao(NO_MENOR_QUE, $1, $3); }
    | expressao_base MAIOR_IGUAL lista
                                    { $$ = criar_operacao(NO_MAIOR_IGUAL, $1, $3); }
    | expressao_base MENOR_IGUAL lista
                                    { $$ = criar_operacao(NO_MENOR_IGUAL, $1, $3); }
    ;

/* Expressoes: numeros, textos, variaveis, operacoes */
expressao_base
    : ENTRADA ABRE_PARENTESES FECHA_PARENTESES { $$ = novo_no(NO_ENTRADA); }
    | VERDADE                       { $$ = novo_no(NO_BOOLEANO); $$->valor = 1; }
    | FALSO                         { $$ = novo_no(NO_BOOLEANO); }
    | NULO                          { $$ = novo_no(NO_NULO); }
    | MAIS expressao_base %prec MENOS_UNARIO
                                    { $$ = criar_operacao(NO_POSITIVO, $2, NULL); }
    | NEM expressao_base            { $$ = criar_operacao(NO_NEM, $2, NULL); }
    | expressao_base MODULO expressao_base
                                    { $$ = criar_operacao(NO_MODULO, $1, $3); }
    | expressao_base E expressao_base
                                    { $$ = criar_operacao(NO_E, $1, $3); }
    | expressao_base OU expressao_base
                                    { $$ = criar_operacao(NO_OU, $1, $3); }
    | INTEIRO                       { $$ = criar_numero($1); }
    | REAL                          { $$ = criar_real($1); }
    | TEXTO                         { $$ = criar_texto($1); }
    | IDENTIFICADOR                 { $$ = criar_variavel($1); }
    | IDENTIFICADOR ABRE_PARENTESES args_chamada FECHA_PARENTESES
                                    { $$ = criar_chamada($1, $3); $$->linha = @1.first_line; }
    | ABRE_PARENTESES expressao FECHA_PARENTESES
                                    { $$ = $2; }

    | expressao_base MAIS expressao_base
                                    { $$ = criar_operacao(NO_SOMA, $1, $3); }
    | expressao_base MENOS expressao_base
                                    { $$ = criar_operacao(NO_SUBTRACAO, $1, $3); }
    | expressao_base MULT expressao_base
                                    { $$ = criar_operacao(NO_MULTIPLICACAO, $1, $3); }
    | expressao_base DIV expressao_base
                                    { $$ = criar_operacao(NO_DIVISAO, $1, $3); }

    | MENOS expressao_base %prec MENOS_UNARIO
                                    { $$ = criar_operacao(NO_SUBTRACAO, criar_numero(0), $2); }

    | expressao_base IGUAL expressao_base
                                    { $$ = criar_operacao(NO_IGUAL, $1, $3); }
    | expressao_base DIFERENTE expressao_base
                                    { $$ = criar_operacao(NO_DIFERENTE, $1, $3); }
    | expressao_base MAIOR_QUE expressao_base
                                    { $$ = criar_operacao(NO_MAIOR_QUE, $1, $3); }
    | expressao_base MENOR_QUE expressao_base
                                    { $$ = criar_operacao(NO_MENOR_QUE, $1, $3); }
    | expressao_base MAIOR_IGUAL expressao_base
                                    { $$ = criar_operacao(NO_MAIOR_IGUAL, $1, $3); }
    | expressao_base MENOR_IGUAL expressao_base
                                    { $$ = criar_operacao(NO_MENOR_IGUAL, $1, $3); }
    ;

%%