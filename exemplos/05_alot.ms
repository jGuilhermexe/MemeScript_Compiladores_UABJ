lancei ativo = tankavel

!EXPOSED! while controlado por um booleano
enquanto_tankar (ativo):
    habla @@Começou!@@
    ativo = intankavel

!EXPOSED! equivalente a um for que conta de 1 até 3
lancei i = 1
enquanto_tankar (i <= 3):
    se_liga (i == 1):
        habla @@Primeira volta@@
    ou_senao (i == 2):
        habla @@Segunda volta@@
    senao_fio:
        habla @@Terceira volta@@
    i = i + 1
