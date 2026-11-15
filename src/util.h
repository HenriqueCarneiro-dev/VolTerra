#ifndef UTIL_H
#define UTIL_H

#include "tipos.h"

/* Funcoes utilitarias (projecao do mapa, mouse, botoes, parametros de fase). */
Rectangle btnFase(int i);
Rectangle abaRect(int i, int n);
int abasVisiveis(void);
Rectangle barraVol(int i);
float projX(float lon);
float projY(float lat);
float velFase(void);
float intensidade(void);
int regioesNaFase(int f);
int maxEventos(void);
int diasFase(void);
float diaDur(void);
float lim(float v, float a, float b);
float dist(Vector2 a, Vector2 b);
Color misturar(Color a, Color b, float t);
Vector2 mouseVirtual(void);
void textoCentro(const char *s, int x, int y, int tam, Color c);
void mensagem(const char *s, const char *titulo, float t, int cor);
void aviso(const char *s);
bool clicou(Rectangle rc);
bool hover(Rectangle rc);
bool dentro(Vector2 p, const Vector2 *v, int n);
int regiaoSob(Vector2 m);
int estadoSob(Vector2 m);
float tempoCritico(float bat, float rate);
float rateReg(int i);
float rateEst(int i);
const char *nomeAlvo(int i);
Rectangle linhaReg(int k);

#endif /* UTIL_H */
