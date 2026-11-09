#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define LARGURA 1280
#define ALTURA 720
#define NUM_REGIOES 5
#define DURACAO_NOITE 20.0f

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
    
    Vector2 pontos[8];      // polígono da região (convexo, sentido horário)
    int numPontos;
    Vector2 centro;         // onde desenhar o ícone da fonte

    float bateria;          // 0 a 100
    float producao;         
    float consumo;          // 0 A 150
    float limiteBlackout;  // % minima de bateria, abaixo disso a regiao sofre blackout
    bool  cidadeGrande;     // if TRUE -> limite blackout (50%), else -> limite blackout (20%)
    float economiaTempo;    // Seg para ativar a economia emergencial;
} Regiao;

//adicao dos eventos:
typedef enum{
    EVENTO_NORMAL,
    EVENTO_CHUVA,
    EVENTO_ENCHENTE,
    EVENTO_VENTOS,
    EVENTO_POLUICAO,
} TipoEvento;

/* ---------- Utilidades de geometria ---------- */

// Ponto dentro de polígono (ray casting) - funciona para qualquer polígono
static bool PontoNoPoligono(Vector2 p, Vector2 *v, int n)
{
    bool dentro = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        bool cruza =  ((v[i].y > p.y) != (v[j].y > p.y));

        if (cruza){
            float intersecaoX = 
                (v[j].x - v[i].x) * (p.y - v[i].y)/
                (v[j].y - v[i].y) + v[i].x;

            if(p.x < intersecaoX){
                dentro = !dentro;
            }
        }
    }

    return dentro;
}

// Preenche polígono qualquer usando triângulos a partir do centro (leque)
static void PreencherPoligono(Vector2 *pontos, int quantidade, Color cor)
{
    Vector2 centro = {0.0f, 0.0f};

    for(int i=0;  i< quantidade; i++){
        centro.x += pontos[i].x;
        centro.y += pontos[i].y;
    }

    centro.x /= quantidade;
    centro.y /= quantidade;

    for(int i=0; i<quantidade; i++){
        DrawTriangle(
            centro,
            pontos[i],
            pontos[(i + 1) % quantidade],
            cor
        );
    }
}

static void DesenharContorno(Vector2 *pontos, int quantidade, float espessura, Color cor){
    for(int i=0; i<quantidade; i++){
        DrawLineEx(
            pontos[i],
            pontos[(i+1) % quantidade],
            espessura,
            cor
        );
    }
}

static void DesenhoIconeFonte(TipoFonte fonte, Vector2 c, float tempo){
switch (fonte) {
        case FONTE_HIDRO:
            DrawRectangle(c.x - 18, c.y - 10, 36, 22, (Color){65, 75, 95, 255});
            DrawRectangle(c.x - 23, c.y + 12, 46, 5, (Color){50, 140, 240, 255});

            for (int i = 0; i < 3; i++) {
                float deslocamento = fmodf(tempo * 24.0f + i * 9.0f, 18.0f);
                DrawCircle(
                    c.x - 10 + i * 10,
                    c.y - 4 + deslocamento,
                    2.5f,
                    SKYBLUE
                );
            }
            break;

        case FONTE_EOLICA: {
            float angulo = tempo * 120.0f;

            DrawLineEx(
                (Vector2){c.x, c.y - 5},
                (Vector2){c.x, c.y + 23},
                4,
                RAYWHITE
            );

            for (int i = 0; i < 3; i++) {
                float a = (angulo + i * 120.0f) * DEG2RAD;
                Vector2 ponta = {
                    c.x + cosf(a) * 23.0f,
                    c.y - 5 + sinf(a) * 23.0f
                };

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
            DrawTriangle(
                (Vector2){c.x - 19, c.y + 19},
                (Vector2){c.x + 19, c.y + 19},
                (Vector2){c.x - 10, c.y - 13},
                LIGHTGRAY
            );

            DrawTriangle(
                (Vector2){c.x + 10, c.y - 13},
                (Vector2){c.x - 10, c.y - 13},
                (Vector2){c.x + 19, c.y + 19},
                LIGHTGRAY
            );

            DrawCircle(c.x + 21, c.y - 17, 6, Fade(WHITE, 0.75f));
            DrawCircle(c.x + 29, c.y - 22, 5, Fade(WHITE, 0.5f));
            break;

        case FONTE_BIOMASSA:
            DrawEllipse(c.x, c.y, 12, 19, GREEN);
            DrawLineEx(
                (Vector2){c.x, c.y + 19},
                (Vector2){c.x, c.y - 16},
                2,
                DARKGREEN
            );
            DrawTriangle(
                (Vector2){c.x - 6, c.y + 25},
                (Vector2){c.x + 6, c.y + 25},
                (Vector2){c.x, c.y + 12 + sinf(tempo * 5.0f) * 2.0f},
                ORANGE
            );
            break;
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

static Color CorBateria(float bateria){
    if(bateria < 30.0f) return RED;
    if(bateria < 55.0f) return ORANGE;    //mudanca de cor por regiao diante a bateria dele;;
    return (Color){80, 220, 150, 255};
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

static void DesenharBarraBateria(float x, float y, float largura, float altura,
                                 float bateria, float limite)
{
    DrawRectangle(x, y, largura, altura, (Color){25, 30, 45, 255});

    float preenchimento = bateria / 100.0f;
    if (preenchimento < 0.0f) preenchimento = 0.0f;
    if (preenchimento > 1.0f) preenchimento = 1.0f;

    DrawRectangle(
        x + 2,
        y + 2,
        (largura - 4) * preenchimento,
        altura - 4,
        CorBateria(bateria)
    );

    float posicaoLimite = x + largura * (limite / 100.0f);
    DrawLineEx(
        (Vector2){posicaoLimite, y - 3},
        (Vector2){posicaoLimite, y + altura + 3},
        2,
        WHITE
    );

    DrawRectangleLines(x, y, largura, altura, LIGHTGRAY);
}

/* ---------- Programa principal ---------- */

int main(void)
{
    InitWindow(LARGURA, ALTURA, "VolTerra - A crise das cinco noites");
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)GetTime());
    
    Regiao regioes[NUM_REGIOES] = {
        {
            "Norte", "Hidreletrica", FONTE_HIDRO, (Color){46, 139, 87, 255},
            {{380,90}, {640,70}, {760,150}, {740,260}, {560,290}, {400,250}, {340,170}},
            7, {560,180}, 100.0f, 0.90f, 0.72f, 20.0f, false, 0.0f
        },
        {
            "Nordeste", "Eolica", FONTE_EOLICA, (Color){218, 165, 32, 255},
            {{760,150}, {900,170}, {960,260}, {900,350}, {780,380}, {740,260}},
            6, {850,270}, 100.0f, 0.95f, 0.72f, 50.0f, true, 0.0f
        },
        {
            "Centro-Oeste", "Solar", FONTE_SOLAR, (Color){210, 105, 30, 255},
            {{560,290}, {740,260}, {780,380}, {720,450}, {560,450}, {470,360}},
            6, {630,365}, 100.0f, 0.88f, 0.70f, 20.0f, false, 0.0f
        },
        {
            "Sudeste", "Nuclear", FONTE_NUCLEAR, (Color){160, 82, 160, 255},
            {{780,380}, {900,350}, {880,450}, {800,520}, {720,500}, {720,450}},
            6, {800,440}, 100.0f, 0.92f, 0.78f, 50.0f, true, 0.0f
        },
        {
            "Sul", "Hidreletrica", FONTE_HIDRO, (Color){65, 105, 225, 255},
            {{560,450}, {720,450}, {720,500}, {680,580}, {600,610}, {540,540}},
            6, {630,520}, 100.0f, 0.88f, 0.70f, 20.0f, false, 0.0f
        }
    };

    int selecionada = NORTE;
    int noite = 1;
    int origemTransferencia = -1;

    float tempoNoite = 0.0f;
    float tempoJogo = 0.0f;
    float mensagemTempo = 0.0f;

    bool jogoEncerrado = false;
    bool vitoria = false;

    char mensagem[160] = "Mantenha as cinco regioes acima do limite!";
    TipoEvento evento = EVENTO_NORMAL;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float tempo = (float)GetTime();
        Vector2 mouse = GetMousePosition();

        if (!jogoEncerrado) {
            tempoNoite += dt;
            tempoJogo += dt;

            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) ||
                IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D)) {
                int novo = selecionada;

                if (IsKeyPressed(KEY_W)) {
                    if (selecionada == SUL) novo = CENTRO_OESTE;
                    else if (selecionada == CENTRO_OESTE) novo = NORTE;
                    else if (selecionada == SUDESTE) novo = NORDESTE;
                }

                if (IsKeyPressed(KEY_S)) {
                    if (selecionada == NORTE) novo = CENTRO_OESTE;
                    else if (selecionada == CENTRO_OESTE) novo = SUL;
                    else if (selecionada == NORDESTE) novo = SUDESTE;
                }

                if (IsKeyPressed(KEY_A)) {
                    if (selecionada == NORDESTE) novo = NORTE;
                    else if (selecionada == SUDESTE) novo = CENTRO_OESTE;
                    else if (selecionada == CENTRO_OESTE) novo = SUL;
                }

                if (IsKeyPressed(KEY_D)) {
                    if (selecionada == NORTE) novo = NORDESTE;
                    else if (selecionada == CENTRO_OESTE) novo = SUDESTE;
                    else if (selecionada == SUL) novo = CENTRO_OESTE;
                }

                selecionada = novo;
            }

            for (int i = 0; i < NUM_REGIOES; i++) {
                if (PontoNoPoligono(mouse, regioes[i].pontos, regioes[i].numPontos) &&
                    IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    selecionada = i;
                }
            }

            // E: seleciona a regiao de onde enviar energia.
            if (IsKeyPressed(KEY_E)) {
                int melhorOrigem = -1;
                float maiorBateria = regioes[selecionada].bateria;

                for (int i = 0; i < NUM_REGIOES; i++) {
                    if (i != selecionada && regioes[i].bateria > maiorBateria) {
                        maiorBateria = regioes[i].bateria;
                        melhorOrigem = i;
                    }
                }

                if (melhorOrigem >= 0 && regioes[melhorOrigem].bateria >= 12.0f) {
                    regioes[melhorOrigem].bateria -= 12.0f;
                    regioes[selecionada].bateria += 9.0f;

                    if (regioes[selecionada].bateria > 100.0f) {
                        regioes[selecionada].bateria = 100.0f;
                    }

                    origemTransferencia = melhorOrigem;
                    snprintf(
                        mensagem, sizeof(mensagem),
                        "Energia enviada de %s para %s: 12%% enviados, 9%% recebidos.",
                        regioes[melhorOrigem].nome,
                        regioes[selecionada].nome
                    );
                    mensagemTempo = 4.0f;
                } else {
                    snprintf(
                        mensagem, sizeof(mensagem),
                        "Nao ha outra regiao com energia suficiente para transferir."
                    );
                    mensagemTempo = 3.0f;
                }
            }

            // Espaco: reduz temporariamente o consumo da regiao selecionada.
            if (IsKeyPressed(KEY_SPACE)) {
                if (regioes[selecionada].economiaTempo <= 0.0f) {
                    regioes[selecionada].economiaTempo = 8.0f;
                    snprintf(
                        mensagem, sizeof(mensagem),
                        "Economia emergencial ativada em %s por 8 segundos!",
                        regioes[selecionada].nome
                    );
                    mensagemTempo = 4.0f;
                } else {
                    snprintf(mensagem, sizeof(mensagem), "A economia emergencial ja esta ativa.");
                    mensagemTempo = 3.0f;
                }
            }

            // Avanco de noite.
            if (tempoNoite >= DURACAO_NOITE) {
                tempoNoite = 0.0f;
                noite++;

                if (noite > 5) {
                    jogoEncerrado = true;
                    vitoria = true;
                    snprintf(mensagem, sizeof(mensagem),
                             "Voce sobreviveu as cinco noites e manteve o Brasil aceso!");
                } else {
                    evento = SortearEvento();
                    AplicarEvento(regioes, evento);

                    snprintf(
                        mensagem, sizeof(mensagem),
                        "Noite %d: evento da noite - %s!",
                        noite, NomeEvento(evento)
                    );
                    mensagemTempo = DURACAO_NOITE;
                }
            }

            if (mensagemTempo > 0.0f) {
                mensagemTempo -= dt;
            }

            // Atualiza baterias. Produção e consumo são taxas por segundo de jogo.
            for (int i = 0; i < NUM_REGIOES; i++) {
                Regiao *r = &regioes[i];

                if (r->economiaTempo > 0.0f) {
                    r->economiaTempo -= dt;
                }

                float consumoAtual = r->consumo;

                if (r->economiaTempo > 0.0f) {
                    consumoAtual *= 0.55f;
                }

                float variacao = (r->producao - consumoAtual) * dt * 2.2f;
                r->bateria += variacao;

                if (r->bateria > 100.0f) r->bateria = 100.0f;
                if (r->bateria < 0.0f) r->bateria = 0.0f;

                if (r->bateria < r->limiteBlackout) {
                    jogoEncerrado = true;
                    vitoria = false;

                    snprintf(
                        mensagem, sizeof(mensagem),
                        "BLACKOUT! %s caiu abaixo do limite de %.0f%%.",
                        r->nome, r->limiteBlackout
                    );
                    break;
                }
            }
        }

        BeginDrawing();
        ClearBackground((Color){10, 14, 30, 255});

        // Fundo decorativo.
        for (int i = 0; i < 55; i++) {
            int x = (i * 197) % LARGURA;
            int y = (i * 91) % 610;
            float brilho = 0.25f + 0.5f * fabsf(sinf(tempo + i));
            DrawPixel(x, y, Fade(WHITE, brilho));
        }

        DrawText("VOLTERRA", 28, 17, 38, (Color){255, 220, 80, 255});
        DrawText("A crise das cinco noites", 30, 57, 18, LIGHTGRAY);

        char textoNoite[80];
        if (noite <= 5) {
            snprintf(textoNoite, sizeof(textoNoite), "NOITE %d / 5", noite);
        } else {
            snprintf(textoNoite, sizeof(textoNoite), "FIM DA PARTIDA");
        }

        DrawText(textoNoite, 1010, 24, 24, YELLOW);

        float restante = DURACAO_NOITE - tempoNoite;
        if (restante < 0.0f) restante = 0.0f;

        char textoTempo[80];
        snprintf(textoTempo, sizeof(textoTempo), "Proxima noite: %.0f s", restante);
        DrawText(textoTempo, 1010, 54, 17, LIGHTGRAY);

        // Regiões do mapa.
        for (int i = 0; i < NUM_REGIOES; i++) {
            Regiao *r = &regioes[i];
            Color cor = r->cor;
            bool sobre = PontoNoPoligono(mouse, r->pontos, r->numPontos);

            if (sobre) {
                cor = ColorBrightness(cor, 0.15f);
            }

            PreencherPoligono(r->pontos, r->numPontos, Fade(BLACK, 0.35f));
            PreencherPoligono(r->pontos, r->numPontos, cor);

            DesenharContorno(
                r->pontos,
                r->numPontos,
                i == selecionada ? 5.0f : 2.0f,
                i == selecionada ? WHITE : (Color){20, 20, 30, 255}
            );

            DesenharIconeFonte(r->fonte, r->centro, tempo);

            int larguraNome = MeasureText(r->nome, 17);
            DrawText(
                r->nome,
                r->centro.x - larguraNome / 2,
                r->centro.y + 33,
                17,
                WHITE
            );

            char bateriaTexto[24];
            snprintf(bateriaTexto, sizeof(bateriaTexto), "%.0f%%", r->bateria);

            DrawText(
                bateriaTexto,
                r->centro.x - 22,
                r->centro.y + 54,
                17,
                CorBateria(r->bateria)
            );
        }

        // Painel inferior.
        DrawRectangle(0, 625, LARGURA, 95, (Color){20, 26, 50, 255});
        DrawLine(0, 625, LARGURA, 625, (Color){255, 220, 80, 255});

        DrawText("ENERGIA DAS REGIOES", 28, 637, 17, YELLOW);

        for (int i = 0; i < NUM_REGIOES; i++) {
            int x = 28 + i * 245;
            Regiao *r = &regioes[i];

            DrawText(r->nome, x, 660, 15, WHITE);
            DesenharBarraBateria(
                x,
                682,
                190,
                16,
                r->bateria,
                r->limiteBlackout
            );

            char limiteTexto[32];
            snprintf(
                limiteTexto,
                sizeof(limiteTexto),
                "limite %.0f%%",
                r->limiteBlackout
            );
            DrawText(limiteTexto, x + 194, 681, 13, LIGHTGRAY);
        }

        // Instruções e evento atual.
        DrawRectangle(20, 94, 265, 90, Fade((Color){15, 20, 38, 255}, 0.92f));
        DrawRectangleLines(20, 94, 265, 90, (Color){100, 120, 160, 255});
        DrawText("CONTROLES", 34, 105, 17, YELLOW);
        DrawText("WASD / clique: selecionar", 34, 130, 15, WHITE);
        DrawText("E: transferir energia", 34, 151, 15, WHITE);
        DrawText("ESPACO: economizar energia", 34, 171, 15, WHITE);

        DrawRectangle(965, 95, 285, 70, Fade((Color){15, 20, 38, 255}, 0.92f));
        DrawRectangleLines(965, 95, 285, 70, (Color){100, 120, 160, 255});
        DrawText("EVENTO", 980, 105, 16, YELLOW);
        DrawText(NomeEvento(evento), 980, 130, 20, WHITE);

        // Detalhes da região selecionada.
        Regiao *r = &regioes[selecionada];
        DrawRectangle(965, 180, 285, 105, Fade((Color){15, 20, 38, 255}, 0.92f));
        DrawRectangleLines(965, 180, 285, 105, (Color){100, 120, 160, 255});

        DrawText(r->nome, 980, 191, 21, WHITE);
        DrawText(r->fonteNome, 980, 219, 16, LIGHTGRAY);

        char limiteRegiao[80];
        snprintf(
            limiteRegiao,
            sizeof(limiteRegiao),
            "%s | blackout abaixo de %.0f%%",
            r->cidadeGrande ? "Cidade grande" : "Cidade pequena",
            r->limiteBlackout
        );
        DrawText(limiteRegiao, 980, 246, 14, YELLOW);

        // Mensagens temporarias.
        if (mensagemTempo > 0.0f || jogoEncerrado) {
            int larguraMensagem = MeasureText(mensagem, 18);
            DrawRectangle(
                LARGURA / 2 - larguraMensagem / 2 - 18,
                590,
                larguraMensagem + 36,
                28,
                Fade((Color){8, 12, 24, 255}, 0.9f)
            );
            DrawText(
                mensagem,
                LARGURA / 2 - larguraMensagem / 2,
                595,
                18,
                jogoEncerrado && !vitoria ? RED : WHITE
            );
        }

        // Tela de fim de jogo.
        if (jogoEncerrado) {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.72f));

            const char *titulo = vitoria ? "BRASIL SALVO!" : "APAGAO NACIONAL";
            Color corTitulo = vitoria ? (Color){100, 240, 150, 255} : RED;
            int larguraTitulo = MeasureText(titulo, 48);

            DrawText(
                titulo,
                LARGURA / 2 - larguraTitulo / 2,
                260,
                48,
                corTitulo
            );

            const char *subtitulo = vitoria
                ? "Voce sobreviveu as cinco noites."
                : mensagem;

            int larguraSubtitulo = MeasureText(subtitulo, 21);
            DrawText(
                subtitulo,
                LARGURA / 2 - larguraSubtitulo / 2,
                325,
                21,
                WHITE
            );

            DrawText(
                "Feche a janela para sair.",
                LARGURA / 2 - MeasureText("Feche a janela para sair.", 18) / 2,
                370,
                18,
                LIGHTGRAY
            );
        }

        DrawFPS(LARGURA - 75, ALTURA - 25);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}