/**
 * jogo.h
 * Estado e regras do jogo (noites, cota de energia, blackouts, pontos).
 *
 * Nenhuma função aqui desenha na tela: o desenho fica em desenho.c.
 */

#ifndef JOGO_H
#define JOGO_H

#include "regiao.h"

#define NUM_REGIOES    5
#define MAX_NOITES     5    // ao terminar a última noite, o jogador vence
#define MAX_BLACKOUTS  3    // ao atingir esse total, o jogador perde
#define DURACAO_NOITE  45   // segundos por noite

typedef enum {
    JOGO_RODANDO,
    JOGO_VITORIA,
    JOGO_DERROTA,
    JOGO_SAIR
} FaseJogo;

typedef struct {
    Regiao *regioes;        // vetor alocado dinamicamente (NUM_REGIOES)
    int numRegioes;
    int selecionada;        // índice da região sob o cursor
    int noite;              // 1..MAX_NOITES
    int segundosNoite;      // segundos já passados na noite atual
    int producao;           // cota total de energia disponível (MW)
    int pontos;
    int blackouts;          // blackouts acumulados
    FaseJogo fase;
    char mensagem[64];      // aviso mostrado ao jogador
} EstadoJogo;

/** Cria o jogo na noite 1. Retorna NULL se faltar memória. */
EstadoJogo *jogoCriar(void);

/** Libera toda a memória do jogo. */
void jogoDestruir(EstadoJogo *j);

/** Soma o consumo das regiões ligadas (MW). */
int jogoEnergiaUsada(const EstadoJogo *j);

/** Segundos de alerta antes do blackout na noite atual. */
int jogoTempoAlerta(const EstadoJogo *j);

/** Move o cursor para a próxima região ativa (delta = +1 ou -1). */
void jogoMover(EstadoJogo *j, int delta);

/** Move o cursor direto para a região idx, se ela estiver ativa. */
void jogoSelecionar(EstadoJogo *j, int idx);

/** Liga/desliga a região selecionada respeitando a cota de energia. */
void jogoAlternar(EstadoJogo *j);

/** Avança o jogo em um segundo (alertas, blackouts, pontos, troca de noite). */
void jogoAtualizarSegundo(EstadoJogo *j);

#endif /* JOGO_H */
