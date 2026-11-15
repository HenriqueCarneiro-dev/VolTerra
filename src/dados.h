#ifndef DADOS_H
#define DADOS_H

#include "tipos.h"

/* Tabelas constantes do jogo (nomes, vizinhanca, eventos, trilhas, estados). */
typedef struct { const char *nome, *sig; int reg; float lon, lat; } EstDef;

extern const char *abrev[N];
extern const int ordemFase[N];
extern const Color fios[4];
extern const int vizinho[N][N];
extern const EvDef evDefs[NUM_EV];
extern const Trilha trilhas[NUM_MUS];
extern const EstDef estDef[NE];

#endif /* DADOS_H */
