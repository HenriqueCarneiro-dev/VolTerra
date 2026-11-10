#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <time.h>

#define LARGURA 1280
#define ALTURA 720
#define NUM_REGIOES 5
#define TOTAL_FASES 5
#define DIAS_POR_FASE 4
#define DURACAO_DIA 30.0f
#define DURACAO_FASE (DURACAO_DIA * DIAS_POR_FASE)
#define TEMPO_BLACKOUT 10.0f
#define TEMPO_MINIGAME 15.0f
#define TAM_RANKING 5
#define FATOR_SIMULACAO 3.5f
#define FATOR_ECONOMIA 0.55f

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

typedef enum {
    EVENTO_NORMAL,
    EVENTO_CHUVA,
    EVENTO_ENCHENTE,
    EVENTO_VENTOS,
    EVENTO_POLUICAO
} TipoEvento;

typedef enum {
    ESTADO_MENU,
    ESTADO_JOGO,
    ESTADO_MINIGAME,
    ESTADO_FASE,
    ESTADO_FIM
} EstadoJogo;

typedef struct {
    const char *nome;
    const char *fonteNome;
    TipoFonte fonte;
    Color cor;

    Vector2 pontos[16];
    int numPontos;
    Vector2 centro;

    float bateria;
    float producao;
    float consumo;
    float limiteBlackout;
    bool cidadeGrande;

    float economiaTempo;
    float recarga;
    bool ativa;
    bool emBlackout;
    float blackoutTempo;
} Regiao;

typedef struct {
    Regiao regioes[NUM_REGIOES];
    int fase;
    int dia;
    int selecionada;
    float tempoDia;
    float tempoFase;

    int pontos;
    float pontosExibidos;
    int sequencia;
    int ganhoDia;

    TipoEvento evento;
    int origem;
    int destino;
    float linhaTempo;

    char mensagem[160];
    float mensagemTempo;

    bool vitoria;
    int posicaoRanking;
} Jogo;

typedef struct {
    int ordem[4];        // ordem[j] = cor que esta no slot j da direita
    bool ligado[4];
    int ligadoPara[4];
    int arrastando;
    float tempo;
    int regiao;
} Minigame;

static const int ordemAtivacao[NUM_REGIOES] = {SUDESTE, NORDESTE, CENTRO_OESTE, SUL, NORTE};
static const Color coresFio[4] = {
    {230, 60, 60, 255},
    {60, 130, 255, 255},
    {250, 220, 60, 255},
    {70, 210, 100, 255}
};

/* ---------- Utilidades de geometria ---------- */

// Ponto dentro de polígono (ray casting) - funciona para qualquer polígono
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

static bool PontoNoPoligono(Vector2 p, Vector2 *v, int n)
{
    bool dentro = false;

    for (int i = 0, j = n - 1; i < n; j = i++) {
        bool cruza = ((v[i].y > p.y) != (v[j].y >p.y))
        if (cruza) {
            float x = (v[j].x - v[i].x) * (p.y - v[i.y]) / (v[j].y - v[i].y) + v[i].x;
            if (p.x <x) dentro = !dentro;
        }
    }
    return dentro;
}

// Desenha cada triangulo nos dois sentidos: a raylib descarta o sentido errado.
static void PreencherPoligono(Vector2 *v, int n, Vector2 centro, Color cor)
{
    Vector2 c = {0.0f, 0.0f};

    for (int i = 0; i < n; i++) {
        c.x += p[i].x;
        c.y += p[i].y;
    }

    for (int i = 0; i <n; i++) {
        Vector2 a = {p[i].x + dx, p[i].y + dy};
        Vector2 b = {p[(i + 1) % n].x + dx, p[(i + 1) % n].y + dy};
        DrawTriangle(c, a, b, cor);
        DrawTriangle(c, b, a, cor);
    }
    
}

static void DesenharContorno(const Vector2 *p, int n, float espessura, Color cor)
{
    for (int i = 0; i < n; i++)
        DrawLineEx(p[i], p[(i + 1) % n], espessura, cor);
}

/* ---------- Ícones das fontes de energia (desenhados em 2D) ---------- */

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

/*============ FUNCOES AUXILIARES ============*/
static const char *NomeEvento(TipoEvento e){
    switch (e){
        case EVENTO_CHUVA:    return "Chuvas fortes";
        case EVENTO_ENCHENTE: return "Enchente";
        case EVENTO_VENTOS:   return "Ventos fortes";
        case EVENTO_POLUICAO: return "Poluicao";
        default:              return "Tempo estavel";
    }
}   

static TipoEvento SortearEvento(void){
    return (TipoEvento)GetRandomValue(EVENTO_CHUVA, EVENTO_POLUICAO);
}

static void AplicarEvento(Regiao regioes[], TipoEvento e){
    for(int i=0; i< NUM_REGIOES; i++){
        regioes[i].producao = regioes[i].producao;
        regioes[i].consumo = regioes[i].consumo;
    }

    switch (e){
        case EVENTO_CHUVA:  // ajuda a hidreletrica
            regioes[NORTE].producao   *= 1.25f;
            regioes[SUL].producao     *= 1.20f;
            regioes[SUDESTE].producao *= 1.15f;
            break;
        case EVENTO_ENCHENTE:   // prejuca a rede
                      regioes[NORTE].producao    *= 0.65f;
            regioes[NORDESTE].producao *= 0.80f;
            break;
        case EVENTO_VENTOS:     // ajuda a eolica
            regioes[NORDESTE].producao *= 1.35f;
            regioes[SUL].producao      *= 1.15f;
            break;
        case EVENTO_POLUICAO:   // prejudica a solar e aumenta a demanda
            regioes[CENTRO_OESTE].producao *= 0.70f;
            regioes[SUDESTE].consumo       *= 1.35f;
            regioes[NORDESTE].consumo      *= 1.15f;
            break;
        default:
            break;  

    }
}

// desenhar barra baretia (aqui):
//
//

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