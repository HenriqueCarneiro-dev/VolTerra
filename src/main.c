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
typedef enum { MENU, JOGO, MINI, FIM} Estado;
typedef enum { MINI_BLACKOUT, MINI_SOBRECARGA} TipoMini;

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
    float tempo;
    int ordem[4];
    bool ligados[4];
    int destino[4];
    int arrasta;
    float calor[4], taxa[4];
} Minigame;

/* ---------- Dados Mapa/Task---------- */

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
static Estado estado = MENU;
static Minigame mini;
static int fase, dia, origem, destino, vidas, streak, pontos;
static float tempoDia, tempoTotal, spawnT, cooldown, msgT, pontosAcum;
static char msg[200] = "";
static bool pausa = false, vitoria = false;
static int ranking[5] = {0, 0, 0, 0, 0};
static int posRanking = 0;

/* ---------------- Utilidades ---------------- */

static float projX(float lon) {return 290.0f + (lon + 74.0f) * 15.2f; }
static float projY(float lat) {return 95.0f + (5.5f - lat) * 15.2f; }

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

static bool dentro(Vector2 p, const Vector2 *v, int n)
{
    bool ok = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if(((v[i].y > p.y) != (v[j] > p.y)) && p.x < (v[j].x - v[i].x) * (p.y - v[i].y) / (v[j].y - v[i].y) + v[i].x) ok = !ok;
    }

    return ok;
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

static Color CorBateria(float b)
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
    for (int i = 0; i < N; i++)
        if (r[i].ativa) c++;

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

static bool eventoAfeta(int id){
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

static Vector2 noEsq(int i) { return (Vector2){435, 300 + i * 65}; }
static Vector2 noDir(int i) { return (Vector2){845, 300 + i * 65}; }

static void iniciarMinigame(int id, TipoMini tipo){
    mini.tipo = tipo;
    mini.reg = id;
    mini.arrasta = -1;
    mini.tempo = (tipo == MINI_BLACKOUT) ? 15.0f : 14.0f;

    for(int i=0; i<4; i++){
        mini.ordem[i] = i;
        mini.ligados[i] = false;
        mini.destino[i] = -1;
        //Divide por 10 para transformar o inteiro 0..300 em decimal 0.0..30.0.
        mini.calor[i] = 30.0f + GetRandomValue(0, 300) / 10.0f;
        mini.taxa[i] = 5.0f + GetRandomValue(0, 60) / 10.0f + false * 0.6f;
    }
    for (int i = 3; i > 0; i--) {
        int k = GetRandomValue(0, i);
        int t = mini.ordem[i];
        mini.ordem[i] = mini.ordem[k];
        mini.ordem[k] = t;
    }

    estado = MINI;
}

static void resolverMinigame(bool sucesso){
    Regiao *g = &r[mini.reg];
    g->inBlack = false; g->inSobre = false;
    g->tBlack = 0; g->tSobre = 0;

    if(sucesso){
        if(mini.tipo == MINI_BLACKOUT) g->bat = 62.0f;
        else {g->bat = 76.0f; g->ajuste = 0.85f;}
        pontos += 150;
        mensagem("Crise Resolvida! +150 pontos", 4.0f);
        estado = JOGO;
    } else {
        vidas--;
        g->bat = (mini.tipo == MINI_BLACKOUT) ? 58.0f : 76.0f;
        if(vidas <=0) { fimDePartida(false); return; }
        mensagem("Falhou ! Voce perdeu uma vida da rede.", 4.0f);
        estado = JOGO;
    }
}

static void atualizarMinigame(float dt)
{
    Vector2 mouse = GetMousePosition();
    mini.tempo -= dt;

    if (mini.tipo == MINI_BLACKOUT) {
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
    } else {
        bool falhou = false;
        for (int i = 0; i < 4; i++) {
            mini.calor[i] += mini.taxa[i] * dt;
            Rectangle rc = {380.0f + i * 140.0f, 270, 100, 200};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, rc)) {
                mini.calor[i] -= 24.0f;
                if (mini.calor[i] < 0) mini.calor[i] = 0;
            }
            if (mini.calor[i] >= 100.0f) falhou = true;
        }
        if (falhou) resolverMinigame(false);
        else if (mini.tempo <= 0) resolverMinigame(true);
    }
}

/*--------------- Logica do Jogo ---------------*/

//Roda quando o dia de 30 segundos termina. Ela avalia o dia, dá pontos e avança dia e fase
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
    //-------------------------------------------//

    //contagem regressiva para prox evento, quando zera cira um evento (max 5)
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

static void DesenharMundo(const Jogo *j, EstadoJogo estado, float tempo, Vector2 mouse)
{
    // Mar
    DrawRectangleGradientV(0, 0, LARGURA, 625, (Color){8, 22, 50, 255}, (Color){16, 58, 98, 255});

    for (int i = 0; i < 55; i++) {
        int x = (i * 197) % LARGURA;
        int y = (i * 91) % 600;
        float brilho = 0.15f + 0.35f * fabsf(sinf(tempo + i));
        DrawPixel(x, y, Fade(WHITE, brilho));
    }

    // Titulo
    DrawText("VOLTERRA", 28, 17, 38, (Color){255, 220, 80, 255});
    DrawText("A crise das cinco noites", 30, 57, 18, LIGHTGRAY);

    // Pontos (centro)
    char buf[128];
    snprintf(buf, sizeof(buf), "PONTOS  %d", (int)(j->pontosExibidos + 0.5f));
    DrawTextCentro(buf, 640, 10, 30, (Color){255, 220, 80, 255});

    int s = (j->sequencia > 6) ? 6 : j->sequencia;
    snprintf(buf, sizeof(buf), "Sequencia: %d dia(s)  |  multiplicador x%.1f",
             j->sequencia, (9 + s) / 10.0f);
    DrawTextCentro(buf, 640, 44, 15, LIGHTGRAY);

    // Fase, dia e tempo (direita)
    snprintf(buf, sizeof(buf), "FASE %d / %d", j->fase, TOTAL_FASES);
    DrawText(buf, 1010, 12, 26, YELLOW);

    float restante = TOTAL_FASES * DURACAO_FASE - ((j->fase - 1) * DURACAO_FASE + j->tempoFase);
    if (restante < 0.0f) restante = 0.0f;
    snprintf(buf, sizeof(buf), "Dia %d/%d   |   restam %d:%02d",
             j->dia, DIAS_POR_FASE, (int)restante / 60, (int)restante % 60);
    DrawText(buf, 1010, 44, 16, LIGHTGRAY);

    float pd = j->tempoDia / DURACAO_DIA;
    if (pd > 1.0f) pd = 1.0f;
    DrawRectangle(1010, 68, 230, 8, (Color){40, 45, 70, 255});
    DrawRectangle(1010, 68, (int)(230 * pd), 8, YELLOW);

    // Regioes (a selecionada por ultimo, para o contorno ficar por cima)
    for (int passo = 0; passo < 2; passo++) {
        for (int i = 0; i < NUM_REGIOES; i++) {
            if ((i == j->selecionada) != (passo == 1)) continue;

            const Regiao *r = &j->regioes[i];
            Color cor = r->cor;

            if (!r->ativa) cor = (Color){62, 68, 84, 255};
            else if (estado == ESTADO_JOGO && PontoNoPoligono(mouse, r->pontos, r->numPontos)) {
                cor = ColorBrightness(cor, 0.15f);
            }

            PreencherPoligono(r->pontos, r->numPontos, Fade(BLACK, 0.45f), 7.0f, 9.0f);
            PreencherPoligono(r->pontos, r->numPontos, cor, 0.0f, 0.0f);

            if (r->emBlackout) {
                float pulso = 0.25f + 0.2f * sinf(tempo * 8.0f);
                PreencherPoligono(r->pontos, r->numPontos, Fade(RED, pulso), 0.0f, 0.0f);
            }

            DesenharContorno(r->pontos, r->numPontos,
                             i == j->selecionada ? 5.0f : 2.0f,
                             i == j->selecionada ? WHITE : (Color){15, 18, 30, 255});
        }
    }

    // Icones e textos das regioes
    for (int i = 0; i < NUM_REGIOES; i++) {
        const Regiao *r = &j->regioes[i];

        if (r->ativa) {
            DesenharIconeFonte(r->fonte, r->centro, tempo);
            DrawTextCentro(r->nome, (int)r->centro.x, (int)r->centro.y + 30, 17, WHITE);

            snprintf(buf, sizeof(buf), "%.0f%%", r->bateria);
            DrawTextCentro(buf, (int)r->centro.x, (int)r->centro.y + 49, 17, CorBateria(r->bateria));

            if (r->emBlackout) {
                snprintf(buf, sizeof(buf), "BLACKOUT %.0f", r->blackoutTempo < 0 ? 0 : r->blackoutTempo + 0.99f);
                DrawTextCentro(buf, (int)r->centro.x, (int)r->centro.y - 52, 22, WHITE);
            }
        } else {
            DrawTextCentro(r->nome, (int)r->centro.x, (int)r->centro.y - 8, 17, LIGHTGRAY);
            snprintf(buf, sizeof(buf), "libera na fase %d", FaseLiberacao(i));
            DrawTextCentro(buf, (int)r->centro.x, (int)r->centro.y + 14, 14, GRAY);
        }
    }

    // Linha de energia
    if (j->linhaTempo > 0.0f && j->origem >= 0 && j->destino >= 0) {
        Vector2 a = j->regioes[j->origem].centro;
        Vector2 b = j->regioes[j->destino].centro;

        DrawLineEx(a, b, 4.0f, Fade(YELLOW, j->linhaTempo));
        for (int i = 0; i < 4; i++) {
            float p = fmodf(tempo * 1.5f + i * 0.25f, 1.0f);
            Vector2 pos = {a.x + (b.x - a.x) * p, a.y + (b.y - a.y) * p};
            DrawCircleV(pos, 6.0f, Fade(WHITE, j->linhaTempo));
        }
    }

    // Painel inferior
    DrawRectangle(0, 625, LARGURA, 95, (Color){20, 26, 50, 255});
    DrawLine(0, 625, LARGURA, 625, (Color){255, 220, 80, 255});
    DrawText("ENERGIA DAS REGIOES", 28, 637, 17, YELLOW);

    for (int i = 0; i < NUM_REGIOES; i++) {
        const Regiao *r = &j->regioes[i];
        int x = 28 + i * 245;

        DrawText(r->nome, x, 660, 15, r->ativa ? WHITE : GRAY);
        DesenharBarraBateria(x, 682, 190, 16, r->bateria, r->limiteBlackout, r->ativa);

        if (r->ativa) {
            snprintf(buf, sizeof(buf), "limite %.0f%%", r->limiteBlackout);
            DrawText(buf, x + 194, 681, 13, LIGHTGRAY);
        } else {
            DrawText("bloqueada", x + 194, 681, 13, GRAY);
        }
    }

    // Controles
    DrawRectangle(20, 94, 265, 112, Fade((Color){15, 20, 38, 255}, 0.92f));
    DrawRectangleLines(20, 94, 265, 112, (Color){100, 120, 160, 255});
    DrawText("CONTROLES", 34, 103, 17, YELLOW);
    DrawText("WASD / clique: selecionar", 34, 127, 15, WHITE);
    DrawText("E: transferir energia", 34, 147, 15, WHITE);
    DrawText("ESPACO: economizar energia", 34, 167, 15, WHITE);
    DrawText("Blackout: 10 s ou minigame!", 34, 187, 14, ORANGE);

    // Evento
    DrawRectangle(965, 95, 285, 70, Fade((Color){15, 20, 38, 255}, 0.92f));
    DrawRectangleLines(965, 95, 285, 70, (Color){100, 120, 160, 255});
    DrawText("EVENTO DO DIA", 980, 105, 16, YELLOW);
    DrawText(NomeEvento(j->evento), 980, 130, 20, WHITE);

    // Regiao selecionada
    const Regiao *sel = &j->regioes[j->selecionada];
    DrawRectangle(965, 180, 285, 125, Fade((Color){15, 20, 38, 255}, 0.92f));
    DrawRectangleLines(965, 180, 285, 125, (Color){100, 120, 160, 255});

    DrawText(sel->nome, 980, 189, 21, WHITE);
    DrawText(sel->fonteNome, 980, 215, 16, LIGHTGRAY);

    snprintf(buf, sizeof(buf), "%s | blackout < %.0f%%",
             sel->cidadeGrande ? "Cidade grande" : "Cidade pequena", sel->limiteBlackout);
    DrawText(buf, 980, 238, 14, YELLOW);

    float saldo = (sel->producao - sel->consumo * (sel->economiaTempo > 0.0f ? FATOR_ECONOMIA : 1.0f))
                  * FATOR_SIMULACAO;
    snprintf(buf, sizeof(buf), "Saldo: %+.1f%% por segundo", saldo);
    DrawText(buf, 980, 258, 15, saldo >= 0.0f ? (Color){80, 220, 150, 255} : RED);

    if (sel->economiaTempo > 0.0f) snprintf(buf, sizeof(buf), "Economia ativa: %.0f s", sel->economiaTempo);
    else if (sel->recarga > 0.0f)  snprintf(buf, sizeof(buf), "Economia em recarga: %.0f s", sel->recarga);
    else                           snprintf(buf, sizeof(buf), "Economia pronta (ESPACO)");
    DrawText(buf, 980, 280, 14, LIGHTGRAY);

    // Mensagem
    if (j->mensagemTempo > 0.0f && estado != ESTADO_MENU && estado != ESTADO_FIM) {
        int w = MeasureText(j->mensagem, 18);
        DrawRectangle(640 - w / 2 - 16, 68, w + 32, 28, Fade((Color){8, 12, 24, 255}, 0.9f));
        DrawText(j->mensagem, 640 - w / 2, 73, 18, WHITE);
    }
}

/* ---------------- Programa principal ---------------- */

int main(void)
{
    InitWindow(LARGURA, ALTURA, "VolTerra - A crise das cinco noites");
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    Jogo jogo;
    Minigame mini = {0};
    int ranking[TAM_RANKING];
    EstadoJogo estado = ESTADO_MENU;

    CarregarRanking(ranking);
    IniciarPartida(&jogo);

    while (!WindowShouldClose()) {
        Jogo *j = &jogo;
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;
        float tempo = (float)GetTime();
        Vector2 mouse = GetMousePosition();

        /* ---------- Atualizacao ---------- */

        if (estado == ESTADO_MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                IniciarPartida(j);
                estado = ESTADO_JOGO;
            }
        }
        else if (estado == ESTADO_FASE) {
            if (IsKeyPressed(KEY_ENTER)) {
                ConfigurarFase(j);
                estado = ESTADO_JOGO;
            }
        }
        else if (estado == ESTADO_FIM) {
            if (IsKeyPressed(KEY_R)) {
                IniciarPartida(j);
                estado = ESTADO_JOGO;
            }
        }
        else if (estado == ESTADO_MINIGAME) {
            int res = MinigameAtualizar(&mini, dt);

            if (res == 1) {
                Regiao *r = &j->regioes[mini.regiao];
                r->bateria = r->limiteBlackout + 25.0f;
                if (r->bateria > 100.0f) r->bateria = 100.0f;
                r->emBlackout = false;
                r->blackoutTempo = 0.0f;
                j->pontos += 50;

                snprintf(j->mensagem, sizeof(j->mensagem),
                         "Rede restaurada em %s! +50 pontos", r->nome);
                j->mensagemTempo = 4.0f;
                estado = ESTADO_JOGO;
            }
            else if (res == -1) {
                snprintf(j->mensagem, sizeof(j->mensagem),
                         "BLACKOUT! %s ficou no escuro e o Brasil apagou.",
                         j->regioes[mini.regiao].nome);
                j->vitoria = false;
                j->posicaoRanking = InserirRanking(ranking, j->pontos);
                estado = ESTADO_FIM;
            }
        }
        else if (estado == ESTADO_JOGO) {
            j->tempoDia += dt;
            j->tempoFase += dt;

            if (j->linhaTempo > 0.0f) j->linhaTempo -= dt;
            if (j->mensagemTempo > 0.0f) j->mensagemTempo -= dt;

            // Selecao por teclado: vai para a regiao ativa mais proxima na direcao.
            Vector2 dir = {0.0f, 0.0f};
            if (IsKeyPressed(KEY_W)) dir.y = -1.0f;
            else if (IsKeyPressed(KEY_S)) dir.y = 1.0f;
            else if (IsKeyPressed(KEY_A)) dir.x = -1.0f;
            else if (IsKeyPressed(KEY_D)) dir.x = 1.0f;

            if (dir.x != 0.0f || dir.y != 0.0f) {
                int melhor = -1;
                float melhorScore = 1e9f;
                Vector2 o = j->regioes[j->selecionada].centro;

                for (int i = 0; i < NUM_REGIOES; i++) {
                    if (i == j->selecionada || !j->regioes[i].ativa) continue;

                    float dx = j->regioes[i].centro.x - o.x;
                    float dy = j->regioes[i].centro.y - o.y;
                    float proj = dx * dir.x + dy * dir.y;
                    if (proj <= 0.0f) continue;

                    float dist2 = dx * dx + dy * dy;
                    float score = dist2 / proj;

                    if (score < melhorScore) {
                        melhorScore = score;
                        melhor = i;
                    }
                }

                if (melhor >= 0) j->selecionada = melhor;
            }

            // Selecao por mouse
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (int i = 0; i < NUM_REGIOES; i++) {
                    if (j->regioes[i].ativa &&
                        PontoNoPoligono(mouse, j->regioes[i].pontos, j->regioes[i].numPontos)) {
                        j->selecionada = i;
                    }
                }
            }

            // E: transferencia (12% saem, 9% chegam)
            if (IsKeyPressed(KEY_E)) {
                int origem = -1;
                float maior = -1.0f;

                for (int i = 0; i < NUM_REGIOES; i++) {
                    const Regiao *o = &j->regioes[i];
                    if (i == j->selecionada || !o->ativa) continue;

                    if (o->bateria - 12.0f >= o->limiteBlackout + 5.0f && o->bateria > maior) {
                        maior = o->bateria;
                        origem = i;
                    }
                }

                if (origem >= 0) {
                    Regiao *dest = &j->regioes[j->selecionada];

                    j->regioes[origem].bateria -= 12.0f;
                    dest->bateria += 9.0f;
                    if (dest->bateria > 100.0f) dest->bateria = 100.0f;

                    j->origem = origem;
                    j->destino = j->selecionada;
                    j->linhaTempo = 1.0f;

                    snprintf(j->mensagem, sizeof(j->mensagem),
                             "%s enviou 12%% para %s (9%% chegaram).",
                             j->regioes[origem].nome, dest->nome);
                } else {
                    snprintf(j->mensagem, sizeof(j->mensagem),
                             "Nenhuma regiao pode doar sem arriscar o proprio blackout.");
                }
                j->mensagemTempo = 3.0f;
            }

            // Espaco: economia emergencial
            if (IsKeyPressed(KEY_SPACE)) {
                Regiao *s2 = &j->regioes[j->selecionada];

                if (s2->economiaTempo > 0.0f) {
                    snprintf(j->mensagem, sizeof(j->mensagem), "A economia ja esta ativa.");
                } else if (s2->recarga > 0.0f) {
                    snprintf(j->mensagem, sizeof(j->mensagem),
                             "Economia em recarga: %.0f s.", s2->recarga);
                } else {
                    s2->economiaTempo = 8.0f;
                    s2->recarga = 20.0f;
                    snprintf(j->mensagem, sizeof(j->mensagem),
                             "Economia ativada em %s por 8 segundos!", s2->nome);
                }
                j->mensagemTempo = 3.0f;
            }

            // Baterias e contagem de blackout
            int regiaoCaiu = -1;

            for (int i = 0; i < NUM_REGIOES; i++) {
                Regiao *r = &j->regioes[i];
                if (!r->ativa) continue;

                if (r->economiaTempo > 0.0f) r->economiaTempo -= dt;
                if (r->recarga > 0.0f) r->recarga -= dt;

                float consumoAtual = r->consumo;
                if (r->economiaTempo > 0.0f) consumoAtual *= FATOR_ECONOMIA;

                r->bateria += (r->producao - consumoAtual) * dt * FATOR_SIMULACAO;
                if (r->bateria > 100.0f) r->bateria = 100.0f;
                if (r->bateria < 0.0f) r->bateria = 0.0f;

                if (r->bateria < r->limiteBlackout) {
                    if (!r->emBlackout) {
                        r->emBlackout = true;
                        r->blackoutTempo = TEMPO_BLACKOUT;
                        snprintf(j->mensagem, sizeof(j->mensagem),
                                 "ALERTA: %s em blackout! Voce tem 10 segundos.", r->nome);
                        j->mensagemTempo = 4.0f;
                    } else {
                        r->blackoutTempo -= dt;
                        if (r->blackoutTempo <= 0.0f && regiaoCaiu < 0) regiaoCaiu = i;
                    }
                } else {
                    r->emBlackout = false;
                }
            }

            if (regiaoCaiu >= 0) {
                MinigameIniciar(&mini, regiaoCaiu);
                estado = ESTADO_MINIGAME;
            }
            else if (j->tempoDia >= DURACAO_DIA) {
                EncerrarDia(j);
                j->tempoDia = 0.0f;
                j->dia++;

                if (j->dia > DIAS_POR_FASE) {
                    if (j->fase >= TOTAL_FASES) {
                        j->pontos += 1000;
                        j->vitoria = true;
                        snprintf(j->mensagem, sizeof(j->mensagem),
                                 "Voce manteve o Brasil aceso por 10 minutos!");
                        j->posicaoRanking = InserirRanking(ranking, j->pontos);
                        estado = ESTADO_FIM;
                    } else {
                        j->pontos += 300 * j->fase;
                        j->fase++;
                        estado = ESTADO_FASE;
                    }
                } else {
                    SortearDia(j);
                }
            }
        }

        // Contador de pontos subindo
        if (j->pontosExibidos < (float)j->pontos) {
            float diff = (float)j->pontos - j->pontosExibidos;
            float passo = diff * dt * 4.0f;
            if (passo < 1.0f) passo = 1.0f;
            if (passo > diff) passo = diff;
            j->pontosExibidos += passo;
        }

        /* ---------- Desenho ---------- */

        BeginDrawing();
        ClearBackground(BLACK);

        DesenharMundo(j, estado, tempo, mouse);

        char buf[160];

        if (estado == ESTADO_MINIGAME) {
            MinigameDesenhar(&mini, j->regioes[mini.regiao].nome);
        }
        else if (estado == ESTADO_MENU) {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.80f));
            DrawTextCentro("VOLTERRA", 640, 90, 80, (Color){255, 220, 80, 255});
            DrawTextCentro("A crise das cinco noites", 640, 175, 24, LIGHTGRAY);

            DrawTextCentro("5 fases de 2 minutos = 10 minutos de partida", 640, 235, 20, WHITE);
            DrawTextCentro("Fase 1: 3 regioes   |   Fase 2: 4 regioes   |   Fases 3 a 5: 5 regioes", 640, 262, 18, LIGHTGRAY);
            DrawTextCentro("Regiao abaixo do limite = 10 s para salvar, ou cai o minigame dos fios", 640, 288, 18, LIGHTGRAY);

            DesenharRanking(ranking, 640, 340, 0);

            if (((int)(tempo * 2.0f)) % 2 == 0) {
                DrawTextCentro("Pressione ENTER para comecar", 640, 530, 26, (Color){100, 240, 150, 255});
            }
        }
        else if (estado == ESTADO_FASE) {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.78f));

            snprintf(buf, sizeof(buf), "FASE %d CONCLUIDA!", j->fase - 1);
            DrawTextCentro(buf, 640, 190, 54, (Color){100, 240, 150, 255});

            snprintf(buf, sizeof(buf), "Pontos: %d  (bonus de fase: +%d)", j->pontos, 300 * (j->fase - 1));
            DrawTextCentro(buf, 640, 270, 24, WHITE);

            snprintf(buf, sizeof(buf), "Proxima: Fase %d  |  %d regioes ativas  |  eventos mais fortes",
                     j->fase, RegioesDaFase(j->fase));
            DrawTextCentro(buf, 640, 315, 20, LIGHTGRAY);

            DrawTextCentro("As baterias recuperam 35% antes da proxima fase.", 640, 345, 18, LIGHTGRAY);
            DrawTextCentro("Pressione ENTER para continuar", 640, 420, 26, YELLOW);
        }
        else if (estado == ESTADO_FIM) {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.80f));

            DrawTextCentro(j->vitoria ? "BRASIL SALVO!" : "APAGAO NACIONAL", 640, 90, 56,
                           j->vitoria ? (Color){100, 240, 150, 255} : RED);
            DrawTextCentro(j->mensagem, 640, 160, 20, WHITE);

            snprintf(buf, sizeof(buf), "Pontuacao final: %d", j->pontos);
            DrawTextCentro(buf, 640, 200, 30, (Color){255, 220, 80, 255});

            if (j->posicaoRanking > 0) {
                snprintf(buf, sizeof(buf), "Voce entrou no ranking: posicao #%d!", j->posicaoRanking);
                DrawTextCentro(buf, 640, 245, 22, (Color){100, 240, 150, 255});
            }

            DesenharRanking(ranking, 640, 300, j->posicaoRanking);
            DrawTextCentro("R: jogar de novo   |   ESC: sair", 640, 500, 22, LIGHTGRAY);
        }

        DrawFPS(LARGURA - 75, ALTURA - 25);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}