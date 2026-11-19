# ⚡ VolTerra

**Jogo de estratégia e gestão de energia em C, feito com [raylib](https://www.raylib.com/).**

Você é o operador da rede elétrica do Brasil. Ajuste a produção de cada região, envie energia de uma para outra e reaja a eventos climáticos para evitar **blackouts** e **sobrecargas**.

Projeto da disciplina **Programação Imperativa e Funcional 2026.2**, CESAR School.

> 🎬 **Vídeo de demonstração:** [ADICIONE AQUI O LINK DO VÍDEO](#)

<!-- Exemplo de capturas de tela (coloque as imagens em docs/ e descomente):
![Menu](docs/menu.png) ![Partida](docs/jogo.png) -->

---

## Sumário

- [Como o jogo funciona](#como-o-jogo-funciona)
- [Funcionalidades](#funcionalidades)
- [Instalação e compilação](#instalação-e-compilação)
- [Execução](#execução)
- [Controles](#controles)
- [Conceitos de programação aplicados](#conceitos-de-programação-aplicados)
- [Estrutura do projeto](#estrutura-do-projeto)
- [Arquitetura resumida](#arquitetura-resumida)
- [Problemas comuns](#problemas-comuns)
- [Autores](#autores)

---

## Como o jogo funciona

Cada região tem uma **bateria** que vai de 0% a 110%. A bateria sobe quando a região produz mais do que consome e desce no caso contrário.

| Nível da bateria | O que acontece |
|---|---|
| **30% a 80%** | Faixa segura. A pontuação sobe e o bônus cresce |
| **Acima de 80%** | Alerta de sobrecarga |
| **Abaixo de 30%** | Alerta de blackout |
| **110%** (sobrecarga) ou **0%** (blackout) | Um **minigame de emergência** é disparado |

**Seu trabalho:**

1. Selecione uma região no mapa e ajuste a produção dela para manter a bateria na faixa segura.
2. Quando uma região estiver em apuros, **arraste** de outra região até ela para enviar energia (a origem perde 10%, o destino recebe 8% se for vizinha ou 5% se for distante; a origem precisa ter pelo menos 25%).
3. Fique de olho nos **eventos climáticos**, que mudam produção e consumo por um tempo.
4. Resolva o minigame antes que o tempo acabe. Falhar custa um coração (você começa com 3).

**Pontuação e bônus:** enquanto todas as regiões estão estáveis, o bônus sobe de **x1 até x5**. Qualquer alerta zera o bônus.

**Fases:** são 5 fases de 4 dias cada. A fase 1 começa com 3 regiões, a fase 2 libera a quarta e a fase 3 libera a quinta. As fases 4 e 5 mantêm as 5 regiões com dificuldade crescente.

**Fase secreta:** concluindo as 5 fases **sem perder nenhuma vida**, você desbloqueia um modo em que cada região se divide nos **27 estados brasileiros**.

---

## Funcionalidades

- Mapa das cinco regiões do Brasil: Norte, Nordeste, Centro-Oeste, Sudeste e Sul;
- Cinco fases com regiões e desafios progressivamente maiores;
- Ajuste de produção por teclado, mouse ou roda do mouse;
- Transferência de energia entre regiões arrastando de uma até a outra;
- Eventos temporários que alteram produção e consumo;
- Alertas de nível crítico, risco de blackout e sobrecarga;
- Minigames de emergência: fios, disjuntores, gerador, turbinas, sincronia, alvos e faíscas;
- Sistema de vidas, combo e pontuação;
- Ranking persistente com os melhores resultados de cada fase;
- Trilha sonora e efeitos sonoros;
- Fase secreta com os 27 estados;
- Menu inicial, ajuda, seleção de fases, pausa, configurações de volume e tela cheia.

---

## Instalação e compilação

### Requisitos

- Windows;
- GCC/MinGW (recomendado: [MSYS2](https://www.msys2.org/), terminal **UCRT64**);
- raylib instalada e acessível ao GCC (no MSYS2: `pacman -S mingw-w64-ucrt-x86_64-raylib`);
- Bibliotecas do Windows usadas no link: `-lopengl32 -lgdi32 -lwinmm`.

O repositório já inclui `glfw3.dll`, `libraylib.dll` e um `volterra.exe` pré-compilado, além dos arquivos de áudio e fontes em `assets/`.

### Passo a passo

```bash
git clone https://github.com/HenriqueCarneiro-dev/VolTerra.git
cd VolTerra
mingw32-make        # ou apenas "make", se estiver disponível
```

Isso compila todos os arquivos de `src/*.c` e gera `volterra.exe` na raiz do projeto.

Para limpar os arquivos gerados:

```bash
mingw32-make clean
```

> O `clean` foi escrito para o prompt de comando do Windows (usa `del`). No terminal MSYS2, apague manualmente com `rm -f src/*.o volterra.exe`.

---

## Execução

```bash
./volterra.exe
```

Execute **a partir da raiz do projeto**, com as DLLs ao lado do executável, para que o jogo encontre `assets/` e as bibliotecas.

---

## Controles

| Ação | Controle |
|---|---|
| Selecionar região | Clique com o botão esquerdo |
| Ajustar a produção | `W` / `S`, `↑` / `↓` ou roda do mouse |
| Ajustar pela interface | Botões `+` e `-` |
| Voltar a produção para 100% | `R` ou botão Reset |
| Transferir energia | Arrastar de uma região ativa até outra |
| Navegar nos menus | Setas e `Enter` |
| Pausar | `P` ou `Esc` |
| Confirmar ações | `Enter` ou `Espaço` |
| Sair | `Q` |

---

## Conceitos de programação aplicados

| Conceito | Como é usado no jogo | Onde ver |
|---|---|---|
| **Estruturas (`struct`)** | `Regiao`, `Estado`, `EvDef`, `EvAtivo`, `Minigame`, `Trilha` e o nó do ranking `No` | `src/tipos.h` |
| **Ponteiros** | Acesso direto aos elementos (`Regiao *g = &r[i]`) e ponteiro para ponteiro (`No **pp`) para inserir no ranking | `src/ranking.c`, `src/desenho_jogo.c` |
| **Alocação dinâmica** | Cada nova pontuação do ranking é um nó criado com `malloc` e liberado com `free` (na poda dos registros e em `liberarRanking`) | `src/ranking.c` |
| **Listas encadeadas** | Ranking de cada fase como lista ordenada por pontos (6 listas: 5 fases + secreta) | `src/ranking.c` |
| **Matrizes** | `vizinho[N][N]` (adjacência entre regiões, define o ganho de 8% ou 5% no apoio) e `gridEst[GH][GW]` (grade que associa cada ponto do mapa a um estado) | `src/dados.c`, `src/regioes.c` |
| **Leitura e escrita em arquivo** | Ranking salvo em `ranking_fases.txt` ao fim de cada partida e lido ao abrir o jogo; `segredo.txt` guarda o desbloqueio da fase secreta | `src/ranking.c` |

---

## Estrutura do projeto

```text
VolTerra/
├── Makefile                  Configuração de compilação
├── README.md
├── .gitignore
├── glfw3.dll                 Dependência para Windows
├── libraylib.dll             raylib para Windows
├── volterra.exe              Executável pré-compilado
├── assets/
│   ├── menu.wav
│   ├── fasesnormal.wav
│   ├── fasesecreta.wav
│   └── fonts/                Fontes das telas (o jogo usa a fonte padrão se faltarem)
└── src/
    ├── main.c                Inicialização e loop principal
    ├── jogo.c                Fases, pontuação e atualização da partida
    ├── regioes.c             Polígonos das regiões e grade dos estados
    ├── atualizar.c           Lógica das telas (menu, ajuda, ranking, pausa)
    ├── eventos.c             Eventos climáticos e multiplicadores
    ├── minigames.c           Desafios de emergência
    ├── desenho_jogo.c        Desenho do mapa e da interface da partida
    ├── desenho_telas.c       Desenho do menu, fases, ajuda e ranking
    ├── dados.c / dados.h     Constantes, estados e tabelas do jogo
    ├── audio.c / audio.h     Sons e trilhas
    ├── ranking.c / ranking.h Ranking (lista encadeada e arquivo)
    ├── globais.c / globais.h Estado global compartilhado
    ├── ui.c / ui.h           Elementos de interface
    ├── util.c / util.h       Utilidades gráficas e matemáticas
    ├── tipos.h               Structs, enums e constantes
    └── gerar_documentacao.py Gera um arquivo único com a documentação do código
```

Arquivos criados pelo jogo durante a execução: `ranking_fases.txt` e `segredo.txt`.

---

## Arquitetura resumida

O ponto de entrada é `src/main.c`: ele abre uma janela de 1280×720, inicializa áudio, fontes, regiões e ranking, e roda o loop de atualização e desenho a 60 FPS. A interface usa uma textura virtual de 1280×720, redimensionada com proporção preservada.

O ciclo de uma partida:

- `atualizarJogo()` recalcula a bateria de cada região a cada quadro, lê o jogador, dá pontos e controla os dias e as fases;
- `tickEventos()` cria e remove eventos temporários, alterando produção e consumo;
- `transferir()` aplica o apoio de energia entre regiões;
- `iniciarMinigame()` e `atualizarMini()` sorteiam e executam o desafio quando uma região estoura;
- `atualizarSecreta()` controla o modo dos 27 estados;
- `fimDePartida()` e `inserirRanking()` registram a pontuação.

---

## Problemas comuns

| Problema | Solução |
|---|---|
| `libraylib.dll não foi encontrado` | Deixe `libraylib.dll` e `glfw3.dll` na mesma pasta do `volterra.exe` |
| `Makefile: missing separator` | As linhas de comando do Makefile precisam começar com **TAB**, não com espaços |
| `undefined reference to glfw...` | Ajuste a ordem e as bibliotecas do link em `LDFLAGS` (`-lraylib -lopengl32 -lgdi32 -lwinmm`) |
| Textos sem acento ou com fonte diferente | A pasta `assets/fonts/` não foi encontrada. O jogo funciona, mas com a fonte padrão |
| Sem som | Confirme que a pasta `assets/` está ao lado do executável |

---

## Observações

- O projeto é voltado ao **Windows**: o Makefile depende de bibliotecas específicas do sistema;
- Ainda não há configuração de build para Linux ou macOS;
- O repositório não possui licença definida.

---

## Autores

- José Henrique Carneiro
- Caio Henrique de Sena Belmont
- Gustavo Cavalcanti Neves

CESAR School, Programação Imperativa e Funcional 2026.2.
