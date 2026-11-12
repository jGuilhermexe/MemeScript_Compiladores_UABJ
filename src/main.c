#include <stdio.h>
#include <string.h>
#include "arvore.h"

extern No *raiz;
int yyparse(void);
void abrir_fonte(FILE *arquivo);
void mostrar_tokens(void);
void interpretar(No *programa);

int main(int argc, char **argv) {
    const char *modo = argc == 3 ? argv[1] : "";
    if ((argc != 2 && argc != 3) || (argc == 3 && strcmp(modo,"--ast") && strcmp(modo,"--tokens"))) {
        fprintf(stderr,"Uso: %s [--tokens|--ast] arquivo.ms\n",argv[0]);
        return 2;
    }
    FILE *arquivo = fopen(argv[argc-1],"r");
    if (!arquivo) { perror(argv[argc-1]); return 1; }
    abrir_fonte(arquivo);
    if (strcmp(modo,"--tokens") == 0) mostrar_tokens();
    else {
        if (yyparse() != 0) return 1;
        if (strcmp(modo,"--ast") == 0) imprimir_arvore(raiz);
        else interpretar(raiz);
        liberar_arvore(raiz);
    }
    fclose(arquivo);
    return 0;
}
