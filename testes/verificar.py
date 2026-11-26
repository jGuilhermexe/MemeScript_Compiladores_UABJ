"""Executa o binário como um usuário. Cada caso roda em um processo novo."""
from pathlib import Path
import subprocess
from tempfile import TemporaryDirectory

ROOT = Path(__file__).resolve().parents[1]
count = 0


def check(path, *, expected=None, error=None, stdin="", mode=None, contains=()):
    global count
    cmd = [ROOT / "memescript"]
    if mode:
        cmd.append(mode)
    result = subprocess.run([*cmd, path], input=stdin, capture_output=True,
                            encoding="utf-8", timeout=5)
    if error:
        assert result.returncode == 1 and error in result.stderr, (path, result)
    else:
        assert result.returncode == 0 and not result.stderr, (path, result)
        if expected is not None:
            assert result.stdout == expected, (path, result.stdout, expected)
        for marker in contains:
            assert marker in result.stdout, (marker, result.stdout)
    count += 1


examples = ROOT / "exemplos"
check(examples / "01_entrada.ms", stdin="Gabriel\n",
      expected="Qual e o seu nome?\nOla, Gabriel\nNivel de aura: 67\nStatus: tankavel\n")
decision = examples / "02_decisao.ms"
check(decision, expected="Mandou bem!\n")
loop = examples / "03_soma.ms"
check(loop, expected="Soma de 1 a 5: 15\n")
check(examples / "04_dobro.ms", expected="Dobro de 1 = 2\nDobro de 2 = 4\nDobro de 3 = 6\n")
check(examples / "04_teste.ms", expected="Hello World\nA é 5\n")
check(examples / "05_alot.ms", expected="Começou!\nPrimeira volta\nSegunda volta\nTerceira volta\n")
check(examples / "06_hell.ms", expected="2\n4\n6\n@grenan\n")
check(examples / "07_curto_circuito.ms", expected='intankavel tankavel\n')
check(examples / "08_escopo.ms", expected='7 10\n')
check(examples / "09_fatorial.ms", expected='120\n')

check(loop, mode="--tokens", contains=["INDENT", "DEDENT", "EOF"])
check(loop, mode="--ast", contains=["ENQUANTO_TANKAR", "MENOR_IGUAL", "ATRIBUICAO soma"])
for name, diagnostic in [("lexico", "léxico na linha 1"), ("sintatico", "sintático na linha 1"),
                         ("semantico", "semântico na linha 2: kd_a_variavel?")]:
    check(examples / "invalidos" / (name + ".ms"), error=diagnostic)

valid = [
    ("habla ||1, 2||, | |", "||1, 2|| ||\n"),
    ("lancei item=7\nreceita ver():\n    para_cada item em ||:\n        habla item\n    devolve item\nhabla ver()", "7\n"),
    ("lancei itens=|1,2|\npara_cada x em itens:\n    itens=||\n    habla x", "1\n2\n"),
    ("receita par(a,b):\n    devolve |a,b|\nhabla par(1+2,4)", "|3, 4|\n"),
    ("habla @@a@b@@, @@@@, nulo == ||", "a@b  intankavel\n"),
    ("lancei lancei_total=1\nlancei Lancei_total=2\nhabla lancei_total, Lancei_total", "1 2\n"),
    ("lancei x=8\nreceita global():\n    devolve x\nreceita local(x):\n    devolve global()\nhabla local(5)", "8\n"),
    ("receita primeiro(xs):\n    enquanto_tankar (tankavel):\n        para_cada x em xs:\n            devolve x\nhabla primeiro(| |1, 2| |)", "|1, 2|\n"),
    ("habla 2+3*4, (2+3)*4, 10-3-2, -7%3, 7%-3, 5/2", "14 20 5 2 -2 2.5\n"),
    ("habla nem 2 eh_engual 2, nulo nada_a_ver 0", "intankavel tankavel\n"),
    ("lancei x=1\nlancei x=@@a@@\nx=x+@@b@@\nhabla x", "ab\n"),
    ("se_liga (tankavel):\r\n    !EXPOSED! oi\r\n    habla @@ação!EXPOSED!@@", "ação!EXPOSED!\n"),
    (" !EXPOSED:\n!@\n  !!EXPOSED!!\nhabla @@@@ !EXPOSED! fim", "\n"),
    ("lancei x=0\nenquanto_tankar (tankavel):\n    x=x+1\n    se_liga (x<3):\n        passa_reto\n    chega\nhabla x", "3\n"),
    (decision.read_text().replace("7.5","5.0"), "Bora revisar!\n"),
    (decision.read_text().replace("7.5","4.9"), "Precisa estudar mais.\n"),
    ("", ""),
    ("lancei vazia = ||\nhabla vazia, |1, @@ok@@, tankavel|\npara_cada x em vazia:\n    habla x\nhabla @@fim@@",
     "|| |1, ok, tankavel|\nfim\n"),
    ("lancei ultimo = 99\npara_cada ultimo em ||:\n    habla ultimo\npara_cada ultimo em |10, 20, 30|:\n    se_liga (ultimo == 20):\n        passa_reto\n    habla ultimo\nhabla ultimo",
     "10\n30\n30\n"),
    ("receita sem_retorno():\n    habla @@oi@@\nreceita vazio():\n    devolve\nhabla sem_retorno(), vazio()",
     "oi\nnulo nulo\n"),
]
invalid = [
    ("habla 1 == nem tankavel", "sintático na linha 1"),
    ("habla f()\nreceita f():\n    devolve 1", "função não definida"),
    ("receita f():\n    devolve 1\nlancei f=0", "nome já utilizado por função"),
    ("habla 7 % 0", "divisor igual a zero"),
    ("habla fala_bunitinha(1)", "sintático na linha 1"),
    ("habla 1 < 2 < 3", "sintático na linha 1"),
    ("se_liga (tankavel):\n    !EXPOSED! só comentário", "sintático"),
    ("se_liga (tankavel):\n\thabla 1", "tabulação"),
    ("!EXPOSED:\ntexto", "comentário de bloco"),
    ("se_liga (tankavel):\n   habla 1", "indentação inválida"),
    ("habla @", "caractere inválido"),
    ("x=1", "kd_a_variavel?"),
    ("se_liga (intankavel):\n    lancei x=1\nhabla x", "kd_a_variavel?"),
    ("se_liga (2):\n    habla 1", "esperado tankavel"),
    ("habla @@a@@+1", "numéricos"),
    ("habla tankavel+1", "numéricos"),
    ("habla 2.5%2", "dois inteiros"),
    ("habla 1/0", "num_da_pra_dividir_por_zero_isso_não_exist"),
    ("se_liga (intankavel):\n    chega", "chega exige um laço"),
    ("lancei enquanto_tankar=0", "sintático"),
    ("devolve 1", "devolve exige uma função"),
    ("para_cada x em 10:\n    habla x", "para_cada exige uma lista"),
    ("habla nao_existe(1)", "função não definida"),
    ("receita f(a, b):\n    devolve a\nhabla f(1)", "quantidade incorreta de argumentos"),
    ("receita f(a, a):\n    devolve a", "parâmetros duplicados"),
    ("lancei x = 10\nreceita f():\n    habla x\n    lancei x = 20\nf()", "kd_a_variavel?"),
    ("lancei x = 10\nreceita f():\n    x = 20\nf()", "kd_a_variavel?"),
    ("habla |1| == |1|", "tipos incompatíveis na comparação"),
    ("se_liga (tankavel):\n    receita f():\n        devolve 1", "sintático na linha 2"),
]
with TemporaryDirectory() as directory:
    path = Path(directory) / "teste.ms"
    path.write_text("habla ||")
    check(path, mode="--tokens", expected="1  habla\n1  |\n1  |\n1  NEWLINE\nEOF\n")
    for source, expected in valid:
        path.write_bytes(source.encode())
        check(path, expected=expected)
    for source, error in invalid:
        path.write_bytes(source.encode())
        check(path, error=error)
print(f"{count} testes passaram.")
