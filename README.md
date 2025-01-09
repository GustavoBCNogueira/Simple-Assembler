# trabalho1_SB
## Alunos
  - Akin Sangiácomo Bazila - 221002002
  - Gustavo Borges Caldas Nogueira - 221030016

## SO e padrão de linguagem utilizados
  - Linux - Ubuntu
  - C++17

## Compilação dos programas
  - As instruções de compilação dos arquivos são direcionadas para SOs Unix/Linux
  - Para compilar os programas usando as linhas de comando abaixo, é nessessário estar localizado na pasta **_src/_**
  - Para compilar o programa **_montador.cpp_**:
> ```g++ montador.cpp -std=c++17 -o montador```
  - Para executar o montador, passando um arquivo .asm como entrada e obtendo o seu código pré-processado:
> ```./montador <arquivo.asm>```
  - Para executar o montador, passando um arquivo .pre como entrada e obtendo o seu código objeto:
> ```./montador <arquivo.pre>```
  - Para compilar o programa **_ligador.cpp_**:
> ```g++ ligador.cpp -std=c++17 -o ligador```
  - Para executar o ligador com dois arquivos objeto:
> ```./ligador <programa1.obj> <programa2.obj>```
