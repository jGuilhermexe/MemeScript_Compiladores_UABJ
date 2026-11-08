
# MemeScript

MemeScript é uma linguagem de programação interpretada,
desenvolvida como projeto da disciplina de Compiladores.

O projeto utiliza as ferramentas Flex e Bison para realizar
a análise léxica e sintática, construir uma Árvore Sintática
Abstrata (AST) e permitir a execução dos programas por meio
de um interpretador desenvolvido em C.

## Tecnologias utilizadas

- Linguagem C
- Flex: análise léxica
- Bison: análise sintática
- GCC: compilação
- Make: automação da compilação
- Git e GitHub: controle de versão

## Estrutura do projeto

```text
MemeScript/
├── src/
│   ├── main.c
│   ├── lexico.l
│   ├── sintatico.y
│   ├── arvore.h
│   ├── arvore.c
│   ├── interpretador.c
│   ├── tabela_simbolos.h
│   ├── tabela_simbolos.c
│   ├── erro.h
│   └── erro.c
├── testes/
│   ├── teste_basico.ms
│   └── teste_erro.ms
├── Makefile
├── Readme.md
└── .gitignore
```

## Como compilar

É necessário ter GCC, Flex, Bison e Make instalados.

No terminal, dentro da pasta principal do projeto, execute:

```bash
make
```

O comando gera o executável `memescript`.

## Como executar

Para executar um arquivo da linguagem MemeScript:

```bash
./memescript testes/teste_basico.ms
```

No ambiente Windows com MSYS2, o executável também pode
ser chamado como:

```bash
./memescript.exe testes/teste_basico.ms
```

## Como limpar os arquivos gerados

Para remover o executável e os arquivos temporários da compilação:

```bash
make clean
```

## Funcionalidades previstas

A linguagem MemeScript possui como objetivo implementar:

- Declaração de variáveis.
- Atribuição de valores.
- Operações aritméticas.
- Comparações.
- Comando de saída.
- Estruturas condicionais.
- Estruturas de repetição.
- Construção de uma Árvore Sintática Abstrata.
- Interpretação dos comandos.

A implementação está sendo desenvolvida gradualmente.
Nem todas as funcionalidades previstas na especificação
estão necessariamente disponíveis na versão atual.

## Testes

Os arquivos de teste ficam na pasta `testes/`.

- `teste_basico.ms`: exemplos de comandos válidos.
- `teste_erro.ms`: exemplos de comandos com erros para
  verificar o comportamento do analisador.

## Desenvolvimento

Projeto desenvolvido em equipe para a disciplina de
Compiladores, com utilização de Flex, Bison e linguagem C.
