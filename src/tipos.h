#ifndef TIPOS_H
#define TIPOS_H

#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define W 1280
#define H 720
#define N 5
#define NE 27
#define FASES 5
#define DIAS 4
#define DIA_SEG 30.0f
#define SEC_DIAS 3
#define SEC_DIA_SEG 40.0f
#define BAT_MAX 110.0f
#define LIM_BAIXO 30.0f
#define LIM_ALTO 80.0f
#define NUM_EV 9
#define MAX_EV 5
#define MAX_RANK 10
#define TAM_NOME 12
#define NUM_RANK 6      /* 5 fases + secreta */
#define CEL 3
#define GW (W / CEL + 1)
#define GH (H / CEL + 1)
#define SR 22050
#define TEMPO_CRONO 20.0f
#define NUM_MUS 7       /* 0 menu, 1..5 fases, 6 secreta */
#define COMBO_SEG 12.0f
#define COMBO_MAX 5
#define MAX_FAISCAS 6
#define RAIO_ESCUDO 14.0f
#define RAIO_FAISCA 11.0f
#define MAX_GOLPES 3

enum { NORTE, NORDESTE, CENTRO, SUDESTE, SUL };
typedef enum { TELA_MENU, TELA_AJUDA, TELA_FASES, TELA_RANK, TELA_JOGO, TELA_MINI, TELA_PAUSA,
               TELA_TROCA, TELA_NOME, TELA_FIM } Tela;
typedef enum { MINI_FIOS, MINI_DISJUNTOR, MINI_GERADOR,
               MINI_TURBINAS, MINI_SINCRONIA, MINI_ALVOS, MINI_FAISCAS } TipoMini;

typedef struct {
    const char *nome;
    Color cor;
    int fonte;
    Vector2 pt[56];
    int n;
    Vector2 c;
    float bat, ajuste, consBase;
    float multProd, multCons;
    bool ativa;
} Regiao;

typedef struct {
    const char *nome, *sig;
    int reg;
    Vector2 c;
    float bat, ajuste, cons;
} Estado;

typedef struct {
    const char *nome;
    float prod[N];
    float cons[N];
} EvDef;

typedef struct {
    int id;
    float t, dur;
} EvAtivo;

typedef struct No {
    char nome[16];
    int pontos;
    struct No *prox;
} No;

typedef struct {
    TipoMini tipo;
    bool blackout;
    int reg;
    float tempo, tempoMax;
    int ordem[4];
    int destino[4];
    int arrasta;
    int slot[6];
    bool on[6];
    int proximo;
    float flash;
    float calor[4], taxa[4];
    float marcador, dir, zona, larg, vel;
    int acertos;
    float carga;
    int alvo;
    float alvoT;
    int resultado;   /* 0 jogando, 1 venceu, 2 perdeu */
    float fimT;
    /* MINI_FAISCAS: escudo do jogador desviando de faiscas em movimento */
    Vector2 escudo;
    Vector2 fPos[MAX_FAISCAS], fVel[MAX_FAISCAS];
    int nFaiscas, golpes;
    float invul;     /* tempo de invulnerabilidade apos um toque */
} Minigame;

typedef struct {
    int raiz[16];
    int tipo[16];    
    float bpm;
    int arp;
    int prob;
    unsigned seed;
    float vol;
} Trilha;

#endif /* TIPOS_H */
