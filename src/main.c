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
typedef enum { MENU, JOGO, MINI, FIM} estado;
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

/*=============== Continuar aqui Estado global e renovacao as Utilizades ===============*/
//
//

/* ---------------- Utilidades ---------------- */
static const char *NomeEvento(TipoEvento evento)
{
    switch (evento) {
        case EVENTO_CHUVA:    return "Chuvas fortes";
        case EVENTO_ENCHENTE: return "Enchente";
        case EVENTO_VENTOS:   return "Ventos fortes";
        case EVENTO_POLUICAO: return "Poluicao";
        default:              return "Tempo estavel";
    }
}

static void DrawTextCentro(const char *texto, int cx, int y, int tamanho, Color cor)
{
    DrawText(texto, cx - MeasureText(texto, tamanho) / 2, y, tamanho, cor);
}

static bool PontoNoPoligono(Vector2 p, const Vector2 *v, int n)
{
    bool dentro = false;

    for (int i = 0, j = n - 1; i < n; j = i++) {
        bool cruza = ((v[i].y > p.y) != (v[j].y > p.y));

        if (cruza) {
            float x = (v[j].x - v[i].x) * (p.y - v[i].y) / (v[j].y - v[i].y) + v[i].x;
            if (p.x < x) dentro = !dentro;
        }
    }

    return dentro;
}

// Desenha cada triangulo nos dois sentidos: a raylib descarta o sentido errado.
static void PreencherPoligono(const Vector2 *p, int n, Color cor, float dx, float dy)
{
    Vector2 c = {0.0f, 0.0f};

    for (int i = 0; i < n; i++) {
        c.x += p[i].x;
        c.y += p[i].y;
    }

    c.x = c.x / n + dx;
    c.y = c.y / n + dy;

    for (int i = 0; i < n; i++) {
        Vector2 a = {p[i].x + dx, p[i].y + dy};
        Vector2 b = {p[(i + 1) % n].x + dx, p[(i + 1) % n].y + dy};
        DrawTriangle(c, a, b, cor);
        DrawTriangle(c, b, a, cor);
    }
}

static void DesenharContorno(const Vector2 *p, int n, float espessura, Color cor)
{
    for (int i = 0; i < n; i++) {
        DrawLineEx(p[i], p[(i + 1) % n], espessura, cor);
    }
}

static void DesenharIconeFonte(TipoFonte fonte, Vector2 c, float tempo)
{
    switch (fonte) {
        case FONTE_HIDRO:
            DrawRectangle(c.x - 18, c.y - 10, 36, 22, (Color){65, 75, 95, 255});
            DrawRectangle(c.x - 23, c.y + 12, 46, 5, (Color){50, 140, 240, 255});
            for (int i = 0; i < 3; i++) {
                float d = fmodf(tempo * 24.0f + i * 9.0f, 18.0f);
                DrawCircle(c.x - 10 + i * 10, c.y - 4 + d, 2.5f, SKYBLUE);
            }
            break;

        case FONTE_EOLICA: {
            float angulo = tempo * 120.0f;
            DrawLineEx((Vector2){c.x, c.y - 5}, (Vector2){c.x, c.y + 23}, 4, RAYWHITE);
            for (int i = 0; i < 3; i++) {
                float a = (angulo + i * 120.0f) * DEG2RAD;
                Vector2 ponta = {c.x + cosf(a) * 23.0f, c.y - 5 + sinf(a) * 23.0f};
                DrawLineEx((Vector2){c.x, c.y - 5}, ponta, 4, RAYWHITE);
            }
            DrawCircle(c.x, c.y - 5, 5, YELLOW);
            break;
        }

        case FONTE_SOLAR:
            DrawCircle(c.x + 15, c.y - 14, 8, YELLOW);
            DrawRectangle(c.x - 24, c.y - 2, 38, 23, (Color){20, 55, 125, 255});
            DrawLine(c.x - 24, c.y + 9, c.x + 14, c.y + 9, SKYBLUE);
            DrawLine(c.x - 5, c.y - 2, c.x - 5, c.y + 21, SKYBLUE);
            DrawLine(c.x + 7, c.y - 2, c.x + 7, c.y + 21, SKYBLUE);
            break;

        case FONTE_NUCLEAR:
            DrawTriangle((Vector2){c.x - 19, c.y + 19}, (Vector2){c.x + 19, c.y + 19},
                         (Vector2){c.x - 10, c.y - 13}, LIGHTGRAY);
            DrawTriangle((Vector2){c.x + 10, c.y - 13}, (Vector2){c.x - 10, c.y - 13},
                         (Vector2){c.x + 19, c.y + 19}, LIGHTGRAY);
            DrawCircle(c.x + 21, c.y - 17, 6, Fade(WHITE, 0.75f));
            DrawCircle(c.x + 29, c.y - 22, 5, Fade(WHITE, 0.5f));
            break;

        case FONTE_BIOMASSA:
            DrawEllipse(c.x, c.y, 12, 19, GREEN);
            DrawLineEx((Vector2){c.x, c.y + 19}, (Vector2){c.x, c.y - 16}, 2, DARKGREEN);
            DrawTriangle((Vector2){c.x - 6, c.y + 25}, (Vector2){c.x + 6, c.y + 25},
                         (Vector2){c.x, c.y + 12 + sinf(tempo * 5.0f) * 2.0f}, ORANGE);
            break;
    }
}

static Color CorBateria(float bateria)
{
    if (bateria < 30.0f) return RED;
    if (bateria < 55.0f) return ORANGE;
    return (Color){80, 220, 150, 255};
}

static void DesenharBarraBateria(float x, float y, float largura, float altura,
                                 float bateria, float limite, bool ativa)
{
    DrawRectangle(x, y, largura, altura, (Color){25, 30, 45, 255});

    if (ativa) {
        float p = bateria / 100.0f;
        if (p < 0.0f) p = 0.0f;
        if (p > 1.0f) p = 1.0f;
        DrawRectangle(x + 2, y + 2, (largura - 4) * p, altura - 4, CorBateria(bateria));

        float lx = x + largura * (limite / 100.0f);
        DrawLineEx((Vector2){lx, y - 3}, (Vector2){lx, y + altura + 3}, 2, WHITE);
    }

    DrawRectangleLines(x, y, largura, altura, ativa ? LIGHTGRAY : DARKGRAY);
}

/* ---------------- Regioes e eventos ---------------- */

static void CriarRegioes(Regiao *saida)
{
    Regiao modelo[NUM_REGIOES] = {
        {   // NORTE
            "Norte", "Hidreletrica", FONTE_HIDRO, {46, 139, 87, 255},
            { {290,250}, {310,170}, {440,110}, {560,115}, {650,95}, {720,130},
              {755,145}, {745,230}, {720,300}, {580,320}, {440,310}, {310,295} },
            12, {520, 205}, 100.0f, 0.90f, 0.95f, 20.0f, false
        },
        {   // NORDESTE
            "Nordeste", "Eolica", FONTE_EOLICA, {218, 165, 32, 255},
            { {755,145}, {790,160}, {860,165}, {910,200}, {905,250}, {880,300},
              {860,350}, {840,400}, {780,385}, {735,340}, {720,300}, {745,230} },
            12, {830, 260}, 100.0f, 0.95f, 1.00f, 50.0f, true
        },
        {   // CENTRO-OESTE
            "Centro-Oeste", "Solar", FONTE_SOLAR, {210, 105, 30, 255},
            { {310,295}, {440,310}, {580,320}, {720,300}, {735,340}, {780,385},
              {710,415}, {655,450}, {650,505}, {610,540}, {590,490}, {540,450},
              {470,420}, {400,380}, {330,340} },
            15, {540, 385}, 100.0f, 0.88f, 0.93f, 20.0f, false
        },
        {   // SUDESTE
            "Sudeste", "Nuclear", FONTE_NUCLEAR, {160, 82, 160, 255},
            { {840,400}, {810,450}, {770,490}, {730,530}, {700,500}, {650,505},
              {655,450}, {710,415}, {780,385} },
            9, {745, 450}, 100.0f, 0.92f, 1.05f, 50.0f, true
        },
        {   // SUL
            "Sul", "Hidreletrica", FONTE_HIDRO, {65, 105, 225, 255},
            { {730,530}, {690,575}, {640,590}, {610,540}, {650,505}, {700,500} },
            6, {668, 538}, 100.0f, 0.88f, 0.93f, 20.0f, false
        }
    };

    for (int i = 0; i < NUM_REGIOES; i++) {
        saida[i] = modelo[i];
    }
}

static int RegioesDaFase(int fase)
{
    if (fase <= 1) return 3;
    if (fase == 2) return 4;
    return 5;
}

static int FaseLiberacao(int id)
{
    for (int k = 0; k < NUM_REGIOES; k++) {
        if (ordemAtivacao[k] == id) return (k < 3) ? 1 : k - 1;
    }
    return 1;
}

static float IntensidadeFase(int fase)
{
    return 1.0f + 0.2f * (fase - 1);
}

// Efeitos negativos de producao crescem com a intensidade.
static float ProdEv(float f, float k)
{
    if (f >= 1.0f) return f;
    float r = 1.0f - (1.0f - f) * k;
    return (r < 0.30f) ? 0.30f : r;
}

// Efeitos negativos de consumo crescem com a intensidade.
static float ConsEv(float f, float k)
{
    if (f <= 1.0f) return f;
    return 1.0f + (f - 1.0f) * k;
}

static void AplicarEvento(Regiao *r, TipoEvento evento, float k)
{
    static const float prodBase[NUM_REGIOES] = {0.90f, 0.95f, 0.88f, 0.92f, 0.88f};
    static const float consBase[NUM_REGIOES] = {0.95f, 1.00f, 0.93f, 1.05f, 0.93f};

    for (int i = 0; i < NUM_REGIOES; i++) {
        r[i].producao = prodBase[i];
        r[i].consumo = consBase[i];
    }

    switch (evento) {
        case EVENTO_CHUVA:
            r[NORTE].producao    *= ProdEv(1.25f, k);
            r[SUL].producao      *= ProdEv(1.20f, k);
            r[SUDESTE].producao  *= ProdEv(1.15f, k);
            r[CENTRO_OESTE].producao *= ProdEv(0.90f, k);
            break;

        case EVENTO_ENCHENTE:
            r[NORTE].producao    *= ProdEv(0.65f, k);
            r[NORDESTE].producao *= ProdEv(0.80f, k);
            r[SUL].producao      *= ProdEv(0.85f, k);
            r[SUDESTE].producao  *= ProdEv(0.90f, k);
            break;

        case EVENTO_VENTOS:
            r[NORDESTE].producao *= ProdEv(1.35f, k);
            r[CENTRO_OESTE].consumo *= ConsEv(1.15f, k);
            r[SUL].consumo       *= ConsEv(1.10f, k);
            break;

        case EVENTO_POLUICAO:
            r[CENTRO_OESTE].producao *= ProdEv(0.70f, k);
            r[SUDESTE].consumo   *= ConsEv(1.35f, k);
            r[NORDESTE].consumo  *= ConsEv(1.15f, k);
            break;

        default:
            break;
    }
}

/* ---------------- Ranking ---------------- */

static void CarregarRanking(int ranking[])
{
    for (int i = 0; i < TAM_RANKING; i++) ranking[i] = 0;

    FILE *f = fopen("ranking.txt", "r");
    if (!f) return;

    for (int i = 0; i < TAM_RANKING; i++) {
        if (fscanf(f, "%d", &ranking[i]) != 1) break;
    }

    fclose(f);
}

static int InserirRanking(int ranking[], int pontos)
{
    int pos = -1;

    if (pontos <= 0) return 0;

    for (int i = 0; i < TAM_RANKING; i++) {
        if (pontos > ranking[i]) {
            pos = i;
            break;
        }
    }

    if (pos < 0) return 0;

    for (int i = TAM_RANKING - 1; i > pos; i--) {
        ranking[i] = ranking[i - 1];
    }
    ranking[pos] = pontos;

    FILE *f = fopen("ranking.txt", "w");
    if (f) {
        for (int i = 0; i < TAM_RANKING; i++) fprintf(f, "%d\n", ranking[i]);
        fclose(f);
    }

    return pos + 1;
}

static void DesenharRanking(const int ranking[], int cx, int y, int destaque)
{
    DrawTextCentro("RANKING", cx, y, 22, YELLOW);

    for (int i = 0; i < TAM_RANKING; i++) {
        char linha[64];

        if (ranking[i] > 0) snprintf(linha, sizeof(linha), "%d.  %d pts", i + 1, ranking[i]);
        else                snprintf(linha, sizeof(linha), "%d.  ---", i + 1);

        DrawTextCentro(linha, cx, y + 30 + i * 24, 20,
                       (destaque == i + 1) ? (Color){100, 240, 150, 255} : WHITE);
    }
}

/* ---------------- Logica da partida ---------------- */

static void SortearDia(Jogo *j)
{
    j->evento = (TipoEvento)GetRandomValue(EVENTO_CHUVA, EVENTO_POLUICAO);
    AplicarEvento(j->regioes, j->evento, IntensidadeFase(j->fase));

    if (j->ganhoDia > 0) {
        snprintf(j->mensagem, sizeof(j->mensagem),
                 "Fase %d, dia %d: %s! (dia anterior: +%d pts)",
                 j->fase, j->dia, NomeEvento(j->evento), j->ganhoDia);
    } else {
        snprintf(j->mensagem, sizeof(j->mensagem),
                 "Fase %d, dia %d: %s!", j->fase, j->dia, NomeEvento(j->evento));
    }
    j->mensagemTempo = 5.0f;
}

static void ConfigurarFase(Jogo *j)
{
    int qtd = RegioesDaFase(j->fase);

    for (int i = 0; i < NUM_REGIOES; i++) j->regioes[i].ativa = false;
    for (int k = 0; k < qtd; k++) j->regioes[ordemAtivacao[k]].ativa = true;

    for (int i = 0; i < NUM_REGIOES; i++) {
        Regiao *r = &j->regioes[i];
        r->economiaTempo = 0.0f;
        r->recarga = 0.0f;
        r->emBlackout = false;
        r->blackoutTempo = 0.0f;

        if (r->ativa) {
            r->bateria += 35.0f;
            if (r->bateria > 100.0f) r->bateria = 100.0f;
        }
    }

    if (!j->regioes[j->selecionada].ativa) j->selecionada = ordemAtivacao[0];

    j->dia = 1;
    j->tempoDia = 0.0f;
    j->tempoFase = 0.0f;
    j->ganhoDia = 0;
    j->linhaTempo = 0.0f;

    SortearDia(j);
}

static void IniciarPartida(Jogo *j)
{
    CriarRegioes(j->regioes);

    j->fase = 1;
    j->selecionada = SUDESTE;
    j->pontos = 0;
    j->pontosExibidos = 0.0f;
    j->sequencia = 0;
    j->ganhoDia = 0;
    j->origem = -1;
    j->destino = -1;
    j->vitoria = false;
    j->posicaoRanking = 0;

    ConfigurarFase(j);
}

static void EncerrarDia(Jogo *j)
{
    float soma = 0.0f;
    int n = 0;
    bool alerta = false;

    for (int i = 0; i < NUM_REGIOES; i++) {
        const Regiao *r = &j->regioes[i];
        if (!r->ativa) continue;
        soma += r->bateria;
        n++;
        if (r->emBlackout) alerta = true;
    }

    float media = (n > 0) ? soma / n : 0.0f;

    if (alerta) {
        j->sequencia = 0;
        j->ganhoDia = 0;
    } else {
        j->sequencia++;
        int s = (j->sequencia > 6) ? 6 : j->sequencia;
        int base = 100 * j->fase + (int)(media * 2.0f);
        j->ganhoDia = base * (9 + s) / 10;
        j->pontos += j->ganhoDia;
    }
}

/* ---------------- Minigame dos fios ---------------- */

static Vector2 NoEsq(int i) { return (Vector2){430.0f, 280.0f + i * 80.0f}; }
static Vector2 NoDir(int j) { return (Vector2){850.0f, 280.0f + j * 80.0f}; }

static void MinigameIniciar(Minigame *m, int regiao)
{
    for (int i = 0; i < 4; i++) {
        m->ordem[i] = i;
        m->ligado[i] = false;
        m->ligadoPara[i] = -1;
    }

    for (int i = 3; i > 0; i--) {
        int k = GetRandomValue(0, i);
        int tmp = m->ordem[i];
        m->ordem[i] = m->ordem[k];
        m->ordem[k] = tmp;
    }

    m->arrastando = -1;
    m->tempo = TEMPO_MINIGAME;
    m->regiao = regiao;
}

// Retorna 1 = venceu, -1 = perdeu, 0 = em andamento.
static int MinigameAtualizar(Minigame *m, float dt)
{
    Vector2 mouse = GetMousePosition();
    m->tempo -= dt;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < 4; i++) {
            if (!m->ligado[i] && CheckCollisionPointCircle(mouse, NoEsq(i), 24.0f)) {
                m->arrastando = i;
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && m->arrastando >= 0) {
        int i = m->arrastando;

        for (int j = 0; j < 4; j++) {
            if (CheckCollisionPointCircle(mouse, NoDir(j), 28.0f) && m->ordem[j] == i) {
                m->ligado[i] = true;
                m->ligadoPara[i] = j;
            }
        }

        m->arrastando = -1;
    }

    bool todos = true;
    for (int i = 0; i < 4; i++) {
        if (!m->ligado[i]) todos = false;
    }

    if (todos) return 1;
    if (m->tempo <= 0.0f) return -1;
    return 0;
}

static void MinigameDesenhar(const Minigame *m, const char *nomeRegiao)
{
    char texto[100];
    Vector2 mouse = GetMousePosition();

    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.75f));
    DrawRectangle(330, 150, 620, 430, (Color){22, 28, 52, 255});
    DrawRectangleLines(330, 150, 620, 430, YELLOW);

    snprintf(texto, sizeof(texto), "A REDE DO(A) %s CAIU!", nomeRegiao);
    DrawTextCentro(texto, 640, 168, 26, RED);
    DrawTextCentro("Arraste cada fio ate a ponta da mesma cor", 640, 204, 18, WHITE);

    float p = m->tempo / TEMPO_MINIGAME;
    if (p < 0.0f) p = 0.0f;
    DrawRectangle(380, 240, 520, 10, (Color){40, 45, 70, 255});
    DrawRectangle(380, 240, (int)(520 * p), 10, p > 0.3f ? (Color){80, 220, 150, 255} : RED);

    for (int i = 0; i < 4; i++) {
        if (m->ligado[i]) {
            DrawLineEx(NoEsq(i), NoDir(m->ligadoPara[i]), 9.0f, coresFio[i]);
        }
    }

    if (m->arrastando >= 0) {
        DrawLineEx(NoEsq(m->arrastando), mouse, 9.0f, coresFio[m->arrastando]);
    }

    for (int i = 0; i < 4; i++) {
        Vector2 e = NoEsq(i);
        Vector2 d = NoDir(i);

        DrawRectangle(e.x - 44, e.y - 18, 40, 36, GRAY);
        DrawCircleV(e, 15.0f, coresFio[i]);

        DrawRectangle(d.x + 4, d.y - 18, 40, 36, GRAY);
        DrawCircleV(d, 15.0f, coresFio[m->ordem[i]]);
    }
}

/* ---------------- Desenho do mundo ---------------- */

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