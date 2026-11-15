#ifndef MINIGAMES_H
#define MINIGAMES_H

#include "tipos.h"

/* Minigames de blackout e sobrecarga. */
Vector2 noEsq(int i);
Vector2 noDir(int i);
Rectangle discRect(int s);
Rectangle turbRect(int i);
Rectangle arenaFaiscas(void);
void iniciarMinigame(int id, bool blackout);
void atualizarMini(float dt);

#endif /* MINIGAMES_H */
