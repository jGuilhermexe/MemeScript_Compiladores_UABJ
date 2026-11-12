CC = gcc
CFLAGS = -std=gnu11 -Wall -Wextra -Isrc -I.
FONTES = src/main.c src/arvore.c src/interpretador.c src/tabela_simbolos.c src/erro.c

memescript: src/sintatico.y src/lexico.l $(FONTES) $(wildcard src/*.h)
	bison -Wall -Werror=conflicts-sr -Werror=conflicts-rr -d -o sintatico.tab.c src/sintatico.y
	flex -o lex.yy.c src/lexico.l
	$(CC) $(CFLAGS) -o $@ sintatico.tab.c lex.yy.c $(FONTES)

test: memescript
	python3 testes/verificar.py

clean:
	rm -f memescript memescript.exe sintatico.tab.c sintatico.tab.h lex.yy.c
.PHONY: clean test
