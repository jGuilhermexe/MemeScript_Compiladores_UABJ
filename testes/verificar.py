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
check(loop, mode="--tokens", contains=["INDENT", "DEDENT", "EOF"])
check(loop, mode="--ast", contains=["ENQUANTO_TANKAR", "MENOR_IGUAL", "ATRIBUICAO soma"])
for name, diagnostic in [("lexico", "léxico na linha 1"), ("sintatico", "sintático na linha 1"),
                         ("semantico", "semântico na linha 2: kd_a_variavel?")]:
    check(examples / "invalidos" / (name + ".ms"), error=diagnostic)

valid = [
    ("habla 2+3*4, (2+3)*4, 10-3-2, -7%3, 7%-3, 5/2", "14 20 5 2 -2 2.5\n"),
    ("habla intankavel e_mais falta, tankavel ou_entao falta", "intankavel tankavel\n"),
    ("habla nem 2 eh_engual 2, nulo nada_a_ver 0", "intankavel tankavel\n"),
    ("lancei x=1\nlancei x=@@a@@\nx=x+@@b@@\nhabla x", "ab\n"),
    ("se_liga (tankavel):\r\n    !EXPOSED! oi\r\n    habla @@ação!EXPOSED!@@", "ação!EXPOSED!\n"),
    (" !EXPOSED:\n!@\n  !!EXPOSED!!\nhabla @@@@ !EXPOSED! fim", "\n"),
    ("lancei x=0\nenquanto_tankar (tankavel):\n    x=x+1\n    se_liga (x<3):\n        passa_reto\n    chega\nhabla x", "3\n"),
    (decision.read_text().replace("7.5","5.0"), "Bora revisar!\n"),
    (decision.read_text().replace("7.5","4.9"), "Precisa estudar mais.\n"),
    ("", ""),
]
invalid = [
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
]
with TemporaryDirectory() as directory:
    path = Path(directory) / "teste.ms"
    for source, expected in valid:
        path.write_bytes(source.encode())
        check(path, expected=expected)
    for source, error in invalid:
        path.write_bytes(source.encode())
        check(path, error=error)
print(f"{count} testes passaram.")
