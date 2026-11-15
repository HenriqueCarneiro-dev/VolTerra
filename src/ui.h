#ifndef UI_H
#define UI_H

#include "tipos.h"

/* Primitivas de desenho reutilizaveis (botoes, icones, gradientes, cronometro). */
void telaCarregando(const char *t);
void caixa(Rectangle rc, float rd, Color fill, Color borda);
void desenharBotao(Rectangle rc, const char *t, Color base);
void coracao(float x, float y, float s, Color c);
void preencherComGradiente(const Vector2 *v, int n, Color corTopo, Color corBase);
void icone(int f, float x, float y, float t, float e);
Color corBatVisual(float b);
void setaEvento(float x, float y, bool sobe, float esc);
void cronometro(float x, float y, float raio, float seg, bool black);

#endif /* UI_H */
