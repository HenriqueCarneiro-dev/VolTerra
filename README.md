# VolTerra

Jogo de terminal em C de gerenciamento de uma usina elétrica nacional.

## Visão Geral

VolTerra é um jogo de terminal desenvolvido em linguagem C com foco na gestão de uma usina elétrica nacional. O jogador atua como operador da usina e precisa distribuir uma cota finita de energia entre as regiões do país, mantendo a rede estável ao longo de cinco noites de operação.

A proposta combina estratégia, tomada de decisão e gerenciamento de recursos em tempo real. O painel de controle usa chaves simples de ligar/desligar para cada região. Como a cota da usina não cobre todas as regiões ao mesmo tempo, o jogador precisa fazer um rodízio: regiões desligadas entram em alerta e, se a contagem chegar a zero antes de serem religadas, sofrem blackout. A dificuldade cresce a cada noite, com novas regiões entrando na rede e menos tempo de alerta.

## Objetivo do Jogo

Sobreviver às cinco noites de simulação sem acumular três blackouts, somando o máximo de pontos possível.

Cada região tem um consumo próprio de energia e pode estar em um destes estados: **bloqueada** (ainda não entrou na rede), **abastecida**, **em alerta** ou **em blackout**. Os pontos aumentam enquanto as regiões permanecem abastecidas e há bônus ao completar cada noite. Cada blackout reduz a pontuação. Ao atingir o limite de 3 blackouts, o jogador perde.

## Regras e Valores

| Item | Valor |
|---|---|
| Duração de cada noite | 45 segundos |
| Número de noites | 5 (completar a quinta é a vitória) |
| Limite de blackouts | 3 (ao atingir, é derrota) |
| Cota de energia | 90 MW na noite 1, mais 10 MW a cada noite (130 MW na noite 5) |
| Tempo de alerta antes do blackout | 14 s na noite 1, menos 2 s por noite (mínimo de 6 s) |
| Pontos | +10 por região abastecida a cada segundo; −50 por blackout; +100 × número da noite ao concluí-la |

| Região | Consumo | Entra na noite |
|---|---|---|
| Sudeste | 50 MW | 1 |
| Nordeste | 35 MW | 1 |
| Sul | 30 MW | 1 |
| Centro-Oeste | 20 MW | 2 |
| Norte | 25 MW | 3 |

Se o jogador tentar ligar uma região que ultrapassaria a cota, a ação é recusada e o jogo avisa. Uma região em blackout continua no escuro até ser religada, o que exige cota disponível. O blackout já contado não é desfeito.

## Funcionalidades Implementadas

- Simulação de rede elétrica com cinco regiões e consumo individual por área;
- Controle de cota de energia por noite, com bloqueio de ligações acima da cota;
- Estados das regiões: bloqueada, abastecida, em alerta e blackout;
- Contagem regressiva de alerta antes do blackout, com destaque visual quando restam poucos segundos;
- Progressão por noites com novas regiões, mais cota e menos tempo de alerta;
- Pontuação por estabilidade, bônus por noite concluída e penalidade por blackout;
- Condições de vitória e derrota, com tela final e opção de jogar novamente;
- Tela de título, painel de status colorido e mensagens de contexto;
- Seleção de regiões por setas, W/S e atalhos numéricos.

## Em Desenvolvimento

- Minigames de eventos de emergência (guiar um fio por um labirinto de circuitos e conectar cabos), com detecção de colisão;
- Lista de eventos ativos em tempo real;
- Labirintos carregados de arquivos em `data/`;
- Recorde (pontuação máxima) salvo em arquivo.

## Controles

| Tecla | Ação |
|---|---|
| Setas para cima/baixo ou W/S | Navegar entre as regiões |
| Enter ou Espaço | Ligar/desligar a região selecionada |
| 1 a 5 | Selecionar a região direto e alternar seu estado |
| Q | Sair do jogo |
| R | Jogar novamente (na tela final) |

O Ctrl+C fica desativado pela biblioteca de terminal. Use **Q** para sair.

## Requisitos de Execução

- Sistema operacional: Linux ou macOS;
- Compilador: GCC e `make`;
- Terminal com pelo menos 80x24 caracteres;
- Windows: usar WSL. Se a virtualização estiver desativada no computador, o WSL 1 funciona sem ela (`wsl --set-default-version 1`, depois `wsl --install -d Ubuntu`). No Ubuntu, instale as ferramentas com `sudo apt install build-essential`.

## Compilação e Execução

```bash
git clone https://github.com/HenriqueCarneiro-dev/VolTerra.git
cd VolTerra
make run
```

Também é possível compilar e executar separadamente:

```bash
make
./volterra
```

Depois de alterar algum arquivo `.h`, use `make clean && make`, porque o Makefile não rastreia headers.

## Estrutura do Projeto

```
VolTerra/
├── Makefile
├── data/                  reservado para labirintos e recorde
├── include/
│   ├── desenho.h
│   ├── jogo.h
│   └── regiao.h
├── lib/                   cli-lib (sem modificações)
└── src/
    ├── desenho.c
    ├── jogo.c
    ├── main.c
    └── regiao.c
```

- `src/main.c`: loop principal, leitura de teclas (incluindo setas) e controle das telas;
- `src/jogo.c`: regras do jogo (cota, noites, alertas, blackouts, pontuação);
- `src/regiao.c`: dados e estados de cada região;
- `src/desenho.c`: todo o desenho na tela, separado da lógica;
- `include/`: estruturas e protótipos de cada módulo;
- `lib/`: biblioteca auxiliar `cli-lib`, usada para tela, teclado e temporizador.

A lógica do jogo (`jogo.c` e `regiao.c`) não desenha nada, e o desenho (`desenho.c`) não altera regras. Isso facilita testar e dividir o trabalho.

## Requisitos da Disciplina

| Conceito | Situação | Onde |
|---|---|---|
| Structs | Implementado | `Regiao` (`include/regiao.h`) e `EstadoJogo` (`include/jogo.h`) |
| Ponteiros | Implementado | Funções como `regiaoLigar(Regiao *r)` e `jogoAlternar(EstadoJogo *j)` recebem ponteiros para as estruturas |
| Alocação dinâmica | Implementado | `calloc` do `EstadoJogo` e `malloc` do vetor de regiões em `jogoCriar` (`src/jogo.c`), com `free` em `jogoDestruir` |
| Vetores | Implementado | Vetor de regiões indexado por posição |
| Matrizes | Previsto | Mapa do labirinto de circuitos nos minigames |
| Listas encadeadas | Previsto | Lista de eventos de emergência ativos |
| Arquivos | Previsto | Leitura dos labirintos e gravação do recorde em `data/` |
| Detecção de colisão | Previsto | Fio contra a parede do labirinto |

## Vídeo de Demonstração

<link do vídeo>

## Equipe

- Nome 1 (@github)
- Nome 2 (@github)
- Nome 3 (@github)

Projeto desenvolvido na disciplina de Programação Imperativa e Funcional 2026.2, CESAR School.

## Créditos

O projeto utiliza a biblioteca [cli-lib](https://github.com/tgfb/cli-lib), distribuída sob a licença BSD-3-Clause, mantida em `lib/` sem modificações no código-fonte original.
