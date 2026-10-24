# VolTerra

VolTerra é um jogo de terminal em C em que o jogador assume o papel de operador de uma usina elétrica nacional. O objetivo é manter as regiões do país abastecidas sem exceder a cota total de energia disponível, evitando blackouts e garantindo a sobrevivência da rede ao longo das noites.

O jogo foi implementado como uma simulação de gestão de energia em tempo real, com interface textual, seleção de regiões, controle de consumo, pontuação e progressão de dificuldade. Cada noite aumenta a demanda e desbloqueia novas regiões, exigindo decisões rápidas para equilibrar a carga e manter o país funcionando.

## Sobre o jogo

O jogador controla uma central de energia responsável por 5 regiões: Sudeste, Nordeste, Sul, Centro-Oeste e Norte. Em cada noite, a produção disponível é limitada por uma cota de energia e o jogador precisa ligar ou desligar regiões conforme a demanda. A região selecionada pode estar em três estados principais: abastecida, em alerta e em blackout. Se uma região for desligada, ela entra em contagem regressiva e, ao zerar o tempo, perde o fornecimento e entra em apagão.

A dificuldade aumenta conforme as noites avançam. A energia disponível cresce gradualmente, mas novas regiões entram em operação, tornando a gestão mais delicada. A cada segundo em que uma região permanece ligada, o jogador acumula pontos, enquanto blackouts reduzem o total e podem levar à derrota. O jogo termina com vitória após a quinta noite concluída ou com derrota caso o limite de blackouts seja alcançado.

## Funcionalidades implementadas

- 5 regiões com consumo próprio e estados de abastecimento/alerta/blackout;
- cota de energia por noite, controlada dinamicamente pelo estado do jogo;
- progressão por noites com desbloqueio gradual de regiões;
- seleção por teclado com navegação e atalhos numéricos;
- mecanismo de alerta antes do blackout;
- pontuação por energia ativa e bônus por sobrevivência da noite;
- derrota quando o número de blackouts excede o limite;
- vitória ao completar todas as noites;
- interface em terminal com renderização de painel, status e mensagens do jogo.

## Controles

| Tecla | Ação |
|---|---|
| Setas para cima/baixo ou W/S | Move o cursor entre as regiões |
| Enter ou Espaço | Liga/desliga a região selecionada |
| 1 a 5 | Seleciona rapidamente uma região e alterna seu estado |
| Q | Sai do jogo |
| R | Reinicia após a tela final |

## Como compilar e jogar

Requisitos:
- Linux ou macOS
- GCC
- WSL ou máquina virtual para execução em Windows

```bash
git clone https://github.com/HenriqueCarneiro-dev/VolTerra.git
cd VolTerra
make run
```

Também é possível compilar e executar manualmente:

```bash
make
./volterra
```

## Como jogar

1. Pressione Enter na tela inicial para começar.
2. Navegue entre as regiões com as setas ou W/S.
3. Use Enter/Space para ligar ou desligar a região atual.
4. Mantenha o consumo total abaixo da cota de energia da noite.
5. Evite deixar regiões em alerta até o blackout.
6. Supervive às noites e alcance a vitória.

## Requisitos da disciplina

Conceitos aplicados no projeto:

- Structs: `EstadoJogo` e `Regiao`, usados para representar o estado geral do jogo e cada região do país.
- Ponteiros: uso de ponteiros para manipular regiões e estados do jogo (`Regiao *`, `EstadoJogo *`).
- Alocação dinâmica de memória: `malloc` e `calloc` para alocar o vetor de regiões e a estrutura principal do jogo.
- Vetores: armazenamento das regiões em vetor dinâmico e controle de seleção por índice.
- Listas encadeadas: não foram utilizadas neste projeto.
- Escrita e leitura em arquivo: não houve persistência de dados em arquivo; o jogo opera em memória durante a execução.

## Equipe

Projeto desenvolvido no contexto da disciplina de Programação Imperativa e Funcional 2026.2, CESAR School.

## Créditos

O projeto utiliza a biblioteca [cli-lib](https://github.com/tgfb/cli-lib) (licença BSD-3-Clause), mantida em `lib/` sem alterações.

