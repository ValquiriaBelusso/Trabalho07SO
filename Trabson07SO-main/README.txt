IMPLEMENTAÇÃO FAT 16

SISTEMAS OPERACIONAIS
UNIOESTE - UNIVERSIDADE ESTADUAL DO OESTE DO PARANÁ

DESENVOLVIDO POR: RODRIGO CAMPOS, VALQUÍRIA BELUSSO
PROFESSOR: MARCIO OYAMADA

-----------------------------------------------------------------------

Implementação realizada em C, em ambiente Linux com compilador GCC.

Para executar o programa no terminal, navegue até o diretório onde se
encontra o arquivo main.c e digite os seguintes
comandos:

    gcc main.c -o out
    ./out

Após executar, digite o nome do arquivo .img que deseja ser analizado.

Por exemplo, fat16_4sectorpercluster.img (que está em anexo).

É necessário que os arquivos .img a serem lidos pelo programa estejam 
localizados no mesmo diretório do arquivo main.c.

Ao ler o arquivo, digite o nome do arquivo a ser lido na linha "Open >",
em letras maiúsculas.

-----------------------------------------------------------------------
