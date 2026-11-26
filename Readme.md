# MemeScript — Projeto de Compiladores

MemeScript é uma linguagem educacional que usa expressões da internet brasileira para ensinar programação. A modalidade escolhida é interpretador, implementado em C com Flex e Bison.

Equipe formada por Gabriel Renan Silva Martins Guimarães, Jean Guilherme Silva de Arruda e Tomas Amorim da Silva.

## Entregas da disciplina

| Etapa | Material para a entrega |
|---|---|
| 1 · Proposta e especificação (22/10/2026) | A [proposta original](MemeScript_Propose.pdf) reúne equipe, finalidade, modalidade e regras. Os quatro programas de referência são [entrada e saída](exemplos/01_entrada.ms), [decisão](exemplos/02_decisao.ms), [soma em laço](exemplos/03_soma.ms) e [função sobre lista](exemplos/04_dobro.ms). A aprovação da proposta original deve ser confirmada pela equipe. |
| 2 · Versão intermediária (12/11/2026) | O commit `205d6ef` contém o léxico, a gramática, a AST, a tabela de variáveis e o interpretador de expressões, condições e laços, com 33 testes. |
| 3 · Entrega final (26/11/2026) | A versão atual acrescenta receitas, listas, escopo local e iteração. A [suíte de testes](testes/verificar.py) contém programas de entrada, saídas esperadas e diagnósticos esperados. |

## Rodar no CachyOS

Abra o terminal na pasta deste arquivo. Se precisar instalar as dependências, execute:

    sudo pacman -S --needed gcc make flex bison m4 python git

Depois compile e rode os testes:

    make
    make test

O resultado esperado é “65 testes passaram.” O interpretador usa C; Python serve somente para os testes. Para reconstruir, execute make clean e depois make.

## Comece pela soma de 1 a 5

    ./memescript exemplos/03_soma.ms

O arquivo declara contador e soma. Enquanto contador for menor ou igual a 5, soma recebe o valor anterior mais contador. A linha seguinte aumenta contador em 1. Quando ele chega a 6, o laço termina e o último habla imprime “Soma de 1 a 5: 15”.

### Siga uma linha pelo código

Use a linha `soma = soma + contador` para entender o projeto sem precisar ler todos os arquivos de uma vez.

1. **Flex · src/lexico.l.** As regras reconhecem identificador, atribuição, identificador, soma e identificador. Rode `./memescript --tokens exemplos/03_soma.ms`. Veja onde surgem INDENT e DEDENT. Quem escreve o programa digita espaços; o analisador produz esses tokens.
2. **Bison · src/sintatico.y.** A regra da adição chama criar_operacao. A regra da atribuição chama criar_no_nomeado. O Bison junta os pedaços da árvore respeitando a precedência.
3. **AST · src/arvore.h e src/arvore.c.** Um No contém o tipo da operação e ponteiros. O nó ATRIBUICAO guarda o nome soma. Sua expressão é um nó SOMA com os filhos VARIAVEL soma e VARIAVEL contador. Confira com `./memescript --ast exemplos/03_soma.ms`.
4. **Tabela · src/tabela_simbolos.c.** Cada célula registra nome, valor e próxima variável. consultar procura um nome; guardar atualiza o valor. Os nós da AST guardam os nomes e a tabela guarda os valores que mudam a cada iteração.
5. **Execução · src/interpretador.c.** executar_bloco encontra a atribuição. avaliar visita seus filhos. Na terceira iteração, soma vale 3 e contador vale 3. O nó SOMA produz 6, e guardar registra esse resultado em soma.

## Dez exemplos testados

Cada linha corresponde a um arquivo executado por `make test`. O teste compara toda a saída com o texto esperado. Para rodar um exemplo individualmente, use `./memescript exemplos/09_fatorial.ms`, trocando o nome do arquivo.

| Arquivo | Situação | Saída esperada em ordem |
|---|---|---|
| [01_entrada.ms](exemplos/01_entrada.ms) | Leitura do nome Gabriel | `Qual e o seu nome?`<br>`Ola, Gabriel`<br>`Nivel de aura: 67`<br>`Status: tankavel` |
| [02_decisao.ms](exemplos/02_decisao.ms) | Decisão com nota 7,5 | `Mandou bem!` |
| [03_soma.ms](exemplos/03_soma.ms) | Soma de 1 a 5 em um laço | `Soma de 1 a 5: 15` |
| [04_dobro.ms](exemplos/04_dobro.ms) | Chamada de função para cada item da lista | `Dobro de 1 = 2`<br>`Dobro de 2 = 4`<br>`Dobro de 3 = 6` |
| [04_teste.ms](exemplos/04_teste.ms) | Igualdade com `eh_engual` | `Hello World`<br>`A é 5` |
| [05_alot.ms](exemplos/05_alot.ms) | Laço com condição booleana e laço com contador | `Começou!`<br>`Primeira volta`<br>`Segunda volta`<br>`Terceira volta` |
| [06_hell.ms](exemplos/06_hell.ms) | Dobros seguidos de um texto iniciado com arroba | `2`<br>`4`<br>`6`<br>`@grenan` |
| [07_curto_circuito.ms](exemplos/07_curto_circuito.ms) | O primeiro operando decide o resultado de cada expressão lógica | `intankavel tankavel` |
| [08_escopo.ms](exemplos/08_escopo.ms) | A função usa uma variável local com o mesmo nome da global | `7 10` |
| [09_fatorial.ms](exemplos/09_fatorial.ms) | Uma função recursiva calcula o fatorial de 5 | `120` |

A suíte também verifica os diagnósticos dos exemplos [léxico](exemplos/invalidos/lexico.ms), [sintático](exemplos/invalidos/sintatico.ms) e [semântico](exemplos/invalidos/semantico.ms).

## Especificação da linguagem

A [gramática formal em Bison](src/sintatico.y) define as produções de programas, blocos, comandos e expressões. A [especificação Flex](src/lexico.l) define o reconhecimento dos tokens.

| Classe | Grafia ou regra |
|---|---|
| Nomes e literais | Identificador conforme a regra abaixo; inteiro `[0-9]+`; real `[0-9]+\.[0-9]+`; texto entre `@@` na mesma linha; `tankavel`, `intankavel`, `nulo`. |
| Comandos e palavras reservadas | `lancei habla fala_bunitinha se_liga ou_senao senao_fio enquanto_tankar chega passa_reto receita devolve para_cada em`. |
| Operadores | `+ - * / % == != > < >= <=`, os equivalentes `eh_engual` e `nada_a_ver`, e os lógicos `nem e_mais ou_entao`. |
| Pontuação | Atribuição com `=`, agrupamento e chamadas com parênteses, separação com vírgula, cabeçalho de bloco terminado por dois-pontos. Cada barra delimita uma lista; duas barras sem elementos formam a lista vazia. |
| Estrutura | Cada linha com código gera `NEWLINE`. A indentação gera `INDENT` e `DEDENT`. O fim do arquivo fecha os blocos abertos. |

## Regras semânticas

- Nomes distinguem maiúsculas e seguem [A-Za-z_][A-Za-z0-9_]*. Palavras reservadas não podem nomear variáveis. aura é um nome comum.
- lancei cria uma variável ou substitui seu valor. A atribuição simples exige um nome já inicializado. Condicionais e laços compartilham as variáveis do escopo atual.
- Inteiros, reais, textos, booleanos, listas e nulo são tipos distintos. Textos usam @@ nas duas pontas. fala_bunitinha() lê texto; habla aceita um ou mais valores separados por vírgula. Na saída, os valores são separados por um espaço.
- Da maior precedência para a menor: agrupamento/chamadas/entrada; sinais unários; multiplicação, divisão e módulo; soma e subtração; comparações; nem; e_mais; ou_entao. Aritmética binária associa à esquerda, unários à direita. Comparações encadeadas são erro sintático.
- A soma também concatena dois textos. A divisão produz real. O módulo exige inteiros e seu resto não nulo tem o sinal do divisor. Igualdade permite dois textos, dois booleanos, dois números ou comparação com nulo. Comparação de ordem exige números. Os lógicos exigem booleanos e usam curto-circuito.
- se_liga e enquanto_tankar exigem condição booleana entre parênteses. O bloco começa após dois-pontos com quatro espaços adicionais. ou_senao admite nova condição; senao_fio cobre o caso restante. chega encerra o laço mais interno; passa_reto começa sua próxima iteração.
- Linhas vazias e comentários não alteram a indentação. !EXPOSED! inicia comentário de uma linha; !EXPOSED: e !!EXPOSED!! em linhas próprias delimitam um comentário de bloco.
- Antes de executar, o interpretador verifica se os desvios estão dentro de laços. Durante a execução, verifica nomes inicializados, tipos de valores e divisão por zero. O primeiro erro interrompe o processo e informa categoria e linha. O detalhe do erro de sintaxe vem do Bison e pode aparecer em inglês.
- A implementação foi testada com GCC em Linux. Usa inteiros de 64 bits no ambiente testado, double para reais e as funções de detecção de estouro do GCC para operações inteiras.

- Uma receita é declarada no nível principal e fica disponível quando a execução passa pela declaração. Admite recursão e exige a quantidade declarada de argumentos. A chamada avalia os argumentos da esquerda para a direita e cria uma tabela local. Parâmetros têm nomes distintos. Replicar o nome de uma função já registrada em outra declaração de função ou de variável produz erro.
- Parâmetros e variáveis declaradas com `lancei` pertencem à chamada inteira. Ler um desses nomes antes de inicializá-lo produz erro. Os demais nomes podem ler valores globais. A atribuição dentro da receita exige uma variável local inicializada. Ao sair da chamada, o interpretador libera a tabela local.
- `devolve expressão` encerra a chamada mesmo dentro de um laço. `devolve` sozinho e o fim do corpo retornam `nulo`.
- `|1, 2, 3|` cria uma lista e `||` cria uma lista vazia. Listas podem combinar tipos. Cada barra é um token. Um exemplo com listas aninhadas é `| |1, 2|, || |`. Variáveis e parâmetros recebem valores por cópia. Listas permitem comparação com nulo.
- `para_cada item em valores:` avalia a lista uma vez e percorre seus elementos em ordem. Ao término, a variável mantém o último valor atribuído. Uma lista vazia pula o corpo e preserva o estado da variável.

## Demonstração coletiva

1. Uma pessoa apresenta a sintaxe de `03_soma.ms` e roda `./memescript --tokens exemplos/03_soma.ms`. Mostra onde começam e terminam os blocos.
2. Outra roda `./memescript --ast exemplos/03_soma.ms` e segue a atribuição de `soma` da árvore até a tabela de símbolos. Executa o programa para conferir o resultado 15.
3. A terceira executa `./memescript exemplos/06_hell.ms`. A função dobra 1, 2 e 3, imprimindo 2, 4 e 6 em linhas separadas. A última instrução imprime `@grenan`. Em seguida, demonstra os três exemplos inválidos da tabela e roda `make test`.

Para experimentar durante a apresentação, altere a entrada ou passe o caminho de qualquer novo arquivo `.ms` ao executável.

## Histórico e uso de IA

O histórico local pode ser consultado com `git log --oneline`. A entrega da disciplina inclui disponibilizar o endereço do repositório ao professor e apresentar o projeto com participação de todos os integrantes.

A documentação da equipe registra uso de ChatGPT/Codex e Gemini. Nesta revisão, o Codex auxiliou na simplificação da AST e da gramática, integração de funções e listas, revisão de escopos, execução dos testes e preparação da documentação. A equipe deve conferir a declaração de uso das ferramentas antes da submissão.
