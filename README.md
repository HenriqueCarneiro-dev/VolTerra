# VolTerra

**VolTerra** é um jogo de estratégia e gerenciamento de energia desenvolvido em C com a biblioteca [raylib](https://www.raylib.com/) para o curso de Programação Imperativa Funcional (PIF).

O jogador mantém a estabilidade da rede elétrica do Brasil, ajustando a produção de regiões, transferindo energia entre elas e respondendo a eventos aleatórios. O objetivo é equilibrar geração e consumo durante as fases, evitando tanto blackout quanto sobrecarga.

## Funcionalidades

- Mapa das cinco regiões do Brasil: Norte, Nordeste, Centro-Oeste, Sudeste e Sul;
- Cinco fases com regiões e desafios progressivamente maiores;
- Ajuste de produção por teclado, mouse ou roda do mouse;
- Transferência de energia entre regiões arrastando uma região até outra;
- Eventos temporários que alteram produção e consumo;
- Alertas de nível crítico, risco de blackout e sobrecarga;
- Minigames emergenciais com fios, disjuntores, geradores, turbinas, sincronização, alvos e faiscas;
- Sistema de vidas, combo e pontuação;
- Ranking persistente com os melhores resultados;
- Trilha sonora dinâmica e efeitos sonoros;
- Fases secretas com os 27 estados brasileiros;
- Modo secreto desbloqueado ao concluir as cinco fases sem perder vidas;
- Tela inicial, ajuda, seleção de fases, pausa e tela de fim de partida.

## Requisitos

- Windows;
- GCC/MinGW;
- `make`;
- raylib configurada para linkagem pelo GCC;
- Bibliotecas Windows usadas pelo Makefile: `-lopengl32 -lgdi32 -lwinmm`.

O repositório já inclui os executáveis distribuídos `glfw3.dll`, `libraylib.dll` e `volterra.exe`, além de os arquivos de áudio em `assets/`.

## Compilação

Clone o repositório:

```bash
git clone https://github.com/HenriqueCarneiro-dev/VolTerra.git
cd VolTerra
```

Compile o jogo:

```bash
make
```

Isso usa o `Makefile` para compilar todos os arquivos `src/*.c` e gera:

```text
volterra.exe
```

Para remover os objetos e o executável:

```bash
make clean
```

O comando `make clean` foi escrito para o prompt de comando do Windows.

## Execução

No Windows:

```bash
./volterra.exe
```

Mantenha o executável na raiz do projeto para que o raylib encontre `assets/` e os arquivos DLL corretamente.

## Controles

| Ação | Controle |
|---|---|
| Ajustar a produção da região selecionada | `W`, `S`, `↑`, `↓` ou roda do mouse |
| Selecionar região | Clique com o botão esquerdo |
| Transferir energia | Arraste de uma região ativa até outra |
| Aumentar/diminuir produção pela interface | Botões `+` e `-` |
| Reiniciar produção da região | `R` |
| Pausar o jogo | `P` ou `Esc` |
| Confirmar ações | `Enter` ou `Espaço` |
| Sair | `Q` |

## Como jogar

1. Selecione uma região no mapa;
2. Ajuste a produção até manter o nível da bateria entre os limites seguros;
3. Transferir energia pode ajudar regiões em situação crítica, mas consome energia da região de origem;
4. Complete os dias de cada fase mantendo todas as regiões estáveis;
5. Ao entrar em blackout ou sobrecarga, resolva o minigame emergencial dentro do tempo;
6. Complete as cinco fases para desbloquear o modo secreto.

O nível seguro é definido pelas constantes `LIM_BAIXO` e `LIM_ALTO`; a capacidade máxima da bateria é controlada por `BAT_MAX`, em `src/tipos.h`.

## Estrutura do projeto

```text
VolTerra/
├── Makefile                 Configuração de compilação
├── README.md
├── .gitignore
├── glfw3.dll                Dependência distribuída para Windows
├── libraylib.dll            raylib distribuída para Windows
├── volterra.exe             Executável pré-compilado
├── assets/
│   ├── menu.wav
│   ├── fasesnormal.wav
│   └── fasesecreta.wav
└── src/
    ├── main.c               Inicialização e loop principal
    ├── jogo.c               Fases, pontuação e atualização da partida
    ├── regioes.c            Polígonos das regiões e estados
    ├── atualizar.c          Gerenciamento do estado atual do jogo
    ├── eventos.c            Eventos dinâmicos da rede
    ├── minigames.c          Desafios emergenciais
    ├── desenho_jogo.c       Renderização do mapa e da interface
    ├── desenho_telas.c      Renderização das telas
    ├── dados.c / dados.h    Constantes, estados e tabelas do jogo
    ├── audio.c / audio.h    Sons e trilhas
    ├── ranking.c / ranking.h Ranking de pontuação
    ├── globais.c / globais.h Estado global compartilhado
    ├── ui.c / ui.h          Elementos e interações de interface
    ├── util.c / util.h      Utilidades gráficas e matemáticas
    └── gerar_documentacao.py Gera um arquivo consolidado da documentação
```

## Arquitetura resumida

O ponto de entrada é `src/main.c`. Ele cria uma janela de 1280×720, inicializa áudio, fontes, regiões e ranking e executa um loop de atualização e desenho a 60 FPS.

A partida é atualizada principalmente por `src/jogo.c`:

- `atualizarJogo()` controla as fases comuns e as cinco regiões brasileiras;
- `atualizarSecreta()` controla o modo com os 27 estados;
- `tickEventos()` cria eventos temporários e modifica os multiplicadores de produção e consumo;
- `atualizarMini()` executa os minigames emergenciais;
- `src/regioes.c` define a representação geográfica das regiões e da grade usada pelo modo secreto.

A interface usa uma textura virtual de 1280×720 e é redimensionada com proporção preservada para a janela real.

## Observações

- O projeto foi desenvolvido principalmente para Windows, porque o `Makefile` depende de bibliotecas específicas do Windows;
- Não há sistema de build Linux configurado atualmente;
- Não há licença adicionada ao repositório;
- Não foram encontrados workflows do GitHub Actions na raiz do projeto.

## Autores

- Jose Henrique Carneiro;
- Caio Henrique de Sena Belmont;
- Gustavo Cavalcanti Neves.

Projeto desenvolvido na disciplina de **Programação Imperativa e Funcional 2026.2**, da CESAR School.
