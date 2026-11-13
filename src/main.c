#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define W 1280
#define H 720
#define N 5
#define FASES 5
#define DIAS 4
#define DIA_SEG 30.0f
#define LIM_BAIXO 50.0f
#define LIM_ALTO 85.0f
#define TEMPO_ALERTA 10.0f
#define NUM_EV 9
#define MAX_EV 5

enum {NORTE, NORDESTE, CENTRO, SUDESTE, SUL};
typedef enum { TELA_MENU, TELA_AJUDA, TELA_JOGO, TELA_MINI, TELA_FIM } Tela;
typedef enum { MINI_FIOS, MINI_DISJUNTOR, MINI_TURBINAS, MINI_SINCRONIA } TipoMini;

typedef struct {
    const char *nome;
    Color cor;
    int fonte;
    Vector2 pt[56];
    int n;
    Vector2 c;
    float bat, ajuste, consBase;
    float multProd, multCons;
    bool ativa, inBlack, inSobre;
    float tBlack, tSobre;
} Regiao;

typedef struct {
    const char *nome;
    float prod[N];
    float cons[N];
} EvDef;

typedef struct { int id; float t, dur; } EvAtivo;

typedef struct {
    TipoMini tipo;
    int reg;
    float tempo, tempoMax;
    /* fios */
    int ordem[4];
    bool ligados[4];
    int destino[4];
    int arrasta;
    /* disjuntores */
    int slot[6];
    int proximo;
    float flash;
    /* turbinas */
    float calor[4], taxa[4];
    /* sincronia */
    float marcador, dir, zona, larg;
    int acertos;
} Minigame;

/* ---------- Dados ---------- */

static const char *abrev[N] = {"Norte", "Nordeste", "C.Oeste", "Sudeste", "Sul"};
static const int ordemFase[N] = {SUDESTE, NORDESTE, CENTRO, SUL, NORTE};
static const Color fios[4] = {     /*cores fios ( Vermelho | Azul | Amarelo | Verde )*/
    {235, 70, 70, 255}, {70, 140, 255, 255}, {255, 220, 50, 255}, {65, 220, 110, 255}
};

/*eventos aleatórios do seu jogo, cada evento possui um nome e dois conjuntos de modificadores numéricos: */
static const EvDef evDefs[NUM_EV] = {
    {"Chuvas fortes",     {0.30f, 0.00f, -0.10f, 0.12f, 0.28f}, {0.00f, 0.08f, 0.00f, 0.00f, 0.00f}},
    {"Seca severa",       {-0.40f, 0.10f, 0.00f, -0.25f, -0.30f}, {0.00f, 0.00f, 0.10f, 0.00f, 0.00f}},
    {"Onda de calor",     {0.00f, 0.12f, 0.30f, 0.00f, 0.00f}, {0.05f, 0.30f, 0.20f, 0.28f, 0.10f}},
    {"Ventos fortes",     {0.00f, 0.45f, 0.00f, 0.00f, 0.20f}, {0.08f, 0.00f, 0.00f, 0.00f, 0.00f}},
    {"Frente fria",       {0.00f, -0.15f, 0.00f, 0.00f, 0.00f}, {0.08f, 0.00f, 0.00f, 0.20f, 0.38f}},
    {"Jogo decisivo",     {0.00f, 0.00f, 0.00f, 0.00f, 0.00f}, {0.00f, 0.20f, 0.00f, 0.40f, 0.10f}},
    {"Manutencao nuclear",{0.00f, 0.00f, 0.00f, -0.45f, 0.00f}, {0.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
    {"Sol forte",         {0.00f, 0.05f, 0.35f, 0.05f, 0.10f}, {0.00f, 0.00f, 0.00f, 0.00f, 0.00f}},
    {"Pico industrial",   {0.00f, 0.00f, 0.00f, 0.00f, 0.00f}, {0.00f, 0.10f, 0.12f, 0.35f, 0.25f}}
};

/* ----------- Contornos Reais aproximados: longitude e latitude ----------- */
static const float geoNorte[][2] = {
    {-73.8f,-7.4f},{-72.8f,-5.2f},{-70.1f,-4.2f},{-70.0f,-2.0f},{-69.4f,-0.8f},{-69.6f,0.7f},
    {-67.3f,2.2f},{-65.4f,1.5f},{-64.0f,4.1f},{-61.5f,4.5f},{-60.7f,5.2f},{-59.9f,4.0f},
    {-60.0f,2.0f},{-58.5f,1.3f},{-56.5f,1.9f},{-54.6f,2.3f},{-52.9f,2.2f},{-51.7f,4.2f},
    {-50.8f,2.0f},{-50.0f,0.6f},{-50.6f,-0.4f},{-49.0f,-1.2f},{-47.3f,-0.9f},{-46.2f,-1.1f},
    {-46.8f,-3.0f},{-48.3f,-5.2f},{-47.7f,-5.8f},{-46.6f,-8.5f},{-46.1f,-10.5f},{-46.3f,-12.5f},
    {-50.2f,-13.4f},{-50.5f,-11.0f},{-50.2f,-9.2f},{-52.0f,-9.0f},{-55.0f,-7.5f},{-58.0f,-8.8f},
    {-60.0f,-10.0f},{-60.5f,-13.5f},{-62.5f,-13.0f},{-65.3f,-9.8f},{-67.5f,-10.3f},
    {-70.5f,-11.0f},{-72.5f,-9.5f}
};
static const float geoNordeste[][2] = {
    {-46.2f,-1.1f},{-46.8f,-3.0f},{-48.3f,-5.2f},{-47.7f,-5.8f},{-46.6f,-8.5f},{-46.1f,-10.5f},
    {-46.3f,-12.5f},{-46.0f,-14.0f},{-44.2f,-15.5f},{-43.5f,-16.5f},{-41.8f,-17.9f},{-39.7f,-18.3f},
    {-39.2f,-16.5f},{-38.9f,-15.0f},{-38.5f,-13.0f},{-37.3f,-11.0f},{-35.8f,-9.8f},{-35.2f,-8.5f},
    {-34.85f,-7.2f},{-34.9f,-6.0f},{-35.3f,-5.2f},{-36.5f,-5.1f},{-37.2f,-4.8f},{-38.5f,-3.7f},
    {-40.0f,-2.9f},{-41.5f,-2.9f},{-42.8f,-2.7f},{-44.0f,-2.5f},{-44.8f,-1.6f}
};
static const float geoCentro[][2] = {
    {-60.5f,-13.5f},{-60.0f,-10.0f},{-58.0f,-8.8f},{-55.0f,-7.5f},{-52.0f,-9.0f},{-50.2f,-9.2f},
    {-50.5f,-11.0f},{-50.2f,-13.4f},{-46.3f,-12.5f},{-46.0f,-14.0f},{-46.8f,-16.0f},{-47.2f,-17.0f},
    {-48.2f,-18.3f},{-50.0f,-19.4f},{-51.0f,-20.2f},{-51.9f,-21.2f},{-52.9f,-22.4f},{-53.1f,-22.7f},
    {-54.6f,-24.0f},{-55.4f,-23.7f},{-55.8f,-22.2f},{-57.9f,-22.0f},{-57.7f,-20.0f},{-58.2f,-17.8f},
    {-60.2f,-16.3f},{-60.2f,-15.0f}
};
static const float geoSudeste[][2] = {
    {-46.0f,-14.0f},{-44.2f,-15.5f},{-43.5f,-16.5f},{-41.8f,-17.9f},{-39.7f,-18.3f},{-39.8f,-19.8f},
    {-40.9f,-21.0f},{-41.0f,-22.0f},{-42.0f,-22.9f},{-44.0f,-23.0f},{-45.4f,-23.8f},{-47.0f,-24.8f},
    {-48.1f,-25.0f},{-49.5f,-24.2f},{-51.5f,-22.9f},{-53.1f,-22.7f},{-52.9f,-22.4f},{-51.9f,-21.2f},
    {-51.0f,-20.2f},{-50.0f,-19.4f},{-48.2f,-18.3f},{-47.2f,-17.0f},{-46.8f,-16.0f}
};
static const float geoSul[][2] = {
    {-48.1f,-25.0f},{-49.5f,-24.2f},{-51.5f,-22.9f},{-53.1f,-22.7f},{-54.6f,-24.0f},{-54.6f,-25.5f},
    {-53.8f,-26.2f},{-54.2f,-27.3f},{-56.0f,-28.5f},{-57.6f,-30.2f},{-56.0f,-30.6f},{-54.6f,-31.5f},
    {-53.6f,-32.9f},{-53.4f,-33.7f},{-52.2f,-32.2f},{-50.8f,-30.5f},{-49.7f,-29.0f},{-48.8f,-27.8f},
    {-48.6f,-26.3f}
};

/* --------- Estado Global ------------ */

static Regiao r[N];
static EvAtivo evs[MAX_EV];
static int nev = 0;
static Tela tela = TELA_MENU;
static Minigame mini;
static int fase, dia, selecionada, arrasta, vidas, streak, pontos;
static float tempoDia, tempoTotal, spawnT, cooldown, msgT, pontosAcum;
static float fluxoT = 0;
static int fluxoO = 0, fluxoD = 0;
static char msg[200] = "";
static bool pausa = false, vitoria = false, sair = false;
static int ranking[5] = {0, 0, 0, 0, 0};
static int posRanking = 0;
static int paginaAjuda = 0;

/* Botoes */
static const Rectangle btnJogar = {540, 300, 200, 52};
static const Rectangle btnComo = {540, 370, 200, 52};
static const Rectangle btnSair = {540, 440, 200, 52};
static const Rectangle btnAnt = {330, 640, 170, 44};
static const Rectangle btnProx = {780, 640, 170, 44};
static const Rectangle btnVoltar = {555, 640, 170, 44};
static const Rectangle btnContinuar = {540, 330, 200, 50};
static const Rectangle btnMenuPausa = {540, 400, 200, 50};
static const Rectangle btnFimMenu = {540, 540, 200, 50};
static const Rectangle btnMenos = {950, 590, 50, 34};
static const Rectangle btnMais = {1010, 590, 50, 34};

/* ---------------- Utilidades ---------------- */

static float projX(float lon) { return 290.0f + (lon + 74.0f) * 15.2f; }
static float projY(float lat) { return 95.0f + (5.5f - lat) * 15.2f; }
static float velFase(void) { return 1.0f + 0.3f * (fase - 1); }

static void criarPoli(Regiao *g, const float (*d)[2], int n)
{
    g->n = n;
    for (int i=0; i<n; i++) g->pt[i] = (Vector2){projX(d[i][0]), projY(d[i][1])};
}

static void textoCentro(const char *s, int x, int y, int tam, Color c)
{
    DrawText(s, x - MeasureText(s, tam) / 2, y, tam, c);
}

static void mensagem(const char *s, float t)
{
    snprintf(msg, sizeof(msg), "%s", s);
    msgT = t;
}

static bool clicou(Rectangle rc)
{
    return IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), rc);
}

static void desenharBotao(Rectangle rc, const char *t)
{
    bool hover = CheckCollisionPointRec(GetMousePosition(), rc);
    DrawRectangleRec(rc, hover ? (Color){70, 120, 190, 255} : (Color){40, 70, 120, 255});
    DrawRectangleLinesEx(rc, 2, hover ? WHITE : (Color){120, 160, 220, 255});
    DrawText(t, (int)(rc.x + rc.width / 2) - MeasureText(t, 22) / 2, (int)(rc.y + rc.height / 2) - 11, 22, WHITE);
}

static bool dentro(Vector2 p, const Vector2 *v, int n)
{
    bool ok = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if(((v[i].y > p.y) != (v[j] > p.y)) && 
            p.x < (v[j].x - v[i].x) * (p.y - v[i].y) / (v[j].y - v[i].y) + v[i].x) ok = !ok;
    }

    return ok;
}

static int regiaoSob(Vector2 m)
{
    for (int i = 0; i < N; i++)
        if (r[i].ativa && dentro(m, r[i].pt, r[i].n)) return i;
    return -1;
}

static void preencher(const Vector2 *v, int n, Color c)
{
    float miny = v[0].y, maxy = v[0].y;
    for (int i = 1; i < n; i++) {
        if (v[i].y < miny) miny = v[i].y;
        if (v[i].y > maxy) maxy = v[i].y;
    }
    for (int y = (int)miny; y <= (int)maxy; y++) {
        float xs[64];
        int k = 0;
        float fy = y + 0.5f;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            if (((v[i].y > fy) != (v[j].y > fy)) && k < 64) {
                xs[k++] = v[j].x + (fy - v[j].y) * (v[i].x - v[j].x) / (v[i].y - v[j].y);
            }
        }
        for (int a = 1; a < k; a++) {
            float t = xs[a];
            int b = a - 1;
            while (b >= 0 && xs[b] > t) { xs[b + 1] = xs[b]; b--; }
            xs[b + 1] = t;
        }
        for (int a = 0; a + 1 < k; a += 2)
            DrawRectangle((int)xs[a], y, (int)(xs[a + 1] - xs[a]) + 1, 1, c);
    }
}

static void icone(int f, float x, float y, float t)
{
    int ix = (int)x, iy = (int)y;
    switch (f) {
        case 0:
            DrawRectangle(ix - 16, iy - 9, 32, 18, (Color){70, 80, 100, 255});
            DrawRectangle(ix - 20, iy + 9, 40, 5, (Color){50, 150, 245, 255});
            for (int i = 0; i < 3; i++) {
                float d = fmodf(t * 22.0f + i * 7.0f, 14.0f);
                DrawCircle(ix - 9 + i * 9, (int)(y - 3 + d), 2, SKYBLUE);
            }
            break;
        case 1: {
            float a0 = t * 120.0f;
            DrawLineEx((Vector2){x, y - 4}, (Vector2){x, y + 20}, 3, RAYWHITE);
            for (int i = 0; i < 3; i++) {
                float a = (a0 + i * 120.0f) * DEG2RAD;
                DrawLineEx((Vector2){x, y - 4}, (Vector2){x + cosf(a) * 20, y - 4 + sinf(a) * 20}, 3, RAYWHITE);
            }
            DrawCircle(ix, iy - 4, 4, YELLOW);
            break;
        }
        case 2:
            DrawCircle(ix + 13, iy - 12, 7, YELLOW);
            DrawRectangle(ix - 20, iy - 2, 34, 20, (Color){20, 55, 125, 255});
            DrawLine(ix - 20, iy + 8, ix + 14, iy + 8, SKYBLUE);
            DrawLine(ix - 6, iy - 2, ix - 6, iy + 18, SKYBLUE);
            DrawLine(ix + 4, iy - 2, ix + 4, iy + 18, SKYBLUE);
            break;
        default:
            DrawRectangle(ix - 13, iy - 6, 26, 22, LIGHTGRAY);
            DrawRectangle(ix - 9, iy - 14, 18, 8, LIGHTGRAY);
            DrawCircle(ix + 14, iy - 18 - (int)(fmodf(t * 8.0f, 6.0f)), 5, Fade(WHITE, 0.7f));
            DrawCircle(ix + 4, iy - 22, 4, Fade(WHITE, 0.5f));
            break;
    }
}

static Color CorBat(float b)
{
    if (b < LIM_BAIXO) return RED;
    if (b < LIM_ALTO + 10) return ORANGE;
    if (b > LIM_ALTO) return (Color){255, 120, 60, 255};
    return (Color){80, 220, 150, 255};
}

static int regioesNaFase(int f) { return f <= 1 ? 3 : (f == 2 ? 4 : 5); }

static int totalAtivas(void)
{
    int c = 0;
    for (int i = 0; i < N; i++) if (r[i].ativa) c++;
    return c;
}

/* ---------------- Ranking ---------------- */

static void carregarRanking(void)
{
    FILE *f = fopen("ranking.txt", "r");
    if (!f) return;
    for (int i = 0; i < 5; i++) if (fscanf(f, "%d", &ranking[i]) != 1) ranking[i] = 0;
    fclose(f);
}

static void registrarRanking(void)
{
    posRanking = 0;
    for (int i = 0; i < 5; i++) {
        if (pontos > ranking[i]) {
            for (int j = 4; j > i; j--) ranking[j] = ranking[j - 1];
            ranking[i] = pontos;
            posRanking = i + 1;
            break;
        }
    }
    FILE *f = fopen("ranking.txt", "w");
    if (!f) return;
    for (int i = 0; i < 5; i++) fprintf(f, "%d\n", ranking[i]);
    fclose(f);
}

static void fimDePartida(bool venceu)
{
    vitoria = venceu;
    registrarRanking();
    estado = FIM;
}

/* ---------- Inicializacao ---------- */

static void setReg(int id, const char *nome, Color cor, int fonte, float cons, float lon, float lat)
{
    r[id].nome = nome;
    r[id].cor = cor;
    r[id].fonte = fonte;
    r[id].consBase = cons;
    r[id].c = (Vector2){projX(lon), projY(lat)};
}

static void iniciarRegioes(void)
{
    memset(r, 0, sizeof(r));
    setReg(NORTE, "Norte", (Color){46, 150, 95, 255}, 0, 0.92f, -62.0f, -4.5f);
    setReg(NORDESTE, "Nordeste", (Color){225, 175, 40, 255}, 1, 1.00f, -42.5f, -8.0f);
    setReg(CENTRO, "Centro-Oeste", (Color){215, 115, 45, 255}, 2, 0.93f, -54.0f, -14.5f);
    setReg(SUDESTE, "Sudeste", (Color){155, 90, 170, 255}, 3, 1.05f, -45.0f, -19.0f);
    setReg(SUL, "Sul", (Color){65, 110, 230, 255}, 0, 0.95f, -52.0f, -27.0f);

    criarPoli(&r[NORTE], geoNorte, (int)(sizeof(geoNorte) / sizeof(geoNorte[0])));
    criarPoli(&r[NORDESTE], geoNordeste, (int)(sizeof(geoNordeste) / sizeof(geoNordeste[0])));
    criarPoli(&r[CENTRO], geoCentro, (int)(sizeof(geoCentro) / sizeof(geoCentro[0])));
    criarPoli(&r[SUDESTE], geoSudeste, (int)(sizeof(geoSudeste) / sizeof(geoSudeste[0])));
    criarPoli(&r[SUL], geoSul, (int)(sizeof(geoSul) / sizeof(geoSul[0])));

    for (int i = 0; i < N; i++) {
        r[i].bat = 70.0f;
        r[i].ajuste = 1.0f;
        r[i].multProd = 1.0f;
        r[i].multCons = 1.0f;
    }
}

static void ativarFase(void)
{
    int qtd = regioesNaFase(fase);
    for (int k = 0; k < qtd; k++) {
        int id = ordemFase[k];
        if (!r[id].ativa) { r[id].ativa = true; r[id].bat = 70.0f; }
    }
}

static void iniciarPartida(void)
{
    iniciarRegioes();
    fase = 1; dia = 1; vidas = 3; streak = 0; pontos = 0;
    origem = SUDESTE; destino = NORDESTE;
    tempoDia = 0; tempoTotal = 0; spawnT = 4.0f; cooldown = 0; pontosAcum = 0;
    nev = 0; pausa = false; vitoria = false;
    ativarFase();
    mensagem("Mantenha cada regiao entre 50% e 85%!", 5.0f);
    estado = JOGO;
}

/* ---------- Eventos --------------*/

static bool eventoAfeta(int id)
{
    for (int i=0; i<N; i++){
        if(!r[i].ativa) continue;
        if (fabsf(evDefs[id].prod[i]) > 0.05f || fabsf(evDefs[id].cons[i]) > 0.05f) return True;
    }
    return false;
}

static void spawnEvento(void){
    int id = -1;
    for (int tent = 0; tent <30 && id < 0; tent++){
        int c = GetRandomValue(0, NUM_EV - 1);
        bool repetido = false;
        for (int i=0; i< nev; i++) if (evs[i].id == c) repetido = true;
        if(!repetido && eventoAfeta(c)) id = c;
    }
    if(id < 0){ spawnT = 3.0f; return;}

    evs[nev].id = id;
    evs[nev].dur = 22.0f;
    evs[nev].t = 22.0f;
    nev++;

    char buf[120];
    snprintf(buf, sizeof(buf), "Novo evento: %d!", evDefs[id].nome);
    mensagem(buf, 4.0f);

    spawnT = 12.0f - 1.8f * (fase - 1) + GetRandomValue(0, 30)/10.0f;
}

static void atualizarMultiplicadores(void)
{
    float intens = 1.0f + 0.2f * (fase - 1);
    for (int i = 0; i < N; i++) {
        float mp = 1.0f, mc = 1.0f;
        for (int e = 0; e < nev; e++) {
            mp += evDefs[evs[e].id].prod[i] * intens;
            mc += evDefs[evs[e].id].cons[i] * intens;
        }
        mc *= 1.0f + 0.05f * sinf(tempoTotal * 0.4f + i * 1.7f);
        if (mp < 0.2f) mp = 0.2f;
        if (mc < 0.3f) mc = 0.3f;
        r[i].multProd = mp;
        r[i].multCons = mc;
    }
}

/* ---------------- Minigames ---------------- */

static bool ehBlackout(void) { return mini.tipo == MINI_FIOS || mini.tipo == MINI_DISJUNTOR; }

static Vector2 noEsq(int i) { return (Vector2){435, 300 + i * 65}; }
static Vector2 noDir(int i) { return (Vector2){845, 300 + i * 65}; }
static Vector2 centroSlot(int s) { return (Vector2){440.0f + (s % 3) * 200.0f, 330.0f + (s / 3) * 120.0f}; }

static void iniciarMinigame(int id, bool blackout)
{
    memset(&mini, 0, sizeof(mini));
    mini.reg = id;
    mini.arrasta = -1;
    arrasta = -1;
    float vel = velFase();
    int variante = GetRandomValue(0, 1);

    if (blackout) mini.tipo = variante ? MINI_DISJUNTOR : MINI_FIOS;
    else mini.tipo = variante ? MINI_SINCRONIA : MINI_TURBINAS;

    switch (mini.tipo) {
        case MINI_FIOS:
            mini.tempoMax = 18.0f - 2.0f * (fase - 1);
            for (int i = 0; i < 4; i++) { mini.ordem[i] = i; mini.destino[i] = -1; }
            for (int i = 3; i > 0; i--) {
                int k = GetRandomValue(0, i);
                int t = mini.ordem[i];
                mini.ordem[i] = mini.ordem[k];
                mini.ordem[k] = t;
            }
            break;
        case MINI_DISJUNTOR:
            mini.tempoMax = 13.0f - 1.2f * (fase - 1);
            for (int i = 0; i < 6; i++) mini.slot[i] = i + 1;
            for (int i = 5; i > 0; i--) {
                int k = GetRandomValue(0, i);
                int t = mini.slot[i];
                mini.slot[i] = mini.slot[k];
                mini.slot[k] = t;
            }
            mini.proximo = 1;
            break;
        case MINI_TURBINAS:
            mini.tempoMax = 12.0f;
            for (int i = 0; i < 4; i++) {
                mini.calor[i] = 20.0f + GetRandomValue(0, 250) / 10.0f;
                mini.taxa[i] = (6.0f + GetRandomValue(0, 50) / 10.0f) * vel;
            }
            break;
        case MINI_SINCRONIA:
            mini.tempoMax = 16.0f;
            mini.marcador = 0.0f;
            mini.dir = 1.0f;
            mini.zona = 0.5f;
            mini.larg = 0.22f - 0.015f * (fase - 1);
            break;
    }
    mini.tempo = mini.tempoMax;
    tela = TELA_MINI;
}

static void resolverMinigame(bool sucesso)
{
    Regiao *g = &r[mini.reg];
    bool bl = ehBlackout();
    g->inBlack = false; g->inSobre = false;
    g->tBlack = 0; g->tSobre = 0;
    arrasta = -1;

    if (sucesso) {
        if (bl) g->bat = 62.0f;
        else { g->bat = 76.0f; g->ajuste = 0.85f; }
        pontos += 100 + 50 * fase;
        mensagem("Crise resolvida! Bonus de pontos", 4.0f);
        tela = TELA_JOGO;
    } else {
        vidas--;
        g->bat = bl ? 58.0f : 76.0f;
        if (!bl) g->ajuste = 0.85f;
        if (vidas <= 0) { fimDePartida(false); return; }
        mensagem("Falhou! Voce perdeu uma vida da rede.", 4.0f);
        tela = TELA_JOGO;
    }
}

static void atualizarMinigame(float dt)
{
    Vector2 mouse = GetMousePosition();
    mini.tempo -= dt;
    if (mini.flash > 0) mini.flash -= dt;

    switch (mini.tipo) {
        case MINI_FIOS: {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (int i = 0; i < 4; i++)
                    if (!mini.ligados[i] && CheckCollisionPointCircle(mouse, noEsq(i), 22)) mini.arrasta = i;
            }
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && mini.arrasta >= 0) {
                int i = mini.arrasta;
                for (int j = 0; j < 4; j++) {
                    if (CheckCollisionPointCircle(mouse, noDir(j), 26) && mini.ordem[j] == i) {
                        mini.ligados[i] = true;
                        mini.destino[i] = j;
                    }
                }
                mini.arrasta = -1;
            }
            bool pronto = true;
            for (int i = 0; i < 4; i++) if (!mini.ligados[i]) pronto = false;
            if (pronto) resolverMinigame(true);
            else if (mini.tempo <= 0) resolverMinigame(false);
            break;
        }
        case MINI_DISJUNTOR: {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (int s = 0; s < 6; s++) {
                    if (CheckCollisionPointCircle(mouse, centroSlot(s), 38)) {
                        if (mini.slot[s] == mini.proximo) mini.proximo++;
                        else if (mini.slot[s] > mini.proximo) { mini.tempo -= 1.5f; mini.flash = 0.3f; }
                    }
                }
            }
            if (mini.proximo > 6) resolverMinigame(true);
            else if (mini.tempo <= 0) resolverMinigame(false);
            break;
        }
        case MINI_TURBINAS: {
            bool falhou = false;
            for (int i = 0; i < 4; i++) {
                mini.calor[i] += mini.taxa[i] * dt;
                Rectangle rc = {380.0f + i * 140.0f, 270, 100, 200};
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, rc)) {
                    mini.calor[i] -= 22.0f;
                    if (mini.calor[i] < 0) mini.calor[i] = 0;
                }
                if (mini.calor[i] >= 100.0f) falhou = true;
            }
            if (falhou) resolverMinigame(false);
            else if (mini.tempo <= 0) resolverMinigame(true);
            break;
        }
        case MINI_SINCRONIA: {
            mini.marcador += mini.dir * 0.9f * velFase() * dt;
            if (mini.marcador > 1.0f) { mini.marcador = 1.0f; mini.dir = -1.0f; }
            if (mini.marcador < 0.0f) { mini.marcador = 0.0f; mini.dir = 1.0f; }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE)) {
                if (fabsf(mini.marcador - mini.zona) <= mini.larg / 2.0f) {
                    mini.acertos++;
                    mini.zona = 0.15f + GetRandomValue(0, 70) / 100.0f;
                } else {
                    mini.tempo -= 2.0f;
                    mini.flash = 0.3f;
                }
            }
            if (mini.acertos >= 4) resolverMinigame(true);
            else if (mini.tempo <= 0) resolverMinigame(false);
            break;
        }
    }
}

/*--------------- Logica do Jogo ---------------*/

static void fimDoDia(void){
    bool ok = true;
    for (int i=0; i<N; i++){
        if(!r[i].ativa) continue;
        if(r[i].bat < LIM_BAIXO || r[i].bat > LIM_ALTO || r[i].inBlack || r[i].inSobre) ok = false;
    }

    if (ok){
        streak++; // +sequencia dias bons seguidos//
        int s = streak > 10 ? 10: streak;
        int ganho = (int)((300 * fase + 100 * totalAtivas()) *(10 + s)/10.0f);
        pontos += ganho;
        char buf[120];
        snprintf(buf, sizeof(buf), "Dia equilibrado! +%d pontos (sequencia x%d)", ganho, streak); // aqui feito para nao estourar o tamanho //
        mensagem(buf, 4.0f); // 4Seg
    } else {
        streak = 0;
        mensagem("Dia fora da faixa segura: sem bonus, sequencia zerada.", 4.0f);
    }

    // reinicia o relogio do dia e avanca +1 dia //
    tempoDia = 0;
    dia++;
    if (dia > DIAS){
        dia = 1;
        fase++;
        //caso o jog venca -> fimDePartida -> return para impedir de continuar //
        if (fase > FASES) { fimDePartida(true); return; }
        ativarFase();
        char buf[120];
        snprintf(buf, sizeof(buf), "FASE %d: eventos mais fortes e mais frequentes!", fase);
        mensagem(buf, 5.0f);
    }
}

static void enviarApoio(int o, int d)
{
    if (cooldown > 0) { mensagem("Linha de transmissao carregando...", 2.0f); return; }
    if (r[o].bat < 25.0f) { mensagem("Origem sem energia suficiente!", 3.0f); return; }

    r[o].bat -= 10.0f;
    r[d].bat += 8.0f;
    cooldown = 0.8f;
    fluxoO = o; fluxoD = d; fluxoT = 0.7f;

    char buf[120];
    snprintf(buf, sizeof(buf), "Apoio: %s -> %s", r[o].nome, r[d].nome);
    mensagem(buf, 2.5f);
}

static void ajustarProducao(float delta)
{
    Regiao *g = &r[selecionada];
    g->ajuste += delta;
    if (g->ajuste < 0.3f) g->ajuste = 0.3f;
    if (g->ajuste > 1.7f) g->ajuste = 1.7f;
}

// CONTINUE A PARTIR DAQUI

// roda todo frame durante jogo -> dt = tempo em Seg //
static void atualizarJogo(float dt){
    Vector2 mouse = GetMousePosition();

    if (IsKeyPressed(KEY_P)) pausa = !pausa;
    if (pausa) return;

    // 3 relogios:  tempo total do jogo; tempo restante mensagem tela; tempo restante do recarregamento de apio // 
    tempoTotal += dt;
    if (msgT > 0) msgT -= dt;
    if (cooldown > 0) cooldown -= dt;

    // Selecao: botao esquerdo = ORIGEM, direito = DESTINO //
    for (int i = 0; i < N; i++) {
        if (!r[i].ativa || !dentro(mouse, r[i].pt, r[i].n)) continue;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) origem = i;
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) destino = i;
    }
    if (IsKeyPressed(KEY_TAB)) { int t = origem; origem = destino; destino = t; }

    // Producao da regiao de origem //
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) r[origem].ajuste += dt * 0.8f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) r[origem].ajuste -= dt * 0.8f;
    if (IsKeyPressed(KEY_R)) r[origem].ajuste = 1.0f;
    if (r[origem].ajuste < 0.3f) r[origem].ajuste = 0.3f;
    if (r[origem].ajuste > 1.7f) r[origem].ajuste = 1.7f;

    // Enviar apoio //
    if (IsKeyPressed(KEY_E)) {
        if (origem == destino) mensagem("Escolha um destino diferente (botao direito).", 3.0f);
        else if (cooldown > 0) mensagem("Linha de transmissao carregando...", 2.0f);
        else if (r[origem].bat < 20.0f) mensagem("Origem sem energia suficiente!", 3.0f);
        else {
            r[origem].bat -= 10.0f;
            r[destino].bat += 8.0f;
            cooldown = 1.2f;
            char buf[120];
            snprintf(buf, sizeof(buf), "Apoio: %s -> %s", r[origem].nome, r[destino].nome);
            mensagem(buf, 2.5f);
        }
    }

    // Eventos  - remove eventos expirados//
    for (int e = 0; e < nev; e++) evs[e].t -= dt;
    for (int e = 0; e < nev;) {
        if (evs[e].t <= 0) {
            for (int k = e; k < nev - 1; k++) evs[k] = evs[k + 1];
            nev--;
        } else e++;
    }
    spawnT -= dt;
    if (spawnT <= 0) {
        if (nev < MAX_EV) spawnEvento();
        else spawnT = 3.0f;
    }
    atualizarMultiplicadores();

    // Baterias, alertas e pontos por segundo //
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        if (!g->ativa) continue;

        // saldo é producao menos consumo. saldo+ carrega bateria e - descarrega//
        float saldo = g->ajuste * g->multProd - g->consBase * g->multCons;
        g->bat += saldo * 4.0f * dt;
        if (g->bat < 0) g->bat = 0;
        if (g->bat > 105) g->bat = 105;

        //O alerta de blackout liga abaixo de 50%, mas só desliga quando passa de 53%
        if (!g->inBlack && g->bat < LIM_BAIXO) { g->inBlack = true; g->tBlack = 0; }
        else if (g->inBlack && g->bat >= LIM_BAIXO + 3) g->inBlack = false;

        if (!g->inSobre && g->bat > LIM_ALTO) { g->inSobre = true; g->tSobre = 0; }
        else if (g->inSobre && g->bat <= LIM_ALTO - 3) g->inSobre = false;

        if (g->bat >= LIM_BAIXO && g->bat <= LIM_ALTO) pontosAcum += fase * dt;

        //Com o alerta ligado, conta o tempo. Ao chegar em 10s (TEMPO_ALERTA), abre o minigame e dá return//
        if (g->inBlack) {
            g->tBlack += dt;
            if (g->tBlack >= TEMPO_ALERTA) { iniciarMinigame(i, MINI_BLACKOUT); return; }
        }
        if (g->inSobre) {
            g->tSobre += dt;
            if (g->tSobre >= TEMPO_ALERTA) { iniciarMinigame(i, MINI_SOBRECARGA); return; }
        }
    }
    if (pontosAcum >= 1.0f) { pontos += (int)pontosAcum; pontosAcum -= (int)pontosAcum; }

    tempoDia += dt;
    if (tempoDia >= DIA_SEG) fimDoDia();
}


// Continuar Aqui: (refinamento desenho mundo) //
//

/* ------------- Desenho ------------- */

static void desenharMapa(void)
{
    for (int i = 0; i < N; i++) {
        Color base = r[i].ativa ? r[i].cor : (Color){50, 58, 70, 255};
        preencher(r[i].pt, r[i].n, base);
        if (r[i].ativa && r[i].inBlack) {
            float p = 0.25f + 0.2f * sinf(tempoTotal * 9.0f);
            preencher(r[i].pt, r[i].n, Fade(RED, p));
        } else if (r[i].ativa && r[i].inSobre) {
            float p = 0.25f + 0.2f * sinf(tempoTotal * 9.0f);
            preencher(r[i].pt, r[i].n, Fade(ORANGE, p));
        }
    }
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < r[i].n; k++)
            DrawLineEx(r[i].pt[k], r[i].pt[(k + 1) % r[i].n], 2.0f, (Color){15, 25, 40, 255});
    }

    /* Linha de apoio */
    if (origem != destino && r[origem].ativa && r[destino].ativa) {
        Vector2 a = r[origem].c, b = r[destino].c;
        DrawLineEx(a, b, 2.0f, Fade(YELLOW, 0.5f));
        for (int k = 0; k < 4; k++) {
            float t = fmodf(tempoTotal * 0.8f + k * 0.25f, 1.0f);
            DrawCircleV((Vector2){a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t}, 4, YELLOW);
        }
    }

    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        int cx = (int)g->c.x, cy = (int)g->c.y;

        if (!g->ativa) {
            textoCentro(g->nome, cx, cy - 14, 14, GRAY);
            textoCentro("BLOQUEADA", cx, cy + 4, 12, GRAY);
            continue;
        }

        icone(g->fonte, g->c.x, g->c.y - 18, tempoTotal);
        textoCentro(g->nome, cx, cy + 12, 14, WHITE);
        textoCentro(TextFormat("%.0f%%", g->bat), cx, cy + 28, 18, corBat(g->bat));

        if (i == origem) {
            DrawCircleLines(cx, cy, 52 + (int)(2 * sinf(tempoTotal * 6)), YELLOW);
            textoCentro("ORIGEM", cx, cy - 66, 14, YELLOW);
        }
        if (i == destino && destino != origem) {
            DrawCircleLines(cx, cy, 52, SKYBLUE);
            textoCentro("DESTINO", cx, cy - 66, 14, SKYBLUE);
        }
        if (g->inBlack)
            textoCentro(TextFormat("BLACKOUT %.1fs", TEMPO_ALERTA - g->tBlack), cx, cy - 48, 15, RED);
        if (g->inSobre)
            textoCentro(TextFormat("SOBRECARGA %.1fs", TEMPO_ALERTA - g->tSobre), cx, cy - 48, 15, ORANGE);
    }
}

static void listaAfetados(int id, char *fav, char *pre, size_t sz)
{
    fav[0] = 0; pre[0] = 0;
    for (int i = 0; i < N; i++) {
        if (!r[i].ativa) continue;
        float s = evDefs[id].prod[i] - evDefs[id].cons[i];
        char *alvo = (s > 0.03f) ? fav : ((s < -0.03f) ? pre : NULL);
        if (!alvo) continue;
        if (alvo[0]) strncat(alvo, ", ", sz - strlen(alvo) - 1);
        strncat(alvo, abrev[i], sz - strlen(alvo) - 1);
    }
}

static void desenharEventos(void)
{
    DrawRectangle(10, 100, 265, 430, Fade((Color){20, 30, 50, 255}, 0.92f));
    DrawRectangleLines(10, 100, 265, 430, (Color){70, 90, 120, 255});
    DrawText("EVENTOS ATIVOS", 22, 110, 17, YELLOW);
    if (nev == 0) DrawText("Nenhum no momento", 22, 142, 14, LIGHTGRAY);

    for (int e = 0; e < nev; e++) {
        int y = 138 + e * 78;
        char fav[100], pre[100];
        listaAfetados(evs[e].id, fav, pre, sizeof(fav));
        DrawText(evDefs[evs[e].id].nome, 22, y, 16, WHITE);
        DrawRectangle(22, y + 20, 235, 6, (Color){45, 55, 75, 255});
        DrawRectangle(22, y + 20, (int)(235 * evs[e].t / evs[e].dur), 6, SKYBLUE);
        DrawText(TextFormat("Favorece: %s", fav[0] ? fav : "-"), 22, y + 32, 12, (Color){110, 230, 150, 255});
        DrawText(TextFormat("Prejudica: %s", pre[0] ? pre : "-"), 22, y + 48, 12, (Color){255, 120, 110, 255});
    }

    DrawText("CONTROLES", 22, 548, 16, YELLOW);
    DrawText("Botao ESQ: escolher ORIGEM", 22, 572, 13, LIGHTGRAY);
    DrawText("Botao DIR: escolher DESTINO", 22, 590, 13, LIGHTGRAY);
    DrawText("E: enviar apoio  |  TAB: inverter", 22, 608, 13, LIGHTGRAY);
    DrawText("W/S ou Setas: producao da origem", 22, 626, 13, LIGHTGRAY);
    DrawText("R: producao normal  |  P: pausa", 22, 644, 13, LIGHTGRAY);
}

static void desenharPainel(void)
{
    DrawRectangle(930, 0, 350, H, (Color){22, 30, 48, 255});
    DrawText(TextFormat("FASE %d/%d   DIA %d/%d", fase, FASES, dia, DIAS), 950, 18, 22, YELLOW);
    DrawText(TextFormat("PONTOS: %d", pontos), 950, 50, 22, (Color){100, 240, 150, 255});
    DrawText(TextFormat("Vidas da rede: %d", vidas), 950, 82, 17, vidas > 1 ? WHITE : RED);
    DrawText(TextFormat("Sequencia: x%d", streak), 950, 106, 17, ORANGE);

    DrawText("REGIOES", 950, 140, 16, LIGHTGRAY);
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        int y = 164 + i * 78;
        Color nome = (i == origem) ? YELLOW : ((i == destino) ? SKYBLUE : WHITE);
        if (!g->ativa) nome = GRAY;
        DrawText(g->nome, 950, y, 16, nome);

        if (!g->ativa) { DrawText("bloqueada", 1180, y, 14, GRAY); continue; }

        DrawText(TextFormat("%.0f%%", g->bat), 1205, y, 16, corBat(g->bat));
        DrawRectangle(950, y + 22, 300, 14, (Color){40, 48, 68, 255});
        DrawRectangle(952, y + 24, (int)(296 * (g->bat > 100 ? 100 : g->bat) / 100.0f), 10, corBat(g->bat));
        DrawLine(950 + 150, y + 19, 950 + 150, y + 40, (Color){255, 120, 120, 255});
        DrawLine(950 + (int)(300 * LIM_ALTO / 100.0f), y + 19, 950 + (int)(300 * LIM_ALTO / 100.0f), y + 40,
                 (Color){255, 200, 90, 255});

        float prod = g->ajuste * g->multProd, cons = g->consBase * g->multCons;
        float saldo = (prod - cons) * 4.0f;
        DrawText(TextFormat("Prod %.0f%%", prod * 100), 950, y + 44, 13, SKYBLUE);
        DrawText(TextFormat("Cons %.0f%%", cons * 100), 1040, y + 44, 13, LIGHTGRAY);
        DrawText(TextFormat("%+.1f%%/s", saldo), 1140, y + 44, 14,
                 saldo > 0.15f ? GREEN : (saldo < -0.15f ? RED : WHITE));
    }

    DrawText(TextFormat("Producao da origem: %.0f%%", r[origem].ajuste * 100), 950, 560, 16, SKYBLUE);
    DrawText("Faixa segura: 50% a 85%", 950, 590, 15, (Color){110, 230, 150, 255});
    DrawText("Abaixo: blackout | Acima: sobrecarga", 950, 612, 14, LIGHTGRAY);
    DrawText(TextFormat("Apoio: %s", cooldown > 0 ? "carregando..." : "pronto (tecla E)"), 950, 640, 15,
             cooldown > 0 ? ORANGE : GREEN);

    /* Barra do dia e mensagem */
    DrawRectangle(290, 82, 600, 8, (Color){40, 48, 68, 255});
    DrawRectangle(290, 82, (int)(600 * tempoDia / DIA_SEG), 8, SKYBLUE);
    DrawText("VOL TERRA  |  Rede Nacional de Energia", 20, 18, 22, RAYWHITE);
    DrawText("Gerencie o Brasil sem apagao e sem sobrecarga", 20, 48, 14, LIGHTGRAY);

    if (msgT > 0) {
        DrawRectangle(290, 28, 600, 34, Fade(BLACK, 0.6f));
        textoCentro(msg, 590, 36, 17, WHITE);
    }
    if (pausa) {
        DrawRectangle(0, 0, W, H, Fade(BLACK, 0.6f));
        textoCentro("PAUSADO - P para continuar", W / 2, H / 2 - 12, 30, WHITE);
    }
}

static void desenharMinigame(void)
{
    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.80f));
    DrawRectangle(320, 150, 640, 420, (Color){25, 32, 53, 255});
    DrawRectangleLines(320, 150, 640, 420, YELLOW);

    if (mini.tipo == MINI_BLACKOUT) {
        textoCentro(TextFormat("BLACKOUT: %s", r[mini.reg].nome), 640, 168, 26, RED);
        textoCentro("Arraste cada fio ate a tomada da mesma cor", 640, 206, 17, WHITE);
        DrawRectangle(380, 238, 520, 8, (Color){40, 48, 68, 255});
        DrawRectangle(380, 238, (int)(520 * mini.tempo / 15.0f), 8, mini.tempo > 5 ? GREEN : RED);

        for (int i = 0; i < 4; i++)
            if (mini.ligados[i]) DrawLineEx(noEsq(i), noDir(mini.destino[i]), 8, fios[i]);
        if (mini.arrasta >= 0) DrawLineEx(noEsq(mini.arrasta), GetMousePosition(), 8, fios[mini.arrasta]);

        for (int i = 0; i < 4; i++) {
            DrawCircleV(noEsq(i), 17, fios[i]);
            DrawCircleV(noDir(i), 17, fios[mini.ordem[i]]);
        }
        textoCentro(TextFormat("Tempo: %.1f", mini.tempo), 640, 540, 20, WHITE);
    } else {
        textoCentro(TextFormat("SOBRECARGA: %s", r[mini.reg].nome), 640, 168, 26, ORANGE);
        textoCentro("Clique nas turbinas para reduzir a producao antes que superaquecam!", 640, 206, 16, WHITE);
        DrawRectangle(380, 238, 520, 8, (Color){40, 48, 68, 255});
        DrawRectangle(380, 238, (int)(520 * mini.tempo / 14.0f), 8, SKYBLUE);

        for (int i = 0; i < 4; i++) {
            int x = 380 + i * 140;
            float p = mini.calor[i] / 100.0f;
            if (p > 1) p = 1;
            DrawRectangle(x, 270, 100, 200, (Color){40, 48, 68, 255});
            DrawRectangle(x, 470 - (int)(200 * p), 100, (int)(200 * p),
                          p > 0.8f ? RED : (p > 0.55f ? ORANGE : GREEN));
            DrawRectangleLines(x, 270, 100, 200, WHITE);
            textoCentro(TextFormat("Turbina %d", i + 1), x + 50, 478, 15, WHITE);
        }
        textoCentro(TextFormat("Tempo restante: %.1f", mini.tempo), 640, 520, 20, LIGHTGRAY);
    }
}


/* ---------------- Programa principal ---------------- */

int main(void)
{
    InitWindow(W, H, "VolTerra - Gerenciamento de Energia");
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));
    carregarRanking();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.1f) dt = 0.1f;

        if (estado == MENU && IsKeyPressed(KEY_ENTER)) iniciarPartida();
        else if (estado == JOGO) atualizarJogo(dt);
        else if (estado == MINI) atualizarMinigame(dt);
        else if (estado == FIM && IsKeyPressed(KEY_ENTER)) estado = MENU;

        BeginDrawing();
        DrawRectangleGradientV(0, 0, W, H, (Color){8, 24, 46, 255}, (Color){10, 45, 78, 255});

        if (estado == MENU) {
            textoCentro("VOL TERRA", W / 2, 90, 56, SKYBLUE);
            textoCentro("Gerencie a energia do Brasil", W / 2, 160, 24, WHITE);
            textoCentro("Mantenha cada regiao entre 50% e 85% de bateria", W / 2, 215, 21, GREEN);
            textoCentro("Abaixo de 50%: blackout. Acima de 85%: sobrecarga.", W / 2, 248, 18, ORANGE);
            textoCentro("Botao esquerdo = origem | Botao direito = destino | E = enviar apoio", W / 2, 295, 18, LIGHTGRAY);
            textoCentro("W/S ajustam a producao da regiao de origem", W / 2, 322, 18, LIGHTGRAY);
            textoCentro("Partida de 10 minutos: 5 fases, eventos cada vez mais fortes", W / 2, 349, 18, LIGHTGRAY);
            textoCentro("ENTER para iniciar", W / 2, 410, 26, YELLOW);
            textoCentro("RANKING", W / 2, 470, 20, YELLOW);
            for (int i = 0; i < 5; i++)
                textoCentro(TextFormat("%d.  %d pts", i + 1, ranking[i]), W / 2, 500 + i * 24, 18, WHITE);
        } else if (estado == JOGO) {
            desenharMapa();
            desenharEventos();
            desenharPainel();
        } else if (estado == MINI) {
            desenharMapa();
            desenharPainel();
            desenharMinigame();
        } else {
            textoCentro(vitoria ? "REDE ESTABILIZADA - VOCE VENCEU!" : "FIM DE JOGO", W / 2, 100, 42,
                        vitoria ? GREEN : RED);
            textoCentro(TextFormat("Pontuacao: %d", pontos), W / 2, 165, 28, WHITE);
            if (posRanking > 0)
                textoCentro(TextFormat("Voce ficou em %do lugar no ranking!", posRanking), W / 2, 205, 20, GREEN);
            textoCentro("RANKING", W / 2, 250, 22, YELLOW);
            for (int i = 0; i < 5; i++)
                textoCentro(TextFormat("%d.  %d pts", i + 1, ranking[i]), W / 2, 290 + i * 30, 20,
                            (posRanking == i + 1) ? GREEN : WHITE);
            textoCentro("ENTER para voltar ao menu", W / 2, 500, 20, LIGHTGRAY);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}