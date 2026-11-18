#include "arvore.h"
#include "erro.h"
#include "tabela_simbolos.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { NORMAL, PARAR, PROXIMA_ITERACAO, RETORNAR } Fluxo;

static Valor valor_retorno;
static Valor avaliar(No *no);
static Fluxo executar_bloco(No *lista);

/* Expressoes produzem valores; comandos alteram variaveis ou controlam fluxo */
static Valor booleano(int verdadeiro) {
    return (Valor){.tipo=BOOLEANO_MS, .inteiro=verdadeiro};
}
static int exigir_booleano(Valor v, int linha) {
    if (v.tipo != BOOLEANO_MS) erro("semântico",linha,"esperado tankavel ou intankavel");
    return v.inteiro;
}
static int numerico(Valor v) { return v.tipo == INTEIRO_MS || v.tipo == REAL_MS; }
static long double numero(Valor v, int linha) {
    if (v.tipo == REAL_MS) return v.real;
    if (v.tipo == INTEIRO_MS) return v.inteiro;
    erro("semântico",linha,"operador exige valores numéricos");
    return 0;
}
static Valor valor_real(double x, int linha) {
    if (!isfinite(x)) erro("de execução",linha,"real fora do intervalo");
    return (Valor){.tipo=REAL_MS,.real=x};
}
static Valor operar(No *no, Valor a, Valor b) {
    int linha = no->linha;
    TipoNo op = no->tipo;
    if (op == NO_IGUAL || op == NO_DIFERENTE) {
        int iguais = 0;
        if (a.tipo == NULO_MS || b.tipo == NULO_MS) iguais = a.tipo == b.tipo;
        else if (numerico(a) && numerico(b)) iguais = numero(a,linha) == numero(b,linha);
        else if (a.tipo != b.tipo || a.tipo == LISTA_MS) erro("semântico",linha,"tipos incompatíveis na comparação");
        else if (a.tipo == TEXTO_MS) iguais = strcmp(a.texto,b.texto) == 0;
        else iguais = a.inteiro == b.inteiro;
        return booleano(op == NO_IGUAL ? iguais : !iguais);
    }
    if (op == NO_SOMA && a.tipo == TEXTO_MS && b.tipo == TEXTO_MS) {
        char *texto = alocar(strlen(a.texto)+strlen(b.texto)+1);
        strcpy(texto,a.texto);
        strcat(texto,b.texto);
        return (Valor){.tipo=TEXTO_MS,.texto=texto};
    }
    long double x = numero(a,linha), y = numero(b,linha);
    switch (op) {
        case NO_MAIOR_QUE: return booleano(x > y);
        case NO_MENOR_QUE: return booleano(x < y);
        case NO_MAIOR_IGUAL: return booleano(x >= y);
        case NO_MENOR_IGUAL: return booleano(x <= y);
        default: break;
    }
    if (op == NO_MODULO && (a.tipo != INTEIRO_MS || b.tipo != INTEIRO_MS))
        erro("semântico",linha,"% exige dois inteiros");
    if ((op == NO_DIVISAO || op == NO_MODULO) && y == 0)
        erro("de execução",linha,"num_da_pra_dividir_por_zero_isso_não_exist: divisor igual a zero");
    if (op == NO_DIVISAO) return valor_real(x/y,linha);
    if (op == NO_MODULO) {
        long long resto = 0;
        if (b.inteiro != -1) resto = a.inteiro % b.inteiro;
        if (resto && ((resto < 0) != (b.inteiro < 0))) resto += b.inteiro;
        return (Valor){.tipo=INTEIRO_MS,.inteiro=resto};
    }
    if (a.tipo == REAL_MS || b.tipo == REAL_MS) {
        if (op == NO_SOMA) return valor_real(x+y,linha);
        if (op == NO_SUBTRACAO) return valor_real(x-y,linha);
        return valor_real(x*y,linha);
    }
    long long resultado;
    int excedeu;
    if (op == NO_SOMA) excedeu = __builtin_add_overflow(a.inteiro,b.inteiro,&resultado);
    else if (op == NO_SUBTRACAO) excedeu = __builtin_sub_overflow(a.inteiro,b.inteiro,&resultado);
    else excedeu = __builtin_mul_overflow(a.inteiro,b.inteiro,&resultado);
    if (excedeu) erro("de execução",linha,"inteiro fora do intervalo");
    return (Valor){.tipo=INTEIRO_MS,.inteiro=resultado};
}
static Valor chamar_funcao(No *no) {
    No *funcao = buscar_funcao(no->nome,no->linha);
    int qtd_params = 0;
    for (No *p = funcao->parametros; p; p = p->proximo) qtd_params++;
    int qtd_args = 0;
    for (No *a = no->expressao; a; a = a->proximo) qtd_args++;
    if (qtd_params != qtd_args)
        erro("semântico",no->linha,"quantidade incorreta de argumentos");

    Valor *args = qtd_args > 0 ? alocar(qtd_args * sizeof(Valor)) : NULL;
    int i = 0;
    for (No *a = no->expressao; a; a = a->proximo) {
        args[i++] = avaliar(a);
    }
    entrar_escopo(funcao,args,qtd_args,no->linha);
    free(args);

    Valor retorno_anterior = valor_retorno;
    valor_retorno = (Valor){.tipo=NULO_MS};
    Fluxo fluxo = executar_bloco(funcao->bloco_entao);
    Valor resultado = (fluxo == RETORNAR) ? valor_retorno : (Valor){.tipo=NULO_MS};
    valor_retorno = retorno_anterior;

    sair_escopo();
    return resultado;
}
static Valor avaliar(No *no) {
    switch (no->tipo) {
        case NO_NUMERO: return (Valor){.tipo=INTEIRO_MS,.inteiro=no->valor};
        case NO_REAL: return valor_real(no->valor_real,no->linha);
        case NO_BOOLEANO: return booleano(no->valor);
        case NO_NULO: return (Valor){.tipo=NULO_MS};
        case NO_TEXTO: return (Valor){.tipo=TEXTO_MS,.texto=copiar_texto(no->texto)};
        case NO_VARIAVEL: return consultar(no->nome,no->linha);
        case NO_CHAMADA: return chamar_funcao(no);
        case NO_LISTA: {
            int qtd = 0;
            for (No *item = no->expressao; item; item = item->proximo) qtd++;
            Valor *itens = qtd > 0 ? alocar(qtd * sizeof(Valor)) : NULL;
            int i = 0;
            for (No *item = no->expressao; item; item = item->proximo) {
                itens[i++] = avaliar(item);
            }
            return (Valor){.tipo=LISTA_MS,.itens=itens,.tamanho=qtd};
        }
        case NO_ENTRADA: {
            char *texto = NULL;
            size_t tamanho = 0;
            if (getline(&texto,&tamanho,stdin) < 0) erro("de execução",no->linha,"entrada indisponível");
            texto[strcspn(texto,"\r\n")] = 0;
            return (Valor){.tipo=TEXTO_MS,.texto=texto};
        }
        default: break;
    }
    Valor a = avaliar(no->esquerda);
    if (no->tipo == NO_POSITIVO) { numero(a,no->linha); return a; }
    if (no->tipo == NO_NEM) return booleano(!exigir_booleano(a,no->linha));
    if (no->tipo == NO_E || no->tipo == NO_OU) {
        int primeiro = exigir_booleano(a,no->linha);
        if (no->tipo == NO_E && !primeiro) return booleano(0);
        if (no->tipo == NO_OU && primeiro) return booleano(1);
        return booleano(exigir_booleano(avaliar(no->direita),no->linha));
    }
    Valor b = avaliar(no->direita);
    Valor resultado = operar(no,a,b);
    liberar_valor(&a);
    liberar_valor(&b);
    return resultado;
}
static void exibir(Valor valor) {
    switch (valor.tipo) {
        case INTEIRO_MS: printf("%lld",valor.inteiro); break;
        case REAL_MS: printf("%.15g",valor.real); break;
        case TEXTO_MS: printf("%s",valor.texto); break;
        case BOOLEANO_MS: printf("%s",valor.inteiro ? "tankavel" : "intankavel"); break;
        case NULO_MS: printf("nulo"); break;
        case LISTA_MS:
            putchar('|');
            for (int i = 0; i < valor.tamanho; i++) {
                if (i > 0) printf(", ");
                exibir(valor.itens[i]);
            }
            putchar('|');
            break;
    }
}
static void validar_desvios(No *lista, int em_laco, int em_funcao, int em_bloco) {
    for (No *no = lista; no; no = no->proximo) {
        if (!em_laco && no->tipo == NO_CHEGA) erro("semântico",no->linha,"chega exige um laço");
        if (!em_laco && no->tipo == NO_PASSA_RETO) erro("semântico",no->linha,"passa_reto exige um laço");
        if (!em_funcao && no->tipo == NO_DEVOLVE) erro("semântico",no->linha,"devolve exige uma função");
        if (em_bloco && no->tipo == NO_RECEITA) erro("semântico",no->linha,"receita só pode ser declarada no nível principal");
        if (no->tipo == NO_RECEITA) {
            validar_desvios(no->bloco_entao,0,1,1);
        } else if (no->tipo == NO_ENQUANTO_TANKAR || no->tipo == NO_PARA_CADA) {
            validar_desvios(no->bloco_while,1,em_funcao,1);
        } else {
            validar_desvios(no->bloco_entao,em_laco,em_funcao,1);
            validar_desvios(no->bloco_senao,em_laco,em_funcao,1);
        }
    }
}

static Fluxo executar_bloco(No *lista) {
    for (No *no = lista; no; no = no->proximo) {
        switch (no->tipo) {
            case NO_RECEITA:
                registrar_funcao(no);
                break;
            case NO_LANCEI:
            case NO_ATRIBUICAO:
                guardar(no->nome,avaliar(no->expressao),no->tipo == NO_LANCEI,no->linha);
                break;
            case NO_CHAMADA: {
                Valor res = chamar_funcao(no);
                liberar_valor(&res);
                break;
            }
            case NO_DEVOLVE:
                valor_retorno = no->expressao ? avaliar(no->expressao) : (Valor){.tipo=NULO_MS};
                return RETORNAR;
            case NO_ENTRADA: {
                Valor lido = avaliar(no);
                liberar_valor(&lido);
                break;
            }
            case NO_HABLA:
                for (No *item = no->expressao; item; item = item->proximo) {
                    Valor valor = avaliar(item);
                    if (item != no->expressao) putchar(' ');
                    exibir(valor);
                    liberar_valor(&valor);
                }
                putchar('\n');
                break;
            case NO_SE_LIGA: {
                int condicao = exigir_booleano(avaliar(no->expressao),no->linha);
                Fluxo fluxo = executar_bloco(condicao ? no->bloco_entao : no->bloco_senao);
                if (fluxo != NORMAL) return fluxo;
                break;
            }
            case NO_ENQUANTO_TANKAR:
                while (exigir_booleano(avaliar(no->expressao),no->linha)) {
                    Fluxo fluxo = executar_bloco(no->bloco_while);
                    if (fluxo == PARAR) break;
                    if (fluxo == RETORNAR) return RETORNAR;
                }
                break;
            case NO_PARA_CADA: {
                Valor lista = avaliar(no->expressao);
                if (lista.tipo != LISTA_MS) {
                    liberar_valor(&lista);
                    erro("semântico",no->linha,"para_cada exige uma lista");
                }
                for (int i = 0; i < lista.tamanho; i++) {
                    guardar(no->nome,copiar_valor(lista.itens[i]),1,no->linha);
                    Fluxo fluxo = executar_bloco(no->bloco_while);
                    if (fluxo == PARAR) break;
                    if (fluxo == RETORNAR) {
                        liberar_valor(&lista);
                        return RETORNAR;
                    }
                }
                liberar_valor(&lista);
                break;
            }
            case NO_CHEGA: return PARAR;
            case NO_PASSA_RETO: return PROXIMA_ITERACAO;
            default: erro("interno",no->linha,"comando desconhecido");
        }
    }
    return NORMAL;
}
void interpretar(No *raiz) {
    validar_desvios(raiz,0,0,0);
    executar_bloco(raiz);
    limpar_tabela();
}