# Ordem: Bison gera o parser e o arquivo .h com os tokens
# -> Flex gera o analisador lexico
# -> GCC compila todos os arquivos

memescript: src/sintatico.y src/lexico.l src/arvore.c src/interpretador.c src/main.c
	bison -d -o sintatico.tab.c src/sintatico.y
	flex -o lex.yy.c src/lexico.l
	gcc -Wall -Wextra -o memescript sintatico.tab.c lex.yy.c src/arvore.c src/interpretador.c src/main.c

# Remove os arquivos gerados durante a compilacao
clean:
	rm -f memescript memescript.exe sintatico.tab.c sintatico.tab.h lex.yy.c
