#ifndef RANKING_H
#define RANKING_H

#include "tipos.h"

/* Ranking por fase, segredo e fim de partida. */
int inserirRanking(int idx, const char *nome, int p);
void liberarRanking(void);
void carregarRanking(void);
void salvarRanking(void);
void carregarSegredo(void);
void salvarSegredo(void);
void fimDePartida(bool venceu);

#endif /* RANKING_H */
