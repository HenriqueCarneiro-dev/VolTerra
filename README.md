# VolTerra

## Visão Geral

VolTerra é um jogo de terminal desenvolvido em linguagem C com foco na gestão de uma usina elétrica nacional. O projeto simula a operação de uma rede de distribuição de energia em que o jogador deve equilibrar a geração e o consumo entre diferentes regiões do país, mantendo o sistema estável ao longo de múltiplas noites de operação.

A proposta do jogo combina elementos de estratégia, tomada de decisão e gerenciamento de recursos em tempo real. O jogador deve selecionar regiões, ativar ou desativar fornecimento de energia e controlar a cota disponível, prevenindo falhas operacionais e evitando blackouts generalizados. A dificuldade cresce progressivamente conforme novas regiões entram em operação e a exigência de estabilidade da rede aumenta.

## Objetivo do Jogo

O objetivo principal é manter o país abastecido e maximizar a pontuação ao longo de cinco noites de simulação. Cada região possui um consumo específico de energia e pode estar em diferentes estados operacionais: abastecida, em alerta ou em blackout. O jogador deve distribuir a energia disponível de forma eficiente, respeitando a cota da usina e evitando que o sistema falhe.

Ao longo da partida, o desempenho é medido por pontos acumulados, que aumentam quando as regiões ativas permanecem abastecidas. Por outro lado, falhas na rede, como blackouts e excesso de demanda, reduzem a pontuação e podem resultar em derrota quando o número de interrupções supera o limite estabelecido.

## Funcionalidades Implementadas

- Simulação de rede elétrica com cinco regiões e consumo individual por área;
- controle de cota de energia disponível por noite;
- estados operacionais das regiões: inativa, abastecida, em alerta e blackout;
- progressão por noites com aumento de complexidade;
- seleção de regiões por teclado com navegação e atalhos numéricos;
- mecanismo de alerta antes da falha total da região;
- pontuação por estabilidade da rede e bônus por sobrevivência das noites;
- condição de vitória ao completar a última noite e derrota ao atingir o limite de blackouts;
- interface em terminal com painel de status, indicadores visuais e mensagens de contexto.

## Mecânica de Jogo

O jogador interage com a interface via teclado, navegando entre as regiões disponíveis e alternando o estado de cada uma. Quando uma região é desligada, ela entra em contagem regressiva de alerta e, caso não seja reativada antes do fim do temporizador, sofre blackout. A manutenção do equilíbrio entre consumo e cota de energia é crítica, uma vez que a usina não possui capacidade infinita para abastecer todas as áreas simultaneamente.

A cada noite, novas regiões passam a fazer parte da rede, exigindo adaptações rápidas na estratégia do jogador. O sistema também contabiliza eventos de falha e recompensa a estabilidade da operação. Dessa forma, a partida une estratégia de gerenciamento com reações em tempo real.

## Controles

| Tecla | Ação |
|---|---|
| Setas para cima/baixo ou W/S | Navegação entre regiões |
| Enter ou Espaço | Liga/desliga a região selecionada |
| 1 a 5 | Seleção direta de região e alternância rápida |
| Q | Encerra a partida |
| R | Reinicia o jogo após a tela final |

## Requisitos de Execução

- Sistema operacional: Linux ou macOS
- Compilador: GCC
- Ambiente Windows: recomendado uso de WSL ou máquina virtual

## Compilação e Execução

```bash
git clone https://github.com/HenriqueCarneiro-dev/VolTerra.git
cd VolTerra
make run
```

Também é possível compilar diretamente:

```bash
make
./volterra
```

## Estrutura do Projeto

O repositório está organizado em módulos com responsabilidades distintas:

- `src/`: implementação principal do jogo, estados, lógica de regras e interface textual;
- `include/`: definições de estruturas e protótipos das funções;
- `lib/`: biblioteca auxiliar `cli-lib`, utilizada para renderização e interação em terminal;
- `Makefile`: automação da compilação e execução do projeto.

## Objetivos de Aprendizagem da Disciplina

O projeto foi desenvolvido no contexto da disciplina de Programação Imperativa e Funcional e aplica conceitos fundamentais da linguagem C, com ênfase em organização modular e gerenciamento de memória. Os principais tópicos contemplados incluem:

- Structs: utilização de `EstadoJogo` e `Regiao` para representar o estado do sistema e das áreas atendidas;
- Ponteiros: manipulação direta de estruturas e referências para regiões e estados do jogo;
- Alocação dinâmica de memória: uso de `malloc` e `calloc` para criação do vetor de regiões e inicialização do estado do jogo;
- Vetores: armazenamento das regiões em estruturas indexadas e acesso por posição;
- Listas encadeadas: não aplicadas neste projeto;
- Leitura e escrita em arquivo: não implementadas como mecanismo persistente de salvamento; o jogo é executado em memória durante a sessão.

## Equipe

Projeto desenvolvido no contexto da disciplina de Programação Imperativa e Funcional 2026.2, CESAR School.

## Créditos

O projeto utiliza a biblioteca [cli-lib](https://github.com/tgfb/cli-lib), distribuída sob a licença BSD-3-Clause, mantida em `lib/` sem modificações no código-fonte original.

