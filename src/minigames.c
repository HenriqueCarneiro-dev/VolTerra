#include "tipos.h"
#include "globais.h"
#include "audio.h"
#include "jogo.h"
#include "ranking.h"
#include "util.h"
#include "minigames.h"

Vector2 noEsq(int i)      { return (Vector2){435, 300 + i * 65}; }

Vector2 noDir(int i)      { return (Vector2){845, 300 + i * 65}; }

static Vector2 centroSlot(int s) { return (Vector2){440.0f + (s % 3) * 200.0f, 330.0f + (s / 3) * 120.0f}; }

Rectangle discRect(int s) { Vector2 c = centroSlot(s); return (Rectangle){c.x - 60, c.y - 40, 120, 80}; }

Rectangle turbRect(int i) { return (Rectangle){370.0f + i * 150.0f, 260, 110, 230}; }

Rectangle arenaFaiscas(void) { return (Rectangle){330, 235, 620, 295}; }

static void novoAlvo(void)
{
    int n = GetRandomValue(0, 5);
    if (n == mini.alvo) n = (n + 1) % 6;
    mini.alvo = n;
    mini.alvoT = lim(1.4f - 0.12f * (fase - 1), 0.8f, 1.4f);
}

void iniciarMinigame(int id, bool blackout)
{
    memset(&mini, 0, sizeof(mini));
    mini.reg = id;
    mini.blackout = blackout;
    mini.arrasta = -1;
    arrasta = -1;
    resetarCombo(true);
    tocar(blackout ? sndBlack : sndSobre);
    int variante = GetRandomValue(0, blackout ? 2 : 3);
    float f = (float)(fase - 1);

    if (blackout) mini.tipo = variante == 0 ? MINI_FIOS : (variante == 1 ? MINI_DISJUNTOR : MINI_GERADOR);
    else          mini.tipo = variante == 0 ? MINI_TURBINAS : (variante == 1 ? MINI_SINCRONIA :
                                  (variante == 2 ? MINI_ALVOS : MINI_FAISCAS));

    switch (mini.tipo) {
        case MINI_FIOS:
            mini.tempoMax = 30.0f - 2.0f * f;
            for (int i = 0; i < 4; i++) { mini.ordem[i] = i; mini.destino[i] = -1; }
            for (int i = 3; i > 0; i--) {
                int k = GetRandomValue(0, i);
                int t = mini.ordem[i];
                mini.ordem[i] = mini.ordem[k];
                mini.ordem[k] = t;
            }
            break;
        case MINI_DISJUNTOR:
            mini.tempoMax = 22.0f - 1.5f * f;
            for (int i = 0; i < 6; i++) mini.slot[i] = i + 1;
            for (int i = 5; i > 0; i--) {
                int k = GetRandomValue(0, i);
                int t = mini.slot[i];
                mini.slot[i] = mini.slot[k];
                mini.slot[k] = t;
            }
            mini.proximo = 1;
            break;
        case MINI_GERADOR:
            mini.tempoMax = 16.0f;
            mini.carga = 10.0f;
            break;
        case MINI_TURBINAS:
            mini.tempoMax = 15.0f;
            for (int i = 0; i < 4; i++) {
                mini.calor[i] = 15.0f + GetRandomValue(0, 15);
                mini.taxa[i] = 7.0f + 1.5f * f + GetRandomValue(0, 40) / 10.0f;
            }
            break;
        case MINI_SINCRONIA:
            mini.tempoMax = 22.0f - 1.0f * f;
            mini.vel = 0.55f + 0.12f * f;
            mini.larg = 0.24f - 0.02f * f;
            mini.marcador = 0.0f;
            mini.dir = 1.0f;
            mini.zona = GetRandomValue(25, 75) / 100.0f;
            break;
        case MINI_FAISCAS: {
            Rectangle a = arenaFaiscas();
            mini.tempoMax = 12.0f + 1.0f * f;
            mini.nFaiscas = 4 + (fase - 1) / 2;
            if (mini.nFaiscas > MAX_FAISCAS) mini.nFaiscas = MAX_FAISCAS;
            mini.escudo = (Vector2){a.x + a.width / 2, a.y + a.height / 2};
            mini.invul = 1.0f;
            float vel = 130.0f + 22.0f * f;
            for (int i = 0; i < mini.nFaiscas; i++) {
                float ang = GetRandomValue(0, 359) * DEG2RAD;
                mini.fPos[i] = (Vector2){a.x + RAIO_FAISCA + GetRandomValue(0, (int)(a.width - 2 * RAIO_FAISCA)),
                                         a.y + RAIO_FAISCA + GetRandomValue(0, 70)};
                mini.fVel[i] = (Vector2){cosf(ang) * vel, sinf(ang) * vel};
            }
            break;
        }
        default:
            mini.tempoMax = 22.0f;
            mini.alvo = -1;
            novoAlvo();
            break;
    }
    mini.tempo = mini.tempoMax;
    tela = TELA_MINI;
}

static void finalizarMini(void)
{
    float *b = secreta ? &est[mini.reg].bat : &r[mini.reg].bat;
    *b = mini.blackout ? 55.0f : 75.0f;
    tickT = 0;
    memset(alertaPrev, 0, sizeof(alertaPrev));
    if (mini.resultado == 1) {
        pontos += 100 * fase;
        mensagem("A rede foi estabilizada.", "MINIGAME VENCIDO", 3.0f, 2);
    } else {
        vidas--;
        if (vidas <= 0) { secretaVenceu = false; fimDePartida(false); return; }
        mensagem("Voce perdeu uma vida.", "FALHA NA REDE", 3.0f, 3);
    }
    tela = TELA_JOGO;
}

static void terminarMini(int res)
{
    mini.resultado = res;
    mini.fimT = 1.3f;
    tocar(res == 1 ? sndOk : sndFalha);
}

void atualizarMini(float dt)
{
    Minigame *g = &mini;
    Vector2 m = mouseVirtual();
    bool press = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (g->flash > 0) g->flash -= dt;
    if (g->resultado) {
        g->fimT -= dt;
        if (g->fimT <= 0) finalizarMini();
        return;
    }
    g->tempo -= dt;

    switch (g->tipo) {
        case MINI_FIOS: {
            if (press) {
                for (int i = 0; i < 4; i++)
                    if (g->destino[i] < 0 && dist(m, noEsq(i)) < 24) g->arrasta = i;
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && g->arrasta >= 0) {
                for (int j = 0; j < 4; j++) {
                    if (dist(m, noDir(j)) < 28) {
                        if (g->ordem[g->arrasta] == j) g->destino[g->arrasta] = j;
                        else { g->flash = 0.4f; g->tempo -= 2.0f; }
                    }
                }
                g->arrasta = -1;
            }
            int ok = 0;
            for (int i = 0; i < 4; i++) if (g->destino[i] >= 0) ok++;
            if (ok == 4) terminarMini(1);
            break;
        }
        case MINI_DISJUNTOR:
            if (press) {
                for (int s = 0; s < 6; s++) {
                    if (!CheckCollisionPointRec(m, discRect(s)) || g->on[s]) continue;
                    if (g->slot[s] == g->proximo) {
                        g->on[s] = true;
                        g->proximo++;
                        tocar(sndClique);
                        if (g->proximo > 6) terminarMini(1);
                    } else {
                        g->flash = 0.5f;
                        g->proximo = 1;
                        for (int k = 0; k < 6; k++) g->on[k] = false;
                        g->tempo -= 1.5f;
                    }
                }
            }
            break;
        case MINI_GERADOR:
            if (IsKeyPressed(KEY_SPACE) || clicou(btnSinc)) g->carga += 8.0f;
            g->carga -= (8.0f + 1.5f * (fase - 1)) * dt;
            g->carga = lim(g->carga, 0.0f, 100.0f);
            if (g->carga >= 100.0f) terminarMini(1);
            break;
        case MINI_TURBINAS:
            for (int i = 0; i < 4; i++) {
                g->calor[i] += g->taxa[i] * dt;
                if (press && CheckCollisionPointRec(m, turbRect(i))) { g->calor[i] -= 38.0f; tocar(sndClique); }
                g->calor[i] = lim(g->calor[i], 0.0f, 100.0f);
                if (g->calor[i] >= 100.0f) { terminarMini(2); return; }
            }
            if (g->tempo <= 0) { terminarMini(1); return; }
            break;
        case MINI_SINCRONIA:
            g->marcador += g->dir * g->vel * dt;
            if (g->marcador > 1.0f) { g->marcador = 1.0f; g->dir = -1.0f; }
            if (g->marcador < 0.0f) { g->marcador = 0.0f; g->dir = 1.0f; }
            if (IsKeyPressed(KEY_SPACE) || clicou(btnSinc)) {
                if (fabsf(g->marcador - g->zona) < g->larg / 2) {
                    g->acertos++;
                    if (g->acertos >= 3) { terminarMini(1); return; }
                    g->zona = lim(GetRandomValue(15, 85) / 100.0f, g->larg / 2 + 0.02f, 1.0f - g->larg / 2 - 0.02f);
                } else {
                    g->flash = 0.4f;
                    g->tempo -= 2.0f;
                }
            }
            break;
        case MINI_FAISCAS: {
            Rectangle a = arenaFaiscas();
            if (g->invul > 0) g->invul -= dt;

            /* o escudo segue o mouse, preso dentro da arena */
            g->escudo.x = lim(m.x, a.x + RAIO_ESCUDO, a.x + a.width - RAIO_ESCUDO);
            g->escudo.y = lim(m.y, a.y + RAIO_ESCUDO, a.y + a.height - RAIO_ESCUDO);

            for (int i = 0; i < g->nFaiscas; i++) {
                g->fPos[i].x += g->fVel[i].x * dt;
                g->fPos[i].y += g->fVel[i].y * dt;

                /* colisao faisca x paredes da arena: reflete a velocidade */
                if (g->fPos[i].x < a.x + RAIO_FAISCA) { g->fPos[i].x = a.x + RAIO_FAISCA; g->fVel[i].x = fabsf(g->fVel[i].x); }
                if (g->fPos[i].x > a.x + a.width - RAIO_FAISCA) { g->fPos[i].x = a.x + a.width - RAIO_FAISCA; g->fVel[i].x = -fabsf(g->fVel[i].x); }
                if (g->fPos[i].y < a.y + RAIO_FAISCA) { g->fPos[i].y = a.y + RAIO_FAISCA; g->fVel[i].y = fabsf(g->fVel[i].y); }
                if (g->fPos[i].y > a.y + a.height - RAIO_FAISCA) { g->fPos[i].y = a.y + a.height - RAIO_FAISCA; g->fVel[i].y = -fabsf(g->fVel[i].y); }

                /* colisao faisca x escudo (circulo contra circulo): a faisca e rebatida */
                float d = dist(g->fPos[i], g->escudo);
                if (d < RAIO_ESCUDO + RAIO_FAISCA) {
                    float vel = hypotf(g->fVel[i].x, g->fVel[i].y);
                    float nx = d > 0.001f ? (g->fPos[i].x - g->escudo.x) / d : 1.0f;
                    float ny = d > 0.001f ? (g->fPos[i].y - g->escudo.y) / d : 0.0f;
                    g->fVel[i] = (Vector2){nx * vel, ny * vel};
                    g->fPos[i].x = g->escudo.x + nx * (RAIO_ESCUDO + RAIO_FAISCA);
                    g->fPos[i].y = g->escudo.y + ny * (RAIO_ESCUDO + RAIO_FAISCA);
                    if (g->invul <= 0) {
                        g->golpes++;
                        g->invul = 1.0f;
                        g->flash = 0.4f;
                        tocar(sndTick);
                        if (g->golpes >= MAX_GOLPES) { terminarMini(2); return; }
                    }
                }
            }
            if (g->tempo <= 0) { terminarMini(1); return; }
            break;
        }
        default:
            g->alvoT -= dt;
            if (g->alvoT <= 0) {
                g->tempo -= 1.0f;
                g->flash = 0.3f;
                novoAlvo();
            }
            if (press) {
                for (int s = 0; s < 6; s++) {
                    if (!CheckCollisionPointRec(m, discRect(s))) continue;
                    if (s == g->alvo) {
                        g->acertos++;
                        tocar(sndClique);
                        if (g->acertos >= 8) { terminarMini(1); return; }
                        novoAlvo();
                    } else {
                        g->flash = 0.3f;
                        g->tempo -= 1.5f;
                    }
                    break;
                }
            }
            break;
    }
    if (g->tipo != MINI_TURBINAS && g->tipo != MINI_FAISCAS && g->tempo <= 0 && !g->resultado) terminarMini(2);
}
