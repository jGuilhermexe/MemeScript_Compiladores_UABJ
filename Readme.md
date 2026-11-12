# MemeScript — Etapa 2 de Compiladores

Interpretador educacional em C, Flex e Bison. Versão intermediária para 12/11/2026, baseada no ZIP enviado pela equipe e na proposta Compilas.pdf. Equipe indicada na proposta: Gabriel Renan Silva Martins Guimarães, Jean Guilherme Silva de Arruda e Tomas Amorim da Silva.

## Rodar no CachyOS

Abra o terminal na pasta deste arquivo. Se precisar instalar as dependências, execute:

    sudo pacman -S --needed gcc make flex bison m4 python git

Depois compile e rode os testes:

    make
    make test

O resultado esperado é “33 testes passaram.” O interpretador usa C; Python serve somente para os testes. Para reconstruir, execute make clean e depois make.

## Comece pela soma de 1 a 5

    ./memescript exemplos/03_soma.ms

O arquivo declara contador e soma. Enquanto contador for menor ou igual a 5, soma recebe o valor anterior mais contador. A linha seguinte aumenta contador em 1. Quando ele chega a 6, o laço termina e o último habla imprime “Soma de 1 a 5: 15”.

### Siga uma linha pelo código

Use a linha `soma = soma + contador` para entender o projeto sem precisar ler todos os arquivos de uma vez.

1. **Flex · src/lexico.l.** As regras reconhecem identificador, atribuição, identificador, soma e identificador. Rode `./memescript --tokens exemplos/03_soma.ms`. Veja onde surgem INDENT e DEDENT. Quem escreve o programa digita espaços; o analisador produz esses tokens.
2. **Bison · src/sintatico.y.** A regra da adição chama criar_operacao. A regra da atribuição chama criar_atribuicao. O Bison junta os pedaços da árvore respeitando a precedência.
3. **AST · src/arvore.h e src/arvore.c.** Um No contém o tipo da operação e ponteiros. O nó ATRIBUICAO guarda o nome soma. Sua expressão é um nó SOMA com os filhos VARIAVEL soma e VARIAVEL contador. Confira com `./memescript --ast exemplos/03_soma.ms`.
4. **Tabela · src/tabela_simbolos.c.** Cada célula registra nome, valor e próxima variável. consultar procura um nome; guardar atualiza o valor. Os nós da AST guardam os nomes e a tabela guarda os valores que mudam a cada iteração.
5. **Execução · src/interpretador.c.** executar_bloco encontra a atribuição. avaliar visita seus filhos. Na terceira iteração, soma vale 3 e contador vale 3. O nó SOMA produz 6, e guardar registra esse resultado em soma.

## Outros exemplos para a apresentação

| Comando | O que observar |
|---|---|
| ./memescript exemplos/01_entrada.ms | Digite Gabriel. O programa imprime Ola, Gabriel, a aura 67 e o booleano tankavel. |
| ./memescript exemplos/02_decisao.ms | Com nota 7.5 imprime Mandou bem!. Em uma cópia, troque para 5.0 ou 4.9 e observe os outros dois ramos. |
| ./memescript exemplos/invalidos/lexico.ms | Linha 1, texto aberto. |
| ./memescript exemplos/invalidos/sintatico.ms | Linha 1, o cabeçalho da condição precisa de dois-pontos. |
| ./memescript exemplos/invalidos/semantico.ms | Linha 2, somaa não foi inicializada. |
| make test | Verifica os resultados dos três programas e casos de precedência, tipos, indentação, laços e erros. |

## Regras implementadas

A gramática formal desta etapa está em src/sintatico.y. Os padrões de tokens estão em src/lexico.l.

- Nomes distinguem maiúsculas e seguem [A-Za-z_][A-Za-z0-9_]*. Palavras reservadas não podem nomear variáveis. aura é um nome comum.
- lancei cria uma variável ou substitui seu valor. A atribuição simples exige um nome já inicializado. A tabela é global nesta etapa. Condicionais e laços compartilham as variáveis.
- Inteiros, reais, textos, booleanos e nulo são tipos distintos. Textos usam @@ nas duas pontas. fala_bunitinha() lê texto; habla aceita um ou mais valores separados por vírgula. Na saída, os valores são separados por um espaço.
- Da maior precedência para a menor: agrupamento/entrada; sinais unários; multiplicação, divisão e módulo; soma e subtração; comparações; nem; e_mais; ou_entao. Aritmética binária associa à esquerda, unários à direita. Comparações encadeadas são erro sintático.
- A soma também concatena dois textos. A divisão produz real. O módulo exige inteiros e seu resto não nulo tem o sinal do divisor. Igualdade permite dois textos, dois booleanos, dois números ou comparação com nulo. Comparação de ordem exige números. Os lógicos exigem booleanos e usam curto-circuito.
- se_liga e enquanto_tankar exigem condição booleana entre parênteses. O bloco começa após dois-pontos com quatro espaços adicionais. ou_senao admite nova condição; senao_fio cobre o caso restante. chega encerra o laço mais interno; passa_reto começa sua próxima iteração.
- Linhas vazias e comentários não alteram a indentação. !EXPOSED! inicia comentário de uma linha; !EXPOSED: e !!EXPOSED!! em linhas próprias delimitam um comentário de bloco.
- Antes de executar, o interpretador verifica se os desvios estão dentro de laços. Durante a execução, verifica nomes inicializados, tipos de valores e divisão por zero. O primeiro erro interrompe o processo e informa categoria e linha. O detalhe do erro de sintaxe vem do Bison e pode aparecer em inglês.
- A implementação foi testada com GCC em Linux. Usa inteiros de 64 bits no ambiente testado, double para reais e as funções de detecção de estouro do GCC para operações inteiras.

## Da versão intermediária à final

A base do colega já tinha os construtores da AST e as produções de declaração, atribuição, condicionais e enquanto. Esta revisão preserva esses componentes, completa o léxico e a execução, permite vários argumentos em habla e acrescenta os testes. Os exemplos 1, 2 e 3 da proposta fazem parte da demonstração.

O exemplo 4 permanece como objetivo da entrega de 26/11: listas, para_cada, funções receita, parâmetros, chamadas, devolve e escopo local. A aprovação da proposta e eventuais ajustes de escopo devem ser confirmados pela equipe com o professor. Para a entrega final, demonstrem o exemplo dobro com a lista de 1, 2 e 3 e acrescentem testes de lista vazia, aridade, retorno e inicialização local.

## Git e declaração de IA

O ZIP original não incluía o diretório .git. Esta cópia inclui histórico local da importação e da revisão. Para continuar no repositório real da equipe, copiem os arquivos de trabalho para o clone existente **sem copiar a pasta .git**. Confiram git diff, executem make test, façam o commit da revisão e disponibilizem o endereço do repositório ao professor. Não foi configurado remoto nesta cópia.

Declaração para a equipe revisar: o ChatGPT (Codex) foi utilizado para revisar a implementação inicial da equipe, completar o analisador léxico, a interpretação, a tabela de símbolos e os diagnósticos, ajustar a gramática e a compilação, elaborar e executar testes e preparar esta documentação. Os integrantes devem registrar os usos adicionais e conseguir explicar o funcionamento dos arquivos.
