
/*
 * arvore.h
 * Define a estrutura da Arvore Sintatica Abstrata (AST)
 * da linguagem MemeScript.
 *
 * Cada parte do programa sera representada por um no.
 *
 * Exemplos:
 *     23                  -> NO_NUMERO
 *     idade              -> NO_VARIAVEL
 *     idade + 10          -> NO_SOMA
 *     lancei idade = 23   -> NO_LANCEI
 *     habla idade         -> NO_HABLA
 */

#ifndef ARVORE_H
#define ARVORE_H

/* Tipos de nos que a linguagem MemeScript possui */
typedef enum {
    NO_LANCEI,          // Declaracao de variavel
    NO_HABLA,           // Exibicao de valor
    NO_NUMERO,          // Numero inteiro
    NO_REAL,            // Numero decimal
    NO_TEXTO,           // Texto
    NO_VARIAVEL,        // Nome de uma variavel

    NO_SOMA,            // +
    NO_SUBTRACAO,       // -
    NO_MULTIPLICACAO,   // *
    NO_DIVISAO,         // /

    NO_ATRIBUICAO,      // Atribuicao de valor

    NO_MAIOR_QUE,       // >
    NO_MENOR_QUE,       // <
    NO_IGUAL,           // ==
    NO_DIFERENTE,       // !=
    NO_MAIOR_IGUAL,     // >=
    NO_MENOR_IGUAL,     // <=

    NO_SE_LIGA,         // Condicional
    NO_ENQUANTO_TANKAR, // Repeticao
    NO_PASSA_RETO, NO_CHEGA, NO_ENTRADA, NO_BOOLEANO, NO_NULO,
    NO_MODULO, NO_E, NO_OU, NO_NEM, NO_POSITIVO

} TipoNo;


/* Um unico struct sera utilizado para todos os tipos de nos.
 * Cada tipo de no utiliza apenas os campos de que precisa. */
typedef struct No {
    TipoNo tipo;            // Identifica o tipo do no

    int linha;              // Linha do código-fonte
    long long valor;              // Guarda numeros inteiros
    double valor_real;      // Guarda numeros decimais

    char *nome;             // Guarda nomes de variaveis
    char *texto;            // Guarda textos

    struct No *expressao;   // Guarda uma expressao ou condicao
    struct No *proximo;     // Aponta para o proximo comando

    struct No *esquerda;    // Filho esquerdo de uma operacao
    struct No *direita;     // Filho direito de uma operacao

    struct No *bloco_entao; // Comandos executados se a condicao for verdadeira
    struct No *bloco_senao; // Comandos executados se for falsa
    struct No *bloco_while; // Comandos dentro de uma repeticao

} No;


/* Funcoes para criar os nos da arvore */
No *novo_no(TipoNo tipo);

No *criar_numero(long long valor);
No *criar_real(double valor);
No *criar_texto(char *texto);
No *criar_variavel(char *nome);

No *criar_operacao(TipoNo tipo, No *esquerda, No *direita);

No *criar_lancei(char *nome, No *expressao);
No *criar_atribuicao(char *nome, No *expressao);
No *criar_habla(No *expressao);

No *criar_se_liga(No *condicao, No *bloco_entao, No *bloco_senao);
No *criar_enquanto_tankar(No *condicao, No *bloco);


/* Adiciona um comando ao final da lista */
No *adicionar_comando(No *lista, No *comando);

/* Funcoes para imprimir a arvore */
void liberar_arvore(No *programa);
void imprimir_arvore(No *programa);

#endif
