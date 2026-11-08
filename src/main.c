#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define LARGURA 1280
#define ALTURA  720
#define NUM_REGIOES 5

typedef enum {
    NORTE = 0,
    NORDESTE,
    CENTRO_OESTE,
    SUDESTE,
    SUL
} IdRegiao;

typedef enum {
    FONTE_HIDRO,
    FONTE_EOLICA,
    FONTE_SOLAR,
    FONTE_NUCLEAR,
    FONTE_BIOMASSA
} TipoFonte;

typedef struct {
    const char *nome;
    const char *fonteNome;
    TipoFonte fonte;
    Color cor;
    Vector2 pontos[8];   // polígono da região (convexo, sentido horário)
    int numPontos;
    Vector2 centro;      // onde desenhar o ícone da fonte
    float bateria;       // 0 a 100
} Regiao;

/* ---------- Utilidades de geometria ---------- */

// Ponto dentro de polígono (ray casting) - funciona para qualquer polígono
static bool PontoNoPoligono(Vector2 p, Vector2 *v, int n)
{
    bool dentro = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((v[i].y > p.y) != (v[j].y > p.y)) &&
            (p.x < (v[j].x - v[i].x) * (p.y - v[i].y) / (v[j].y - v[i].y) + v[i].x))
            dentro = !dentro;
    }
    return dentro;
}

// Preenche polígono qualquer usando triângulos a partir do centro (leque)
static void PreencherPoligono(Vector2 *v, int n, Vector2 centro, Color cor)
{
    for (int i = 0; i < n; i++) {
        Vector2 a = v[i];
        Vector2 b = v[(i + 1) % n];
        // garante sentido anti-horário exigido pelo raylib
        Vector2 ab = { b.x - a.x, b.y - a.y };
        Vector2 ac = { centro.x - a.x, centro.y - a.y };
        float cruz = ab.x * ac.y - ab.y * ac.x;
        if (cruz > 0) DrawTriangle(centro, b, a, cor);
        else          DrawTriangle(centro, a, b, cor);
    }
}

static void ContornoPoligono(Vector2 *v, int n, float espessura, Color cor)
{
    for (int i = 0; i < n; i++)
        DrawLineEx(v[i], v[(i + 1) % n], espessura, cor);
}

/* ---------- Ícones das fontes de energia (desenhados em 2D) ---------- */

static void IconeHidro(Vector2 c, float t)
{
    // barragem + gotas de água
    DrawRectangle(c.x - 22, c.y - 14, 44, 28, (Color){70, 70, 80, 255});
    DrawRectangle(c.x - 22, c.y - 14, 44, 6, (Color){110, 110, 120, 255});
    for (int i = 0; i < 3; i++) {
        float off = fmodf(t * 30 + i * 8, 24);
        DrawRectangle(c.x - 12 + i * 12, c.y - 8 + off * 0.8f, 4, 6, (Color){80, 170, 255, 255});
    }
    DrawRectangle(c.x - 26, c.y + 14, 52, 5, (Color){50, 120, 220, 255});
}

static void IconeEolica(Vector2 c, float t)
{
    // torre + pás giratórias
    DrawTriangle((Vector2){c.x - 4, c.y + 24}, (Vector2){c.x + 4, c.y + 24},
                 (Vector2){c.x, c.y - 4}, RAYWHITE);
    float ang = t * 120.0f;
    for (int i = 0; i < 3; i++) {
        float a = (ang + i * 120.0f) * DEG2RAD;
        Vector2 ponta = { c.x + cosf(a) * 24, c.y - 4 + sinf(a) * 24 };
        DrawLineEx((Vector2){c.x, c.y - 4}, ponta, 3, RAYWHITE);
    }
    DrawCircle(c.x, c.y - 4, 4, LIGHTGRAY);
}

static void IconeSolar(Vector2 c, float t)
{
    // painel inclinado + sol com raios
    Vector2 sol = { c.x + 16, c.y - 16 };
    DrawCircleV(sol, 7, YELLOW);
    for (int i = 0; i < 8; i++) {
        float a = (t * 40 + i * 45) * DEG2RAD;
        DrawLineEx((Vector2){sol.x + cosf(a) * 10, sol.y + sinf(a) * 10},
                   (Vector2){sol.x + cosf(a) * 15, sol.y + sinf(a) * 15}, 2, YELLOW);
    }
    DrawRectangle(c.x - 22, c.y - 4, 34, 20, (Color){20, 50, 120, 255});
    for (int i = 1; i < 3; i++)
        DrawLine(c.x - 22 + i * 11, c.y - 4, c.x - 22 + i * 11, c.y + 16, SKYBLUE);
    DrawLine(c.x - 22, c.y + 6, c.x + 12, c.y + 6, SKYBLUE);
}

static void IconeNuclear(Vector2 c, float t)
{
    // torre de resfriamento + fumaça
    DrawTriangle((Vector2){c.x - 16, c.y + 20}, (Vector2){c.x + 16, c.y + 20},
                 (Vector2){c.x - 9, c.y - 12}, LIGHTGRAY);
    DrawTriangle((Vector2){c.x + 9, c.y - 12}, (Vector2){c.x - 9, c.y - 12},
                 (Vector2){c.x + 16, c.y + 20}, LIGHTGRAY);
    float f = fmodf(t * 15, 20);
    DrawCircle(c.x, c.y - 16 - f, 6 + f * 0.2f, Fade(WHITE, 1.0f - f / 20.0f));
}

static void IconeBiomassa(Vector2 c, float t)
{
    // folha + chama
    DrawEllipse(c.x, c.y, 14, 22, (Color){40, 160, 70, 255});
    DrawLineEx((Vector2){c.x, c.y + 22}, (Vector2){c.x, c.y - 18}, 2, (Color){20, 100, 40, 255});
    float osc = sinf(t * 6) * 2;
    DrawTriangle((Vector2){c.x - 6, c.y + 26}, (Vector2){c.x + 6, c.y + 26},
                 (Vector2){c.x + osc, c.y + 14}, ORANGE);
}

static void DesenharIcone(TipoFonte f, Vector2 c, float t)
{
    switch (f) {
        case FONTE_HIDRO:    IconeHidro(c, t); break;
        case FONTE_EOLICA:   IconeEolica(c, t); break;
        case FONTE_SOLAR:    IconeSolar(c, t); break;
        case FONTE_NUCLEAR:  IconeNuclear(c, t); break;
        case FONTE_BIOMASSA: IconeBiomassa(c, t); break;
    }
}

/* ---------- Programa principal ---------- */

int main(void)
{
    InitWindow(LARGURA, ALTURA, "VolTerra - Parte 1");
    SetTargetFPS(60);

    // Mapa estilizado (coordenadas em pixels, região central da tela)
    Regiao regioes[NUM_REGIOES] = {
        { "Norte", "Hidreletrica", FONTE_HIDRO, (Color){46, 139, 87, 255},
          { {380,90}, {640,70}, {760,150}, {740,260}, {560,290}, {400,250}, {340,170} }, 7,
          {560, 180}, 100 },

        { "Nordeste", "Eolica", FONTE_EOLICA, (Color){218, 165, 32, 255},
          { {760,150}, {900,170}, {960,260}, {900,350}, {780,380}, {740,260} }, 6,
          {850, 270}, 100 },

        { "Centro-Oeste", "Solar", FONTE_SOLAR, (Color){210, 105, 30, 255},
          { {560,290}, {740,260}, {780,380}, {720,450}, {560,450}, {470,360} }, 6,
          {630, 365}, 100 },

        { "Sudeste", "Nuclear", FONTE_NUCLEAR, (Color){160, 82, 160, 255},
          { {780,380}, {900,350}, {880,450}, {800,520}, {720,500}, {720,450} }, 6,
          {800, 440}, 100 },

        { "Sul", "Hidreletrica", FONTE_HIDRO, (Color){65, 105, 225, 255},
          { {560,450}, {720,450}, {720,500}, {680,580}, {600,610}, {540,540} }, 6,
          {630, 520}, 100 }
    };

    int selecionada = 0;   // região selecionada (WASD ou mouse)
    int hover = -1;

    while (!WindowShouldClose()) {
        float t = (float)GetTime();
        Vector2 mouse = GetMousePosition();

        /* --- Entrada: mouse --- */
        hover = -1;
        for (int i = 0; i < NUM_REGIOES; i++) {
            if (PontoNoPoligono(mouse, regioes[i].pontos, regioes[i].numPontos)) {
                hover = i;
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selecionada = i;
            }
        }

        /* --- Entrada: WASD (move a seleção para a região mais próxima na direção) --- */
        Vector2 dir = {0, 0};
        if (IsKeyPressed(KEY_W)) dir.y = -1;
        if (IsKeyPressed(KEY_S)) dir.y =  1;
        if (IsKeyPressed(KEY_A)) dir.x = -1;
        if (IsKeyPressed(KEY_D)) dir.x =  1;

        if (dir.x != 0 || dir.y != 0) {
            int melhor = selecionada;
            float melhorDist = 1e9f;
            Vector2 origem = regioes[selecionada].centro;
            for (int i = 0; i < NUM_REGIOES; i++) {
                if (i == selecionada) continue;
                Vector2 d = { regioes[i].centro.x - origem.x, regioes[i].centro.y - origem.y };
                float proj = d.x * dir.x + d.y * dir.y;   // quanto está na direção
                if (proj <= 0) continue;
                float dist = sqrtf(d.x * d.x + d.y * d.y);
                if (dist < melhorDist) { melhorDist = dist; melhor = i; }
            }
            selecionada = melhor;
        }

        /* --- Desenho --- */
        BeginDrawing();
        ClearBackground((Color){10, 14, 30, 255});

        // Título
        DrawText("VOLTERRA", 30, 20, 40, (Color){255, 220, 80, 255});
        DrawText("Controle a energia do Brasil", 30, 66, 18, LIGHTGRAY);

        // Estrelas decorativas do fundo
        for (int i = 0; i < 60; i++) {
            int x = (i * 197) % LARGURA;
            int y = (i * 91) % 560;
            float brilho = 0.4f + 0.6f * fabsf(sinf(t + i));
            DrawPixel(x, y, Fade(WHITE, brilho));
        }

        // Regiões
        for (int i = 0; i < NUM_REGIOES; i++) {
            Regiao *r = &regioes[i];
            Color c = r->cor;
            if (i == hover) c = ColorBrightness(c, 0.2f);
            // sombra
            PreencherPoligono(r->pontos, r->numPontos, r->centro, Fade(BLACK, 0.35f));
            PreencherPoligono(r->pontos, r->numPontos, r->centro, c);
            ContornoPoligono(r->pontos, r->numPontos, i == selecionada ? 5 : 2,
                             i == selecionada ? WHITE : (Color){20, 20, 30, 255});
        }

        // Ícones e nomes
        for (int i = 0; i < NUM_REGIOES; i++) {
            DesenharIcone(regioes[i].fonte, regioes[i].centro, t);
            int w = MeasureText(regioes[i].nome, 18);
            DrawText(regioes[i].nome, regioes[i].centro.x - w / 2,
                     regioes[i].centro.y + 34, 18, WHITE);
        }

        // Painel inferior (controles)
        DrawRectangle(0, 630, LARGURA, 90, (Color){20, 26, 50, 255});
        DrawLine(0, 630, LARGURA, 630, (Color){255, 220, 80, 255});
        char info[128];
        snprintf(info, sizeof(info), "Regiao: %s | Fonte: %s | Bateria: %.0f%%",
                 regioes[selecionada].nome, regioes[selecionada].fonteNome,
                 regioes[selecionada].bateria);
        DrawText(info, 30, 645, 22, WHITE);
        DrawText("Mouse: clique na regiao   |   WASD: trocar de regiao   |   ESC: sair",
                 30, 685, 18, LIGHTGRAY);

        DrawFPS(LARGURA - 90, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}