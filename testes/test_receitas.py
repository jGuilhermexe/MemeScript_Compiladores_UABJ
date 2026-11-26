"""Na raiz do projeto, rode make e depois python3 testes/test_receitas.py -v."""

import subprocess
import unittest
from pathlib import Path
from tempfile import TemporaryDirectory
from textwrap import dedent

INTERPRETADOR = Path(__file__).resolve().parents[1] / "memescript"


def executar(programa, entrada=""):
    with TemporaryDirectory() as pasta:
        arquivo = Path(pasta) / "programa.ms"
        arquivo.write_text(dedent(programa).strip() + "\n", encoding="utf-8")
        return subprocess.run(
            [INTERPRETADOR, arquivo], input=entrada, encoding="utf-8",
            capture_output=True, timeout=5,
        )


class TestReceitas(unittest.TestCase):
    def conferir_saida(self, programa, esperado, entrada=""):
        resultado = executar(programa, entrada)
        self.assertEqual(resultado.returncode, 0, resultado.stderr)
        self.assertEqual(resultado.stderr, "")
        self.assertEqual(resultado.stdout, esperado)

    def test_argumentos_leem_a_entrada_da_esquerda_para_a_direita(self):
        self.conferir_saida("""
            receita juntar(a, b):
                devolve a + b
            habla juntar(fala_bunitinha(), fala_bunitinha())
        """, "MemeScript\n", entrada="Meme\nScript\n")

    def test_recursao_preserva_o_texto_local_de_cada_chamada(self):
        self.conferir_saida("""
            receita repetir(n, texto):
                se_liga (n == 0):
                    devolve texto
                lancei marca = texto
                devolve repetir(n - 1, texto + @@x@@) + marca
            habla repetir(2, @@a@@)
        """, "axxaxa\n")

    def test_lista_retornada_sobrevive_ao_fim_da_receita(self):
        self.conferir_saida("""
            receita montar(nome):
                lancei dupla = |nome, nome + @@!@@|
                devolve |dupla, nulo|
            lancei primeira = montar(@@Ana@@)
            lancei segunda = montar(@@Bia@@)
            habla primeira, segunda
        """, "||Ana, Ana!|, nulo| ||Bia, Bia!|, nulo|\n")

    def test_chega_encerra_apenas_o_laco_interno(self):
        self.conferir_saida("""
            receita contar(grupos):
                lancei total = 0
                para_cada grupo em grupos:
                    para_cada item em grupo:
                        se_liga (item == 0):
                            chega
                        total = total + 1
                devolve total
            habla contar(| |1, 0, 9|, |2, 3| |)
        """, "3\n")

    def test_chamadas_sucessivas_comecam_com_locais_novos(self):
        resultado = executar("""
            receita ler(criar):
                se_liga (criar):
                    lancei segredo = 42
                devolve segredo
            habla ler(tankavel)
            habla ler(intankavel)
        """)
        self.assertEqual(resultado.returncode, 1, resultado.stderr)
        self.assertEqual(resultado.stdout, "42\n")
        self.assertIn("semântico na linha 4: kd_a_variavel?", resultado.stderr)
        self.assertIn("segredo", resultado.stderr)


if __name__ == "__main__":
    unittest.main()
