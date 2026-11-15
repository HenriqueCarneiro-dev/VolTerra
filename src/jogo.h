#ifndef JOGO_H
#define JOGO_H

#include "tipos.h"

/* Logica principal do jogo: novo jogo, fases, combo, transferencia de energia. */
void resetarCombo(bool avisar);
void voltarMenu(void);
void novoJogoFase(int f);
void novoJogo(void);
void iniciarSecreta(void);
void atualizarJogo(float dt);
void atualizarSecreta(float dt);

#endif /* JOGO_H */
