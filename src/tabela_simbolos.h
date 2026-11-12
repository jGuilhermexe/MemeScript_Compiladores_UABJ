#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H
typedef enum { INTEIRO_MS, REAL_MS, TEXTO_MS, BOOLEANO_MS, NULO_MS } TipoValor;
typedef struct {
    TipoValor tipo;
    long long inteiro;
    double real;
    char *texto;
} Valor;
Valor consultar(const char *nome, int linha);
void guardar(const char *nome, Valor valor, int declaracao, int linha);
void limpar_tabela(void);
#endif
