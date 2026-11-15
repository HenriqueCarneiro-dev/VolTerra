# VolTerra

VolTerra é um jogo de gerenciamento e estratégia em C, desenvolvido com a biblioteca raylib. O objetivo é manter a rede elétrica em funcionamento, equilibrando a geração e o consumo das regiões para evitar blackouts, sobrecargas e falhas na operação da usina.

O projeto foi pensado como uma simulação de rede nacional, com progressão por fases, eventos dinâmicos, minigames de emergência e ranking de pontuação.

## Funcionalidades

- Gerenciamento de energia por região;
- Sistema de fases com progressão e desafios crescentes;
- Detecção de alertas, blackouts e sobrecarga;
- Minigames para recuperar regiões em estado crítico;
- Ranking de pontuação por fase;
- Modo secreto desbloqueável;
- Menu principal, pausa, ajustes de volume e tela de fim de partida;
- Controles por teclado e mouse.

## Requisitos

- GCC ou MinGW;
- raylib instalado e configurado;
- Ambiente Windows para a build atual do projeto (o Makefile usa bibliotecas específicas de Windows).

## Como compilar

Clone o repositório e execute:

```bash
git clone https://github.com/HenriqueCarneiro-dev/VolTerra.git
cd VolTerra
make
```

O comando acima gera o executável:

```bash
volterra.exe
```

Se quiser executar diretamente:

```bash
./volterra.exe
```

## Controles

- W / S ou setas para ajustar a produção da região selecionada;
- Mouse para selecionar regiões e interagir com a interface;
- R para resetar a produção da região;
- P ou Esc para pausar o jogo;
- Enter/Space para confirmar ações em telas de interação;
- Q para sair do jogo.

## Estrutura do projeto

```text
VolTerra/
├── Makefile
├── README.md
├── .gitignore
├── src/
│   ├── audio.c
│   ├── audio.h
│   ├── atualizar.c
│   ├── atualizar.h
│   ├── dados.c
│   ├── dados.h
│   ├── desenho_jogo.c
│   ├── desenho_jogo.h
│   ├── desenho_telas.c
│   ├── desenho_telas.h
│   ├── eventos.c
│   ├── eventos.h
│   ├── globais.c
│   ├── globais.h
│   ├── jogo.c
│   ├── jogo.h
│   ├── main.c
│   ├── minigames.c
│   ├── minigames.h
│   ├── ranking.c
│   ├── ranking.h
│   ├── regioes.c
│   ├── regioes.h
│   ├── tipos.h
│   ├── ui.c
│   ├── ui.h
│   ├── util.c
│   └── util.h
└──
```

## Observações de desenvolvimento

- O loop principal fica em `src/main.c`;
- A lógica do jogo está em `src/jogo.c` e `src/regioes.c`;
- Os elementos visuais e telas são tratados em `src/desenho_telas.c` e `src/desenho_jogo.c`;
- Os minigames e eventos são implementados em `src/minigames.c`, `src/eventos.c` e módulos relacionados;
- O projeto usa audio, ranking e utilitários em arquivos separados para manter a organização do código.

## Estado atual

Este repositório já contém uma versão funcional do jogo com menu, fases, minigames e sistema de pontuação. A documentação foi atualizada para refletir a estrutura real do projeto e os passos necessários para compilação local.

## Autores

- Jose Henrique Carneiro
- Caio Henrique de Sena Belmont
- Gustavo Cavalcanti Neves

Projeto desenvolvido na disciplina de Programação Imperativa e Funcional 2026.2, CESAR School.
