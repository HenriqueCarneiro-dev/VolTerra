#ifndef EVENTOS_H
#define EVENTOS_H

#include "tipos.h"

/* Eventos climaticos e multiplicadores. */
bool linhaEfeito(char *buf, int sz, int id, int i, float *net);
void tickEventos(float dt);

#endif /* EVENTOS_H */
