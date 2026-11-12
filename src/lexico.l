%option noyywrap nounput noinput nodefault never-interactive
%{
#include "arvore.h"
#include "sintatico.tab.h"
#include "erro.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#define YY_DECL static int ler_token(void)
int linha_atual = 0;
static FILE *fonte;
%}
%%
"lancei"        return LANCEI;
"habla"         return HABLA;
"se_liga"       return SE_LIGA;
"ou_senao"      return OU_SENAO;
"senao_fio"     return SENAO_FIO;
"enquanto_tankar" return ENQUANTO_TANKAR;
"fala_bunitinha" return ENTRADA;
"tankavel"      return VERDADE;
"intankavel"    return FALSO;
"nulo"          return NULO;
"nem"           return NEM;
"e_mais"        return E;
"ou_entao"      return OU;
"chega"         return CHEGA;
"passa_reto"    return PASSA;
"receita"|"devolve"|"para_cada"|"em" return RESERVADA;
"=="|"eh_engual" return IGUAL;
"!="|"nada_a_ver" return DIFERENTE;
">="            return MAIOR_IGUAL;
"<="            return MENOR_IGUAL;
">"             return MAIOR_QUE;
"<"             return MENOR_QUE;
"="             return ATRIBUICAO;
"+"             return MAIS;
"-"             return MENOS;
"*"             return MULT;
"/"             return DIV;
"%"             return MODULO;
":"             return DOIS_PONTOS;
","             return VIRGULA;
"("             return ABRE_PARENTESES;
")"             return FECHA_PARENTESES;
"|"             return RESERVADA;
[0-9]+          { errno=0; yylval.inteiro=strtoll(yytext,NULL,10); if(errno) erro("de execução",linha_atual,"inteiro fora do intervalo"); return INTEIRO; }
[0-9]+\.[0-9]+  { errno=0; yylval.real=strtod(yytext,NULL); if(errno) erro("de execução",linha_atual,"real fora do intervalo"); return REAL; }
[A-Za-z_][A-Za-z0-9_]* { yylval.texto=copiar_texto(yytext); return IDENTIFICADOR; }
"@@"([^@\n]|"@"[^@\n])*"@@" { yytext[yyleng-2]=0; yylval.texto=copiar_texto(yytext+2); yytext[yyleng-2]='@'; return TEXTO; }
"@@"            erro("léxico",linha_atual,"texto sem @@ de fechamento");
"!EXPOSED!".*   ;
[ \t]+          ;
(.|\n)          erro("léxico",linha_atual,"caractere inválido");
%%
static YY_BUFFER_STATE buffer;
static int indentacao, recuos, avanco;
static char *linha;
static size_t capacidade;

void abrir_fonte(FILE *arquivo) { fonte = arquivo; }

/* Cada linha com código gera NEWLINE. Só essas linhas mudam a indentação. */
static void preparar_linha(void) {
    static int inicio_comentario;
    while (getline(&linha,&capacidade,fonte) >= 0) {
        linha_atual++;
        size_t fim = strlen(linha);
        while (fim && strchr(" \t\n\r",linha[fim-1])) linha[--fim] = 0;
        size_t espacos = strspn(linha," \t");
        char *codigo = linha + espacos;
        if (inicio_comentario) {
            if (strcmp(codigo,"!!EXPOSED!!") == 0) inicio_comentario = 0;
            continue;
        }
        if (strcmp(codigo,"!EXPOSED:") == 0) { inicio_comentario = linha_atual; continue; }
        if (!*codigo || strncmp(codigo,"!EXPOSED!",9) == 0) continue;
        if (memchr(linha,'\t',espacos)) erro("léxico",linha_atual,"tabulação na indentação");
        int atual = (int)espacos;
        if (atual % 4 || atual > indentacao + 4) erro("léxico",linha_atual,"indentação inválida");
        avanco = atual > indentacao;
        recuos = atual < indentacao ? (indentacao-atual)/4 : 0;
        indentacao = atual;
        buffer = yy_scan_string(codigo);
        return;
    }
    if (ferror(fonte)) erro("de execução",linha_atual,"falha de leitura do arquivo");
    if (inicio_comentario) erro("léxico",inicio_comentario,"comentário de bloco sem fechamento");
    recuos = indentacao / 4;
    indentacao = 0;
}

int yylex(void) {
    if (!buffer && !recuos) preparar_linha();
    yylloc.first_line = yylloc.last_line = linha_atual;
    if (avanco) { avanco = 0; return INDENT; }
    if (recuos) { recuos--; return DEDENT; }
    if (!buffer) return 0;
    int token = ler_token();
    if (token) return token;
    yy_delete_buffer(buffer);
    buffer = NULL;
    return NEWLINE;
}
void mostrar_tokens(void) {
    int token;
    while ((token = yylex()) != 0) {
        const char *nome = yytext;
        if (token == INDENT) nome = "INDENT";
        if (token == DEDENT) nome = "DEDENT";
        if (token == NEWLINE) nome = "NEWLINE";
        if (token == IDENTIFICADOR) printf("%d  ID  %s\n", linha_atual, nome);
        else if (token == INTEIRO) printf("%d  INTEIRO  %s\n", linha_atual, nome);
        else if (token == REAL) printf("%d  REAL  %s\n", linha_atual, nome);
        else if (token == TEXTO) printf("%d  TEXTO  %s\n", linha_atual, nome);
        else printf("%d  %s\n", linha_atual, nome);
        if (token == IDENTIFICADOR || token == TEXTO) free(yylval.texto);
    }
    puts("EOF");
}
