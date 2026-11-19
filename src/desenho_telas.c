#include "tipos.h"
#include "globais.h"
#include "dados.h"
#include "desenho_jogo.h"
#include "minigames.h"
#include "ui.h"
#include "util.h"
#include "desenho_telas.h"

static void desenharVolumes(void);

static const char *nomeAba(int i)
{
    static char buf[NUM_RANK][32];
    if (i == NUM_RANK - 1) snprintf(buf[i], sizeof(buf[i]), "Secreta");
    else snprintf(buf[i], sizeof(buf[i]), "Fase %d", i + 1);
    return buf[i];
}

static void desenharRankingLista(int cx, int y, int idx, int max)
{
    textoCentro(TextFormat("RANQUE - %s", nomeAba(idx)), cx, y, 20, (Color){255, 215, 60, 255});
    int i = 0;
    for (No *a = rankingLista[idx]; a && i < max; a = a->prox, i++) {
        Color c = (posRanking == i + 1 && tela == TELA_FIM) ? YELLOW : LIGHTGRAY;
        int ly = y + 30 + i * 24;
        DrawText(TextFormat("%d. %s", i + 1, a->nome), cx - 160, ly, 20, c);
        const char *pt = TextFormat("%d", a->pontos);
        DrawText(pt, cx + 160 - MeasureText(pt, 20), ly, 20, c);
    }
    if (i == 0) textoCentro("Ainda sem registros.", cx, y + 30, 18, GRAY);
}

static void desenharRank(void)
{
    int nAbas = abasVisiveis();
    if (abaRank >= nAbas) abaRank = 0;

    textoCentro("RANQUE", 640, 60, 60, (Color){255, 215, 60, 255});
    textoCentro("Melhores pontuacoes de cada fase", 640, 120, 18, LIGHTGRAY);

    for (int i = 0; i < nAbas; i++) {
        Rectangle rc = abaRect(i, nAbas);
        bool sel = (i == abaRank);
        bool sec = (i == NUM_RANK - 1);
        Color base = sec ? (Color){150, 110, 20, 255} : (Color){40, 100, 180, 255};
        Color fill = sel ? ColorBrightness(base, 0.15f) : ColorBrightness(base, -0.45f);
        caixa(rc, 0.25f, hover(rc) && !sel ? ColorBrightness(fill, 0.2f) : fill,
              sel ? (Color){255, 215, 60, 255} : Fade(WHITE, 0.3f));
        textoCentro(nomeAba(i), (int)(rc.x + rc.width / 2), (int)rc.y + 11, 22, sel ? WHITE : LIGHTGRAY);
    }

    caixa((Rectangle){340, 220, 600, 395}, 0.03f, (Color){14, 26, 48, 245}, (Color){60, 90, 130, 255});
    textoCentro(abaRank == NUM_RANK - 1 ? "FASE SECRETA" : TextFormat("FASE %d", abaRank + 1), 640, 232, 26,
                (Color){120, 200, 255, 255});
    DrawText("#", 370, 270, 16, (Color){150, 170, 200, 255});
    DrawText("Jogador", 420, 270, 16, (Color){150, 170, 200, 255});
    DrawText("Pontos", 880 - MeasureText("Pontos", 16), 270, 16, (Color){150, 170, 200, 255});
    DrawRectangle(360, 292, 560, 2, (Color){60, 90, 130, 255});

    int i = 0;
    for (No *a = rankingLista[abaRank]; a && i < MAX_RANK; a = a->prox, i++) {
        Color c = i == 0 ? (Color){255, 215, 60, 255} : (i == 1 ? (Color){210, 215, 225, 255}
                : (i == 2 ? (Color){210, 140, 80, 255} : RAYWHITE));
        int ly = 300 + i * 30;
        DrawText(TextFormat("%d", i + 1), 370, ly, 22, c);
        DrawText(a->nome, 420, ly, 22, c);
        const char *pt = TextFormat("%d", a->pontos);
        DrawText(pt, 880 - MeasureText(pt, 22), ly, 22, c);
    }
    if (i == 0) textoCentro("Ainda sem registros nesta fase.", 640, 400, 22, GRAY);

    desenharBotao(btnVoltar, "Menu", (Color){160, 55, 55, 255});
}

/*  Menu principal  */

static Font fTitulo, fCorpo, fCorpoMed, fPeq, fNum;
static bool fontesProprias = false;

/* Metricas das fontes (altura das maiusculas e centro delas, em fracao do tamanho). */
#define CAP_TITULO    0.5833f
#define CENTRO_TITULO 0.4750f
#define CENTRO_CORPO  0.5450f

void carregarFontesMenu(void)
{
    int cps[200];
    int n = 0;
    for (int c = 32; c <= 126; c++) cps[n++] = c;
    for (int c = 160; c <= 255; c++) cps[n++] = c;   /* acentos do portugues */
    cps[n++] = 0x2022;                               /* marcador do rodape */

    fontesProprias = FileExists("assets/fonts/PixelifySans-Bold.ttf") &&
                     FileExists("assets/fonts/Exo2-SemiBold.ttf") &&
                     FileExists("assets/fonts/Exo2-Medium.ttf");

    if (fontesProprias) {
        fTitulo   = LoadFontEx("assets/fonts/PixelifySans-Bold.ttf", 104, cps, n);
        fCorpo    = LoadFontEx("assets/fonts/Exo2-SemiBold.ttf", 24, cps, n);
        fCorpoMed = LoadFontEx("assets/fonts/Exo2-Medium.ttf", 18, cps, n);
        fPeq      = LoadFontEx("assets/fonts/Exo2-SemiBold.ttf", 14, cps, n);
        fNum      = LoadFontEx("assets/fonts/PixelifySans-Bold.ttf", 44, cps, n);
        if (fTitulo.texture.id == 0 || fCorpo.texture.id == 0 || fCorpoMed.texture.id == 0 ||
            fPeq.texture.id == 0 || fNum.texture.id == 0) {
            fontesProprias = false;
        } else {
            SetTextureFilter(fPeq.texture, TEXTURE_FILTER_BILINEAR);
            SetTextureFilter(fNum.texture, TEXTURE_FILTER_BILINEAR);
            SetTextureFilter(fTitulo.texture, TEXTURE_FILTER_BILINEAR);
            SetTextureFilter(fCorpo.texture, TEXTURE_FILTER_BILINEAR);
            SetTextureFilter(fCorpoMed.texture, TEXTURE_FILTER_BILINEAR);
        }
    }

    if (!fontesProprias) {
        /* Plano B: fonte padrao da raylib (o jogo continua funcionando sem os arquivos). */
        fTitulo = GetFontDefault();
        fCorpo = GetFontDefault();
        fCorpoMed = GetFontDefault();
        fPeq = GetFontDefault();
        fNum = GetFontDefault();
    }
}

void descarregarFontesMenu(void)
{
    if (fontesProprias) {
        UnloadFont(fTitulo);
        UnloadFont(fCorpo);
        UnloadFont(fCorpoMed);
        UnloadFont(fPeq);
        UnloadFont(fNum);
        fontesProprias = false;
    }
}

/* Texto centralizado em cx; cy e o centro das letras maiusculas. */
static void textoFonte(Font f, const char *s, float cx, float cy, float tam, float esp, float centro, Color c)
{
    float w = MeasureTextEx(f, s, tam, esp).x;
    if (!fontesProprias) centro = 0.5f;
    DrawTextEx(f, s, (Vector2){roundf(cx - w * 0.5f), roundf(cy - centro * tam)}, tam, esp, c);
}

/* Tamanho da fonte para o texto ocupar exatamente "larg" pixels (espacamento = ep * tamanho). */
static float tamPorLargura(Font f, const char *s, float larg, float ep)
{
    float base = MeasureTextEx(f, s, 100.0f, 100.0f * ep).x;
    return base > 1.0f ? 100.0f * larg / base : 20.0f;
}

static float aleat(int i, int k)
{
    unsigned h = (unsigned)i * 374761393u + (unsigned)k * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (float)(h % 10000u) / 10000.0f;
}

/* Arco de elipse inclinada que some nas pontas (a "orbita" do titulo). */
static void orbita(float cx, float cy, float rx, float ry, float ang, float a0, float a1, Color c)
{
    const int seg = 48;
    float ca = cosf(ang), sa = sinf(ang);
    Vector2 ant = {0.0f, 0.0f};
    for (int i = 0; i <= seg; i++) {
        float a = a0 + (a1 - a0) * (float)i / (float)seg;
        float ex = cosf(a) * rx, ey = sinf(a) * ry;
        Vector2 p = {cx + ex * ca - ey * sa, cy + ex * sa + ey * ca};
        if (i > 0) DrawLineEx(ant, p, 1.8f, Fade(c, 0.8f * sinf(3.14159f * (float)i / (float)seg)));
        ant = p;
    }
}

static void fundoMenu(float t)
{
    DrawRectangleGradientV(0, 0, W, H, (Color){3, 8, 24, 255}, (Color){7, 14, 34, 255});

    /* estrelas pequenas, fixas, piscando devagar */
    for (int i = 0; i < 70; i++) {
        float x = aleat(i, 1) * W, y = aleat(i, 2) * H;
        float pisc = 0.25f + 0.35f * (0.5f + 0.5f * sinf(t * (0.8f + aleat(i, 3)) + (float)i));
        bool amarela = aleat(i, 4) > 0.8f;
        Color c = amarela ? (Color){255, 200, 60, 255} : (Color){200, 215, 255, 255};
        DrawCircle((int)x, (int)y, amarela ? 1.8f : 1.2f, Fade(c, pisc));
    }

    /* quatro pontos laranja maiores, com brilho */
    const Vector2 laranjas[4] = {{163, 244}, {1097, 244}, {163, 521}, {1097, 521}};
    for (int i = 0; i < 4; i++) {
        DrawCircle((int)laranjas[i].x, (int)laranjas[i].y, 8.0f, Fade((Color){255, 170, 40, 255}, 0.10f));
        DrawCircle((int)laranjas[i].x, (int)laranjas[i].y, 3.2f, Fade((Color){255, 170, 40, 255}, 0.9f));
    }

    /* silhuetas de colinas/nuvens */
    Color m1 = (Color){12, 19, 40, 255}, m2 = (Color){16, 25, 50, 255};
    DrawEllipse(88, 600, 130, 175, m1);
    DrawEllipse(195, 650, 75, 90, m2);
    DrawEllipse(1192, 600, 130, 175, m1);
    DrawEllipse(1105, 655, 85, 85, m2);
}

static void botaoMenu(Rectangle rc, const char *t, Color base, bool sel)
{
    float ex = sel ? 4.0f : 0.0f;
    Rectangle r = {rc.x - ex, rc.y - ex * 0.5f, rc.width + 2.0f * ex, rc.height + ex};
    Color contorno = sel ? (Color){196, 210, 218, 255} : ColorBrightness(base, -0.5f);
    Rectangle face = {r.x, r.y, r.width, r.height - 4.0f};

    DrawRectangleRounded((Rectangle){r.x + 1, r.y + 5, r.width, r.height}, 0.5f, 10, Fade(BLACK, 0.45f));
    DrawRectangleRounded((Rectangle){r.x - 2, r.y - 2, r.width + 4, r.height + 4}, 0.5f, 10, contorno);
    DrawRectangleRounded(r, 0.5f, 10, ColorBrightness(base, -0.28f));
    DrawRectangleRounded(face, 0.5f, 10, sel ? ColorBrightness(base, 0.08f) : base);
    DrawRectangleRounded((Rectangle){face.x + 8, face.y + 3, face.width - 16, face.height * 0.36f}, 0.5f, 10,
                         Fade(WHITE, 0.14f));

    float cy = face.y + face.height * 0.5f;
    textoFonte(fCorpo, t, face.x + face.width * 0.5f, cy + 1, 22.0f, 0.9f, CENTRO_CORPO, Fade(BLACK, 0.30f));
    textoFonte(fCorpo, t, face.x + face.width * 0.5f, cy, 22.0f, 0.9f, CENTRO_CORPO, WHITE);
}

static void desenharMenu(void)
{
    float t = (float)GetTime();
    float pulso = 0.5f + 0.5f * sinf(t * 2.0f);
    Color amarelo = (Color){255, 215, 60, 255};

    fundoMenu(t);

    /* titulo: anel de orbita, sombra, brilho pulsante e texto */
    const char *tit = "VOLTERRA";
    float tamT = fontesProprias ? 60.0f / CAP_TITULO : 80.0f;
    float w0 = MeasureTextEx(fTitulo, tit, tamT, 0.0f).x;
    float espT = (487.0f - w0) / 8.0f;
    if (espT < 0.0f) espT = 0.0f;

    orbita(433.0f, 121.0f, 82.0f, 33.0f, -0.26f, 3.14159f, 0.3f * 3.14159f, (Color){110, 140, 190, 255});

    textoFonte(fTitulo, tit, 643, 119, tamT, espT, CENTRO_TITULO, Fade(BLACK, 0.55f));
    const float dx[4] = {-2, 2, 0, 0}, dy[4] = {0, 0, -2, 2};
    for (int i = 0; i < 4; i++)
        textoFonte(fTitulo, tit, 640 + dx[i], 116 + dy[i], tamT, espT, CENTRO_TITULO,
                   Fade((Color){255, 150, 30, 255}, 0.10f + 0.12f * pulso));
    textoFonte(fTitulo, tit, 640, 116, tamT, espT, CENTRO_TITULO, amarelo);

    /* subtitulo, com linhas decorativas nas laterais */
    const char *sub1 = "Gerencie a energia do Brasil";
    const char *sub2 = "Evite blackouts. Evite mais prejuízos. Fortaleça o país ligado.";
    if (!fontesProprias) sub2 = "Evite blackouts. Evite mais prejuizos. Fortaleca o pais ligado.";

    DrawRectangleGradientH(354, 181, 82, 2, Fade((Color){110, 140, 190, 255}, 0.0f), Fade((Color){110, 140, 190, 255}, 0.7f));
    DrawRectangleGradientH(844, 181, 82, 2, Fade((Color){110, 140, 190, 255}, 0.7f), Fade((Color){110, 140, 190, 255}, 0.0f));

    float tam1 = tamPorLargura(fCorpo, sub1, 288.0f, 0.02f);
    float tam2 = tamPorLargura(fCorpoMed, sub2, 479.0f, 0.02f);
    textoFonte(fCorpo, sub1, 640, 180, tam1, tam1 * 0.02f, CENTRO_CORPO, WHITE);
    textoFonte(fCorpoMed, sub2, 640, 209, tam2, tam2 * 0.02f, CENTRO_CORPO, (Color){225, 232, 245, 255});

    /* painel com moldura amarela */
    Rectangle pn = {456, 240, 368, 305};
    for (int i = 3; i >= 1; i--)
        DrawRectangleRounded((Rectangle){pn.x - i * 5.0f, pn.y - i * 5.0f, pn.width + i * 10.0f, pn.height + i * 10.0f},
                             0.17f, 12, Fade(amarelo, 0.025f + 0.01f * pulso));
    DrawRectangleRounded((Rectangle){pn.x - 2, pn.y - 2, pn.width + 4, pn.height + 4}, 0.17f, 12,
                         Fade((Color){250, 200, 50, 255}, 0.75f + 0.2f * pulso));
    DrawRectangleRounded(pn, 0.17f, 12, (Color){15, 22, 41, 255});

    botaoMenu(btnNovo,   "Novo Jogo",     (Color){34, 200, 130, 255}, menuSel == 0);
    botaoMenu(btnFases,  "Escolher Fase", (Color){128, 88, 208, 255}, menuSel == 1);
    botaoMenu(btnRanque, "Ranque",        (Color){245, 176, 20, 255}, menuSel == 2);
    botaoMenu(btnComo,   "Como jogar",    (Color){44, 134, 230, 255}, menuSel == 3);
    botaoMenu(btnSair,   "Sair",          (Color){226, 72, 72, 255},  menuSel == 4);

    if (segredoOk)
        textoFonte(fCorpoMed, "* Conquista secreta desbloqueada *",
                   640, 559, 14.0f, 0.3f, CENTRO_CORPO, amarelo);

    /* icones das fontes de energia em circulos */
    for (int i = 0; i < 5; i++)
        iconeMenu(i, 640.0f + (float)(i - 2) * 103.0f, 613.0f, t, 1.75f,
                  (Color){17, 24, 46, 255}, (Color){52, 66, 98, 255});

    /* rodape */
    char rodape[128];
    snprintf(rodape, sizeof(rodape), "Utilize as setas para navegar%sPressione Enter para selecionar",
             fontesProprias ? "  \xE2\x80\xA2  " : "  |  ");
    float tamR = tamPorLargura(fCorpoMed, rodape, 459.0f, 0.02f);
    if (tamR > 20.0f) tamR = 20.0f;
    textoFonte(fCorpoMed, rodape, 640, 679, tamR, tamR * 0.02f, CENTRO_CORPO, (Color){140, 155, 185, 255});
}

/*  Tela "Escolher Fase"  */

/* Converte coordenadas do Figma (1600x835) para a janela do jogo (1280x720). */
#define FX(x) (640.0f + ((x) - 800.0f) * 0.862f)
#define FY(y) ((y) * 0.862f)

/* Textos de cada missao (edite aqui para mudar nomes e descricoes). */
static const char *missaoNome[FASES] = {
    "Primeira carga",
    "Rede em expansão",
    "Pico de consumo",
    "Tempestade na rede",
    "Rede nacional"
};
static const char *missaoDesc[FASES] = {
    "Reative os primeiros polos de energia.",
    "Conecte uma nova região e mantenha o equilíbrio.",
    "Equilibre as cinco regiões nos horários de pico.",
    "Vários eventos ao mesmo tempo: evite os blackouts.",
    "O desafio final: mantenha o país inteiro ligado."
};

static Vector2 fig(float x, float y)
{
    return (Vector2){FX(x), FY(y)};
}

/* Texto alinhado a esquerda em x; cy e o centro das letras maiusculas. */
static void textoEsq(Font f, const char *s, float x, float cy, float tam, float esp, float centro, Color c)
{
    if (!fontesProprias) centro = 0.5f;
    DrawTextEx(f, s, (Vector2){roundf(x), roundf(cy - centro * tam)}, tam, esp, c);
}

static void setaLinhas(float cx, float cy, float s, bool esquerda, Color c)
{
    float d = esquerda ? -1.0f : 1.0f;
    DrawLineEx((Vector2){cx - d * s, cy}, (Vector2){cx + d * s, cy}, 1.6f, c);
    DrawLineEx((Vector2){cx + d * s, cy}, (Vector2){cx + d * s * 0.2f, cy - s * 0.75f}, 1.6f, c);
    DrawLineEx((Vector2){cx + d * s, cy}, (Vector2){cx + d * s * 0.2f, cy + s * 0.75f}, 1.6f, c);
}

static void teclaRotulo(Rectangle rc)
{
    DrawRectangleRounded((Rectangle){rc.x - 1, rc.y - 1, rc.width + 2, rc.height + 2}, 0.3f, 8, (Color){52, 66, 98, 255});
    DrawRectangleRounded(rc, 0.3f, 8, (Color){20, 28, 50, 255});
}

/* Trilha tracejada ondulada que liga as fases (curva Catmull-Rom). */
static void trilhaFases(float t)
{
    static const Vector2 pts[] = {
        {452, 424}, {500, 440}, {560, 455}, {625, 458}, {690, 444}, {745, 414}, {795, 391},
        {850, 377}, {905, 371}, {945, 376}, {990, 388}, {1040, 415}, {1100, 455}, {1160, 490},
        {1205, 485}, {1245, 462}, {1268, 425}, {1278, 402}
    };
    const int n = (int)(sizeof(pts) / sizeof(pts[0]));
    const float periodo = 13.0f, tracoLen = 7.0f;
    float desloc = fmodf(t * 10.0f, periodo);
    float acum = 0.0f;
    Color cor = Fade((Color){150, 165, 195, 255}, 0.6f);
    Vector2 ant = fig(pts[0].x, pts[0].y);

    for (int i = 0; i < n - 1; i++) {
        int ia = i > 0 ? i - 1 : 0;
        int id = i + 2 < n ? i + 2 : n - 1;
        Vector2 p0 = fig(pts[ia].x, pts[ia].y), p1 = fig(pts[i].x, pts[i].y);
        Vector2 p2 = fig(pts[i + 1].x, pts[i + 1].y), p3 = fig(pts[id].x, pts[id].y);

        for (int s = 1; s <= 12; s++) {
            float u = (float)s / 12.0f, u2 = u * u, u3 = u2 * u;
            Vector2 p;
            p.x = 0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * u + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * u2 +
                          (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * u3);
            p.y = 0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * u + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * u2 +
                          (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * u3);
            if (fmodf(acum + desloc, periodo) < tracoLen) DrawLineEx(ant, p, 2.0f, cor);
            acum += dist(ant, p);
            ant = p;
        }
    }
}

static void noFase(int i, bool sel, float pulso)
{
    static const float descePilula[FASES] = {54.0f, 59.0f, 59.0f, 59.0f, 60.0f};
    Color amarelo = (Color){255, 215, 60, 255};
    Vector2 c = centroFase(i);
    float r = sel ? 41.0f : 35.0f;

    /* corpo do no */
    if (sel) {
        DrawCircle((int)c.x, (int)c.y, r + 16.0f, Fade(amarelo, 0.04f + 0.03f * pulso));
        DrawCircle((int)c.x, (int)c.y, r + 9.0f, Fade(amarelo, 0.06f + 0.03f * pulso));
        DrawRing(c, r + 5.0f, r + 6.5f, 0.0f, 360.0f, 64, Fade(amarelo, 0.8f));
    }
    DrawCircleGradient((int)c.x, (int)c.y, r, (Color){40, 58, 102, 255}, (Color){15, 23, 46, 255});
    DrawRing(c, r - 1.5f, r, 0.0f, 360.0f, 64, sel ? Fade(amarelo, 0.9f) : (Color){72, 92, 142, 255});

    /* ponto satelite no canto superior direito */
    DrawCircle((int)(c.x + (r + 3.0f) * 0.7071f), (int)(c.y - (r + 3.0f) * 0.7071f), 2.4f,
               sel ? (Color){255, 170, 40, 255} : (Color){130, 160, 225, 255});

    /* "FASE" e numero */
    textoFonte(fPeq, "FASE", c.x, c.y - 0.42f * r, 8.5f, 1.4f, CENTRO_CORPO, (Color){150, 165, 200, 255});
    float tamN = sel ? 41.0f : 34.0f;
    textoFonte(fNum, TextFormat("%d", i + 1), c.x, c.y + 0.10f * r, tamN, 0.0f, CENTRO_TITULO,
               sel ? WHITE : (Color){232, 238, 250, 255});

    /* pilula com a quantidade de regioes */
    Vector2 pc = {c.x, c.y + descePilula[i]};
    Rectangle pr = {pc.x - 31.0f, pc.y - 10.0f, 62.0f, 20.0f};
    DrawRectangleRounded((Rectangle){pr.x - 1, pr.y - 1, pr.width + 2, pr.height + 2}, 0.5f, 10,
                         sel ? Fade(amarelo, 0.5f) : (Color){40, 52, 80, 255});
    DrawRectangleRounded(pr, 0.5f, 10, (Color){13, 20, 38, 255});
    textoFonte(fPeq, TextFormat("%d regiões", regioesNaFase(i + 1)), pc.x, pc.y, 9.0f, 0.5f, CENTRO_CORPO,
               sel ? amarelo : (Color){150, 165, 195, 255});
}

static void desenharFases(void)
{
    float t = (float)GetTime();
    float pulso = 0.5f + 0.5f * sinf(t * 2.0f);
    Color amarelo = (Color){255, 215, 60, 255};
    Color linha = (Color){110, 140, 190, 255};

    fundoMenu(t);

    /* aneis de orbita bem discretos */
    DrawCircleLines((int)FX(375), (int)FY(345), 256.0f * 0.862f, Fade((Color){70, 95, 140, 255}, 0.28f));
    DrawCircleLines((int)FX(1330), (int)FY(583), 217.0f * 0.862f, Fade((Color){70, 95, 140, 255}, 0.22f));
    Vector2 pt = fig(461, 104);
    DrawCircle((int)pt.x, (int)pt.y, 8.0f, Fade((Color){255, 170, 40, 255}, 0.10f));
    DrawCircle((int)pt.x, (int)pt.y, 3.2f, Fade((Color){255, 170, 40, 255}, 0.9f));

    /* botao MENU (canto superior esquerdo) */
    Rectangle bm = btnVoltarFas;
    bool hm = hover(bm);
    float mcy = bm.y + bm.height * 0.5f;
    DrawRectangleRounded((Rectangle){bm.x - 1, bm.y - 1, bm.width + 2, bm.height + 2}, 0.5f, 12,
                         hm ? Fade(amarelo, 0.7f) : (Color){48, 62, 92, 255});
    DrawRectangleRounded(bm, 0.5f, 12, hm ? (Color){24, 34, 60, 255} : (Color){16, 24, 44, 255});
    DrawCircle((int)(bm.x + 19.0f), (int)mcy, 11.0f, (Color){255, 200, 40, 255});
    setaLinhas(bm.x + 19.0f, mcy, 4.5f, true, (Color){30, 24, 8, 255});
    textoEsq(fCorpo, "MENU", bm.x + 38.0f, mcy, 11.5f, 1.6f, CENTRO_CORPO, WHITE);

    /* "CENTRAL DE OPERACOES" com linhas laterais */
    DrawRectangle((int)FX(678), (int)FY(169), (int)(26.0f * 0.862f), 1, Fade(linha, 0.7f));
    DrawRectangle((int)FX(896), (int)FY(169), (int)(26.0f * 0.862f), 1, Fade(linha, 0.7f));
    const char *olho = "CENTRAL DE OPERAÇÕES";
    float tamO = tamPorLargura(fPeq, olho, 144.0f, 0.2f);
    textoFonte(fPeq, olho, 640, FY(169), tamO, tamO * 0.2f, CENTRO_CORPO, (Color){105, 125, 165, 255});

    /* titulo */
    const char *tit = "ESCOLHER FASE";
    float tamT = fontesProprias ? 46.0f / CAP_TITULO : 64.0f;
    float w0 = MeasureTextEx(fTitulo, tit, tamT, 0.0f).x;
    float espT = (636.0f - w0) / 13.0f;
    if (espT < 0.0f) espT = 0.0f;
    float ty = FY(220);
    textoFonte(fTitulo, tit, 643, ty + 3, tamT, espT, CENTRO_TITULO, Fade(BLACK, 0.55f));
    const float dx[4] = {-2, 2, 0, 0}, dy[4] = {0, 0, -2, 2};
    for (int i = 0; i < 4; i++)
        textoFonte(fTitulo, tit, 640 + dx[i], ty + dy[i], tamT, espT, CENTRO_TITULO,
                   Fade((Color){255, 150, 30, 255}, 0.10f + 0.12f * pulso));
    textoFonte(fTitulo, tit, 640, ty, tamT, espT, CENTRO_TITULO, amarelo);

    /* subtitulo */
    const char *sub = "Escolha um ponto da rede para iniciar sua próxima missão.";
    float tamS = tamPorLargura(fCorpoMed, sub, 420.0f, 0.02f);
    textoFonte(fCorpoMed, sub, 640, FY(285), tamS, tamS * 0.02f, CENTRO_CORPO, (Color){190, 200, 222, 255});

    /* trilha e nos das fases (o selecionado e desenhado por ultimo) */
    trilhaFases(t);
    for (int i = 0; i < FASES; i++) {
        if (i != faseSel) noFase(i, false, pulso);
    }
    noFase(faseSel, true, pulso);

    /* cartao da missao */
    Rectangle cartao = {FX(458), FY(557), 685.0f * 0.862f, 100.0f * 0.862f};
    DrawRectangleRounded((Rectangle){cartao.x, cartao.y + 5, cartao.width, cartao.height}, 0.4f, 12, Fade(BLACK, 0.35f));
    DrawRectangleRounded((Rectangle){cartao.x - 1.5f, cartao.y - 1.5f, cartao.width + 3, cartao.height + 3}, 0.4f, 12,
                         (Color){48, 62, 94, 255});
    DrawRectangleRounded(cartao, 0.4f, 12, (Color){17, 25, 46, 255});

    textoFonte(fPeq, "MISSÃO", FX(516), FY(591), 8.5f, 1.4f, CENTRO_CORPO, (Color){125, 140, 175, 255});
    textoFonte(fNum, TextFormat("%02d", faseSel + 1), FX(516), FY(615), 26.0f, 0.0f, CENTRO_TITULO, amarelo);
    DrawRectangle((int)FX(556), (int)FY(575), 1, (int)(65.0f * 0.862f), (Color){45, 58, 88, 255});

    textoEsq(fPeq, TextFormat("%d REGIÕES CONECTADAS", regioesNaFase(faseSel + 1)), FX(575), FY(580), 8.5f, 1.3f,
             CENTRO_CORPO, (Color){120, 135, 170, 255});
    textoEsq(fCorpo, missaoNome[faseSel], FX(575), FY(604), 17.0f, 0.3f, CENTRO_CORPO, WHITE);
    textoEsq(fCorpoMed, missaoDesc[faseSel], FX(575), FY(630), 11.5f, 0.2f, CENTRO_CORPO, (Color){150, 165, 195, 255});

    /* botao "Iniciar fase" */
    Rectangle bi = btnIniciarFase;
    bool hi = hover(bi);
    Color ouro = hi ? (Color){255, 196, 50, 255} : (Color){247, 181, 36, 255};
    Rectangle face = {bi.x, bi.y, bi.width, bi.height - 4.0f};
    DrawRectangleRounded((Rectangle){bi.x + 1, bi.y + 5, bi.width, bi.height}, 0.5f, 12, Fade(BLACK, 0.4f));
    DrawRectangleRounded(bi, 0.5f, 12, ColorBrightness(ouro, -0.30f));
    DrawRectangleRounded(face, 0.5f, 12, ouro);
    DrawRectangleRounded((Rectangle){face.x + 8, face.y + 3, face.width - 16, face.height * 0.38f}, 0.5f, 10,
                         Fade(WHITE, 0.22f));
    textoEsq(fCorpo, "Iniciar fase", face.x + 16.0f, face.y + face.height * 0.5f, 13.5f, 0.3f, CENTRO_CORPO,
             (Color){38, 28, 8, 255});
    DrawCircle((int)(face.x + face.width - 21.0f), (int)(face.y + face.height * 0.5f), 11.0f, (Color){22, 27, 50, 255});
    setaLinhas(face.x + face.width - 21.0f, face.y + face.height * 0.5f, 4.5f, false, WHITE);

    /* dicas de teclado */
    float hy = FY(687);
    Color dica = (Color){120, 135, 170, 255};
    Rectangle k1 = {FX(702), hy - 9.5f, 39.0f * 0.862f, 19.0f};
    teclaRotulo(k1);
    setaLinhas(k1.x + 9.5f, hy, 3.6f, true, (Color){190, 200, 225, 255});
    setaLinhas(k1.x + 23.5f, hy, 3.6f, false, (Color){190, 200, 225, 255});
    textoEsq(fCorpoMed, "Navegar", FX(750), hy, 10.0f, 0.2f, CENTRO_CORPO, dica);
    DrawCircle((int)FX(805), (int)hy, 1.6f, dica);
    Rectangle k2 = {FX(815), hy - 9.5f, 45.0f * 0.862f, 19.0f};
    teclaRotulo(k2);
    textoFonte(fPeq, "Enter", k2.x + k2.width * 0.5f, hy, 9.0f, 0.3f, CENTRO_CORPO, (Color){210, 218, 235, 255});
    textoEsq(fCorpoMed, "Iniciar", FX(868), hy, 10.0f, 0.2f, CENTRO_CORPO, dica);
}


#define AJ_AMARELO ((Color){255, 214, 10, 255})
#define AJ_VERDE   ((Color){34, 205, 130, 255})
#define AJ_VERM    ((Color){235, 75, 75, 255})
#define AJ_LARANJA ((Color){255, 150, 30, 255})
#define AJ_TEXTO   ((Color){235, 240, 250, 255})
#define AJ_SUAVE   ((Color){140, 155, 185, 255})
#define AJ_ESCURO  ((Color){22, 27, 50, 255})

static bool sobre(Rectangle r)
{
    return CheckCollisionPointRec(GetMousePosition(), r);
}

/* Texto alinhado a direita em xDir. */
static void textoDir(Font f, const char *s, float xDir, float cy, float tam, float esp, float centro, Color c)
{
    float w = MeasureTextEx(f, s, tam, esp).x;
    textoEsq(f, s, xDir - w, cy, tam, esp, centro, c);
}

/* Indicador de passos: 01 --- 02 --- 03 */
static void ajudaPassos(int pag)
{
    const char *rot[3] = {"EQUILÍBRIO", "MAPA", "BÔNUS"};
    const float xs[3] = {440.0f, 640.0f, 840.0f};
    const float y = 240.0f;

    for (int i = 0; i < 2; i++) {
        Color c = (i < pag) ? AJ_VERDE : Fade(AJ_SUAVE, 0.35f);
        DrawLineEx((Vector2){xs[i] + 20.0f, y}, (Vector2){xs[i + 1] - 20.0f, y}, 2.0f, c);
    }

    for (int i = 0; i < 3; i++) {
        int x = (int)xs[i];
        if (i == pag) {
            DrawCircle(x, (int)y, 22.0f, Fade(AJ_AMARELO, 0.18f));
            DrawCircle(x, (int)y, 15.0f, AJ_AMARELO);
            textoFonte(fCorpo, TextFormat("%02d", i + 1), xs[i], y, 12.0f, 0.5f, CENTRO_CORPO, AJ_ESCURO);
        } else if (i < pag) {
            DrawCircle(x, (int)y, 15.0f, AJ_VERDE);
            DrawLineEx((Vector2){xs[i] - 6.0f, y}, (Vector2){xs[i] - 2.0f, y + 4.0f}, 2.5f, AJ_ESCURO);
            DrawLineEx((Vector2){xs[i] - 2.0f, y + 4.0f}, (Vector2){xs[i] + 6.0f, y - 4.0f}, 2.5f, AJ_ESCURO);
        } else {
            DrawCircle(x, (int)y, 15.0f, (Color){14, 22, 46, 255});
            DrawCircleLines(x, (int)y, 15.0f, Fade(AJ_SUAVE, 0.5f));
            textoFonte(fCorpo, TextFormat("%02d", i + 1), xs[i], y, 12.0f, 0.5f, CENTRO_CORPO, AJ_SUAVE);
        }
        textoFonte(fPeq, rot[i], xs[i], y + 34.0f, 9.5f, 1.2f, CENTRO_CORPO, i == pag ? AJ_AMARELO : AJ_SUAVE);
    }
}

/* Passo 1: barra de carga com faixa vermelha / verde / laranja. */
static void ajudaDemoCarga(float t)
{
    const float x0 = 640.0f, w = 430.0f;
    float v = 55.0f + 50.0f * sinf(t * 0.9f);              /* anima entre 5% e 105% */
    Color cv = v < 30.0f ? AJ_VERM : (v > 80.0f ? AJ_LARANJA : AJ_VERDE);
    const char *estado = v < 30.0f ? "Risco de blackout"
                       : (v > 80.0f ? "Risco de sobrecarga" : "Rede estável");

    textoEsq(fPeq, "CARGA DA REGIÃO", x0, 345.0f, 11.0f, 1.5f, CENTRO_CORPO, AJ_SUAVE);
    textoDir(fNum, TextFormat("%d%%", (int)v), x0 + w, 345.0f, 26.0f, 1.0f, CENTRO_TITULO, cv);

    float segR = w * 30.0f / 110.0f;                       /* 0..30%   */
    float segV = w * 50.0f / 110.0f;                       /* 30..80%  */
    DrawRectangle((int)x0, 372, (int)segR, 8, AJ_VERM);
    DrawRectangle((int)(x0 + segR), 372, (int)segV, 8, AJ_VERDE);
    DrawRectangle((int)(x0 + segR + segV), 372, (int)(w - segR - segV) + 1, 8, AJ_LARANJA);

    float px = x0 + w * v / 110.0f;
    DrawCircle((int)px, 376, 9.0f, WHITE);
    DrawCircle((int)px, 376, 5.0f, cv);

    textoFonte(fPeq, "0%",   x0,                 396.0f, 10.0f, 0.5f, CENTRO_CORPO, AJ_SUAVE);
    textoFonte(fPeq, "30%",  x0 + segR,          396.0f, 10.0f, 0.5f, CENTRO_CORPO, AJ_SUAVE);
    textoFonte(fPeq, "80%",  x0 + segR + segV,   396.0f, 10.0f, 0.5f, CENTRO_CORPO, AJ_SUAVE);
    textoFonte(fPeq, "110%", x0 + w,             396.0f, 10.0f, 0.5f, CENTRO_CORPO, AJ_SUAVE);

    DrawCircle((int)x0 + 4, 440, 4.0f, cv);
    textoEsq(fCorpo, estado, x0 + 16.0f, 440.0f, 14.0f, 0.4f, CENTRO_CORPO, cv);
}

/* Passo 2: cronometro circular + lista de regioes com sobra/falta. */
static void ajudaDemoMapa(float t)
{
    const Vector2 c = {700.0f, 440.0f};
    int seg = 8 - ((int)t) % 9;                            /* 8..0 */
    float frac = (float)seg / 8.0f;

    DrawRing(c, 34.0f, 40.0f, 0.0f, 360.0f, 48, Fade(AJ_AMARELO, 0.18f));
    if (seg > 0) DrawRing(c, 34.0f, 40.0f, -90.0f, -90.0f + 360.0f * frac, 48, AJ_AMARELO);
    textoFonte(fNum, TextFormat("%02d", seg), c.x, c.y - 3.0f, 28.0f, 1.0f, CENTRO_TITULO, AJ_TEXTO);
    textoFonte(fPeq, "seg", c.x, c.y + 18.0f, 9.0f, 0.5f, CENTRO_CORPO, AJ_SUAVE);

    const char *nome[3] = {"Norte", "Centro", "Sudeste"};
    const char *val[3]  = {"+10%", "58%", "-12%"};
    Color cor[3] = {AJ_VERDE, AJ_AMARELO, AJ_VERM};
    for (int i = 0; i < 3; i++) {
        float y = 395.0f + 45.0f * (float)i;
        DrawCircle(800, (int)y, 4.0f, cor[i]);
        textoEsq(fCorpoMed, nome[i], 816.0f, y, 15.0f, 0.3f, CENTRO_CORPO, AJ_TEXTO);
        textoDir(fCorpo, val[i], 1070.0f, y, 15.0f, 0.3f, CENTRO_CORPO, cor[i]);
    }
}

/* Passo 3: multiplicador x1..x5 e pontuacao. */
static void ajudaDemoBonus(float t)
{
    int n = 1 + ((int)(t * 1.2f)) % 5;

    textoFonte(fPeq, "MULTIPLICADOR ATUAL", 855.0f, 350.0f, 11.0f, 1.5f, CENTRO_CORPO, AJ_SUAVE);
    textoFonte(fNum, TextFormat("x%d", n), 855.0f, 420.0f, 56.0f, 1.0f, CENTRO_TITULO, AJ_AMARELO);
    for (int i = 0; i < 5; i++)
        DrawCircle(855 + (i - 2) * 26, 480, 8.0f, i < n ? AJ_VERDE : Fade(AJ_SUAVE, 0.35f));
    textoDir(fCorpo, TextFormat("%d.000", n * 5), 1070.0f, 530.0f, 16.0f, 0.5f, CENTRO_CORPO, AJ_TEXTO);
}

static void desenharAjuda(void)
{
    float t = (float)GetTime();
    int pag = paginaAjuda;
    fundoMenu(t);

    /* voltar ao menu (canto superior esquerdo) */
    Rectangle bm = btnAjudaMenu;
    float cm = bm.y + bm.height * 0.5f;
    DrawCircle((int)(bm.x + 12.0f), (int)cm, 12.0f, sobre(bm) ? WHITE : AJ_AMARELO);
    setaLinhas(bm.x + 12.0f, cm, 4.5f, true, AJ_ESCURO);
    textoEsq(fCorpo, "MENU", bm.x + 32.0f, cm, 13.0f, 1.5f, CENTRO_CORPO, AJ_TEXTO);

    /* titulo */
    float tam = fontesProprias ? 46.0f / CAP_TITULO : 56.0f;
    textoFonte(fPeq, "TREINAMENTO DE OPERADOR", 640.0f, 60.0f, 11.0f, 2.5f, CENTRO_CORPO, AJ_SUAVE);
    textoFonte(fTitulo, "COMO JOGAR", 643.0f, 119.0f, tam, 3.0f, CENTRO_TITULO, Fade(BLACK, 0.6f));
    textoFonte(fTitulo, "COMO JOGAR", 640.0f, 116.0f, tam, 3.0f, CENTRO_TITULO, AJ_AMARELO);
    textoFonte(fCorpoMed, "Domine a rede em três passos rápidos", 640.0f, 168.0f, 15.0f, 0.3f,
               CENTRO_CORPO, AJ_TEXTO);

    ajudaPassos(pag);

    /* card */
    static const char *num[3]   = {"PASSO 01", "PASSO 02", "PASSO 03"};
    static const char *tit1[3]  = {"Mantenha a rede", "Leia o mapa", "Construa seu"};
    static const char *tit2[3]  = {"estável", "e o tempo", "multiplicador"};
    static const char *desc1[3] = {"Cada região possui uma bateria. Mantenha a",
                                   "Cores mostram onde sobra ou falta energia.",
                                   "Atravesse as cinco fases sem alertas para"};
    static const char *desc2[3] = {"carga na faixa segura para evitar alertas.",
                                   "O cronômetro indica quando a demanda vai mudar.",
                                   "aumentar seu bônus de pontos."};
    static const char *dica1[3] = {"Faixa ideal", "Antecipe-se", "Sequência perfeita"};
    static const char *dica2[3] = {"Entre 30% e 80% você mantém o país operando.",
                                   "Redistribua energia antes que o tempo termine.",
                                   "Cada fase limpa aumenta o bônus de x1 até x5."};

    caixa((Rectangle){170, 300, 940, 270}, 0.04f, (Color){10, 18, 40, 235}, (Color){30, 45, 85, 255});

    textoEsq(fPeq, num[pag], 210.0f, 335.0f, 11.0f, 2.0f, CENTRO_CORPO, AJ_AMARELO);
    textoEsq(fCorpo, tit1[pag], 210.0f, 372.0f, 30.0f, 0.5f, CENTRO_CORPO, AJ_TEXTO);
    textoEsq(fCorpo, tit2[pag], 210.0f, 408.0f, 30.0f, 0.5f, CENTRO_CORPO, AJ_TEXTO);
    textoEsq(fCorpoMed, desc1[pag], 210.0f, 452.0f, 15.0f, 0.3f, CENTRO_CORPO, AJ_SUAVE);
    textoEsq(fCorpoMed, desc2[pag], 210.0f, 476.0f, 15.0f, 0.3f, CENTRO_CORPO, AJ_SUAVE);
    DrawRectangle(210, 502, 3, 44, AJ_AMARELO);
    textoEsq(fCorpo, dica1[pag], 224.0f, 514.0f, 13.0f, 0.4f, CENTRO_CORPO, AJ_TEXTO);
    textoEsq(fCorpoMed, dica2[pag], 224.0f, 534.0f, 12.0f, 0.3f, CENTRO_CORPO, AJ_SUAVE);

    if (pag == 0) ajudaDemoCarga(t);
    else if (pag == 1) ajudaDemoMapa(t);
    else ajudaDemoBonus(t);

    /* navegacao */
    if (pag > 0) {
        Rectangle ba = btnAjudaAnt;
        float ca = ba.y + ba.height * 0.5f;
        Color c = sobre(ba) ? WHITE : AJ_SUAVE;
        setaLinhas(ba.x + 8.0f, ca, 4.5f, true, c);
        textoEsq(fCorpoMed, "Anterior", ba.x + 24.0f, ca, 14.0f, 0.3f, CENTRO_CORPO, c);
    }
    textoFonte(fPeq, TextFormat("%d / 3", pag + 1), 640.0f, 617.0f, 11.0f, 1.5f, CENTRO_CORPO, AJ_SUAVE);

    Rectangle bp = btnAjudaProx;
    float cp = bp.y + bp.height * 0.5f;
    DrawRectangleRounded(bp, 1.0f, 16, sobre(bp) ? (Color){255, 230, 90, 255} : AJ_AMARELO);
    textoEsq(fCorpo, pag < 2 ? "Próximo" : "Concluir", bp.x + 26.0f, cp, 15.0f, 0.5f, CENTRO_CORPO, AJ_ESCURO);
    DrawCircle((int)(bp.x + bp.width - 26.0f), (int)cp, 14.0f, AJ_ESCURO);
    setaLinhas(bp.x + bp.width - 26.0f, cp, 4.5f, false, AJ_AMARELO);
}

static void desenharTroca(void)
{
    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.72f));

    if (secreta) {
        caixa((Rectangle){340, 90, 600, 510}, 0.04f,
              (Color){20, 20, 44, 255}, (Color){255, 215, 60, 255});

        textoCentro(TextFormat("FASE %d CONCLUIDA!", FASES),
                    640, 110, 28, (Color){110, 240, 150, 255});
        textoCentro("FASE SECRETA", 640, 150, 48, (Color){255, 215, 60, 255});
        textoCentro("Voce nao perdeu nenhuma vida!", 640, 210, 22, WHITE);

        DrawText("- Cada regiao agora se divide em seus 27 estados",
                 380, 260, 19, RAYWHITE);
        DrawText("- Gestao mais leve: rede mais lenta, eventos mais fracos",
                 380, 292, 19, RAYWHITE);
        DrawText("- 3 dias de 40 segundos e 5 vidas",
                 380, 324, 19, RAYWHITE);
        DrawText("- Clique num estado e ajuste a producao",
                 380, 356, 19, RAYWHITE);
        DrawText("- Arraste de um estado a outro para enviar energia",
                 380, 388, 19, RAYWHITE);
        DrawText("- Vencer rende +5000 pontos e a conquista secreta",
                 380, 420, 19, (Color){255, 215, 60, 255});

        textoCentro("O tempo so comeca quando voce clicar em Comecar.",
                    640, 510, 17, (Color){150, 190, 230, 255});
        desenharBotao(btnOkTroca, "Comecar", (Color){40, 140, 90, 255});
        return;
    }

    caixa((Rectangle){340, 75, 600, 540}, 0.04f,
          (Color){14, 26, 48, 255}, (Color){255, 215, 60, 255});

    int y = 95;

    if (!introJogo) {
        textoCentro(TextFormat("FASE %d CONCLUIDA!", fase),
                    640, y, 30, (Color){110, 240, 150, 255});
        textoCentro(TextFormat("Bonus: +%d pontos", bonusFase),
                    640, y + 36, 20, YELLOW);
        y += 75;
    } else {
        textoCentro("PREPARE-SE", 640, y, 30, (Color){110, 240, 150, 255});
        y += 45;
    }

    textoCentro(TextFormat("FASE %d DE %d", fase, FASES), 640, y, 42, WHITE);
    y += 55;

    /*
     * As instruções são exibidas somente no início do jogo,
     * não na tela de conclusão de fase.
     */
    if (introJogo) {
        caixa((Rectangle){360, (float)y, 560, 130}, 0.05f,
              (Color){24, 38, 68, 255}, (Color){70, 130, 200, 255});

        DrawText("COMO FUNCIONA O SALDO E RESET:",
                 375, y + 10, 18, (Color){255, 215, 60, 255});
        DrawText("- Saldo positivo (amarelo) CARREGA a bateria.",
                 375, y + 35, 15, RAYWHITE);
        DrawText("- Saldo negativo (vermelho) DESCARREGA a bateria.",
                 375, y + 55, 15, RAYWHITE);
        DrawText("- Use W/S, roda do mouse ou botoes +/- para equilibrar.",
                 375, y + 75, 15, RAYWHITE);
        DrawText("- Pressione R ou aperte RESET para voltar a 100%.",
                 375, y + 95, 15, (Color){110, 240, 150, 255});
        y += 145;
    }

    int qtd = regioesNaFase(fase);

    DrawText(TextFormat("- Regioes na rede: %d", qtd),
             380, y, 17, RAYWHITE);
    y += 24;

    if (fase > 1 && !introJogo &&
        qtd > regioesNaFase(fase - 1)) {
        DrawText(TextFormat("- Nova regiao liberada: %s",
                            r[ordemFase[qtd - 1]].nome),
                 380, y, 17, (Color){255, 215, 60, 255});
        y += 24;
    }

    DrawText(TextFormat("- Eventos simultaneos: ate %d", maxEventos()),
             380, y, 17, RAYWHITE);
    y += 24;

    DrawText(TextFormat("- Velocidade da rede: %d%%",
                        (int)(velFase() * 100)),
             380, y, 17, RAYWHITE);
    y += 24;

    DrawText("- Mantenha as baterias na faixa segura (30%% a 80%%)",
             380, y, 17, RAYWHITE);

    if (!introJogo && fase < FASES) {
        textoCentro("Escolha como deseja continuar:",
                    640, 525, 16, (Color){150, 190, 230, 255});

        desenharBotao(btnProsseguirFase,
                      "Proxima fase",
                      (Color){40, 140, 90, 255});

        desenharBotao(btnEncerrarFase,
                      "Encerrar e registrar",
                      (Color){160, 55, 55, 255});
    } else {
        textoCentro("O tempo so comeca quando voce clicar em Comecar.",
                    640, 525, 16, (Color){150, 190, 230, 255});

        desenharBotao(btnOkTroca,
                      "Comecar",
                      (Color){40, 140, 90, 255});
    }
}

static void desenharMini(void)
{
    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.75f));
    Color cor = mini.blackout ? (Color){255, 80, 80, 255} : (Color){255, 160, 50, 255};
    caixa(painelMini, 0.04f, (Color){16, 26, 46, 255}, cor);
    textoCentro(TextFormat("%s em %s", mini.blackout ? "BLACKOUT" : "SOBRECARGA", nomeAlvo(mini.reg)), 640, 158, 30, cor);

    const char *ins = "";
    switch (mini.tipo) {
        case MINI_FIOS:      ins = "Raquete: mouse ou A/D/setas. Acerte os 4 terminais; 3 quedas = falha."; break;
        case MINI_DISJUNTOR: ins = "Clique nos disjuntores em ordem: 1, 2, 3, 4, 5, 6. Erro reinicia a sequencia."; break;
        case MINI_GERADOR:   ins = "Coletor: mouse ou WASD/setas. Capture 8 celulas de energia."; break;
        case MINI_TURBINAS:  ins = "Clique nas turbinas para esfria-las. Nao deixe nenhuma chegar a 100%!"; break;
        case MINI_SINCRONIA: ins = "Pare o marcador na zona verde (ESPACO ou botao). Acerte 3 vezes."; break;
        case MINI_ALVOS:     ins = "Disco: mouse ou WASD/setas. Aterre 4 nucleos, acertando 2 vezes cada."; break;
        case MINI_FAISCAS:   ins = "Mouse ou WASD/setas: desvie das faiscas ate o tempo acabar. 3 toques = falha."; break;
        default:             ins = "Desligue o disjuntor ACESO antes que ele apague. Faca 8 acertos."; break;
    }
    textoCentro(ins, 640, 202, 18, LIGHTGRAY);

    float f = lim(mini.tempo / mini.tempoMax, 0.0f, 1.0f);
    DrawRectangleRounded((Rectangle){330, 545, 620, 14}, 0.5f, 6, (Color){12, 20, 34, 255});
    DrawRectangleRounded((Rectangle){330, 545, fmaxf(6.0f, 620 * f), 14}, 0.5f, 6,
                         f < 0.25f ? (Color){255, 80, 80, 255} : (Color){70, 225, 140, 255});
    textoCentro(TextFormat("%.1fs", fmaxf(mini.tempo, 0.0f)), 640, 564, 18, WHITE);

    Vector2 m = mouseVirtual();
    switch (mini.tipo) {
        case MINI_FIOS: {
            Rectangle a = arenaFaiscas();
            caixa(a, 0.03f, (Color){10, 16, 30, 255}, mini.flash > 0 ? (Color){255, 80, 80, 255} : (Color){90, 120, 170, 255});
            for (int i = 0; i < 4; i++) {
                Vector2 alvo = {420.0f + 140.0f * i, a.y + 58.0f};
                DrawCircleV(alvo, 28.0f, mini.on[i] ? (Color){60, 210, 130, 255} : (Color){40, 55, 80, 255});
                DrawCircleLines((int)alvo.x, (int)alvo.y, 28.0f, mini.on[i] ? WHITE : (Color){100, 150, 200, 255});
                textoCentro(mini.on[i] ? "OK" : TextFormat("NO %d", i + 1), (int)alvo.x, (int)alvo.y - 10, 16, WHITE);
            }
            DrawCircleV(mini.fPos[0], 15.0f, Fade((Color){70, 210, 255, 255}, 0.3f));
            DrawCircleV(mini.fPos[0], 10.0f, (Color){100, 230, 255, 255});
            Rectangle raquete = {mini.escudo.x - 58.0f, a.y + a.height - 36.0f, 116.0f, 14.0f};
            DrawRectangleRounded(raquete, 0.5f, 8, (Color){80, 200, 255, 255});
            DrawRectangleLinesEx(raquete, 1.5f, WHITE);
            textoCentro(TextFormat("Terminais: %d/4", mini.acertos), 640, 492, 18, WHITE);
            for (int i = 0; i < MAX_GOLPES; i++)
                coracao(a.x + 22 + i * 30, a.y + a.height - 18, 8,
                        i < MAX_GOLPES - mini.golpes ? (Color){240, 60, 80, 255} : (Color){50, 55, 70, 255});
            break;
        }
        case MINI_DISJUNTOR:
            for (int s = 0; s < 6; s++) {
                Rectangle rc = discRect(s);
                Color bg = mini.on[s] ? (Color){40, 170, 100, 255} : (Color){36, 50, 78, 255};
                Color bd = mini.flash > 0 ? (Color){255, 80, 80, 255} : (hover(rc) ? WHITE : (Color){90, 120, 170, 255});
                caixa(rc, 0.2f, bg, bd);
                textoCentro(TextFormat("%d", mini.slot[s]), (int)(rc.x + rc.width / 2), (int)rc.y + 12, 40, WHITE);
                textoCentro(mini.on[s] ? "LIGADO" : "desligado", (int)(rc.x + rc.width / 2), (int)rc.y + 58, 14,
                            mini.on[s] ? WHITE : LIGHTGRAY);
            }
            if (mini.proximo <= 6) textoCentro(TextFormat("Proximo: %d", mini.proximo), 640, 505, 20, YELLOW);
            break;
        case MINI_GERADOR: {
            Rectangle a = arenaFaiscas();
            caixa(a, 0.03f, (Color){10, 16, 30, 255}, (Color){90, 120, 170, 255});
            for (int i = 0; i < mini.nFaiscas; i++) {
                DrawCircleV(mini.fPos[i], RAIO_FAISCA + 5.0f, Fade((Color){80, 220, 255, 255}, 0.25f));
                DrawCircleV(mini.fPos[i], RAIO_FAISCA, (Color){90, 220, 255, 255});
                DrawCircleV(mini.fPos[i], 4.0f, WHITE);
            }
            DrawCircleV(mini.escudo, RAIO_ESCUDO + 5.0f, Fade((Color){110, 240, 150, 255}, 0.28f));
            DrawCircleV(mini.escudo, RAIO_ESCUDO, (Color){80, 220, 140, 255});
            DrawCircleLines((int)mini.escudo.x, (int)mini.escudo.y, RAIO_ESCUDO, WHITE);
            textoCentro(TextFormat("Celulas capturadas: %d/8", mini.acertos), 640, 510, 20, WHITE);
            break;
        }
        case MINI_TURBINAS:
            for (int i = 0; i < 4; i++) {
                Rectangle rc = turbRect(i);
                float h = rc.height * mini.calor[i] / 100.0f;
                Color cc = mini.calor[i] < 50 ? (Color){70, 225, 140, 255}
                         : (mini.calor[i] < 80 ? (Color){255, 200, 60, 255} : (Color){255, 70, 70, 255});
                caixa(rc, 0.1f, (Color){12, 20, 34, 255}, mini.calor[i] > 80 ? (Color){255, 70, 70, 255} : (Color){90, 120, 170, 255});
                DrawRectangle((int)rc.x + 6, (int)(rc.y + rc.height - h), (int)rc.width - 12, (int)h, cc);
                textoCentro(TextFormat("%.0f%%", mini.calor[i]), (int)(rc.x + rc.width / 2), (int)rc.y + 100, 24, WHITE);
                textoCentro(TextFormat("Turbina %d", i + 1), (int)(rc.x + rc.width / 2), (int)(rc.y + rc.height) + 8, 16, LIGHTGRAY);
            }
            break;
        case MINI_SINCRONIA:
            textoCentro(TextFormat("Acertos: %d/3", mini.acertos), 640, 300, 26, WHITE);
            caixa(barraSinc, 0.2f, (Color){12, 20, 34, 255}, mini.flash > 0 ? (Color){255, 80, 80, 255} : (Color){90, 120, 170, 255});
            DrawRectangle((int)(barraSinc.x + (mini.zona - mini.larg / 2) * barraSinc.width), (int)barraSinc.y + 4,
                          (int)(mini.larg * barraSinc.width), (int)barraSinc.height - 8, (Color){70, 225, 140, 255});
            DrawRectangle((int)(barraSinc.x + mini.marcador * barraSinc.width) - 4, (int)barraSinc.y - 8, 8,
                          (int)barraSinc.height + 16, WHITE);
            desenharBotao(btnSinc, "PARAR", (Color){40, 100, 180, 255});
            break;
        case MINI_ALVOS: {
            Rectangle a = arenaFaiscas();
            caixa(a, 0.03f, (Color){10, 16, 30, 255}, mini.flash > 0 ? (Color){255, 80, 80, 255} : (Color){90, 120, 170, 255});
            for (int i = 0; i < mini.nFaiscas; i++) {
                if (mini.on[i]) {
                    DrawCircleV(mini.fPos[i], RAIO_FAISCA + 5.0f, Fade((Color){80, 220, 150, 255}, 0.25f));
                    DrawCircleV(mini.fPos[i], RAIO_FAISCA, (Color){65, 200, 125, 255});
                } else {
                    Color calor = mini.slot[i] > 0 ? (Color){255, 230, 120, 255} : (Color){255, 115, 55, 255};
                    DrawCircleV(mini.fPos[i], RAIO_FAISCA + 7.0f, Fade(calor, 0.26f));
                    DrawCircleV(mini.fPos[i], RAIO_FAISCA, calor);
                    DrawCircleV(mini.fPos[i], 4.0f, WHITE);
                    textoCentro(TextFormat("%d/2", mini.slot[i]), (int)mini.fPos[i].x, (int)mini.fPos[i].y + 15, 13, WHITE);
                }
            }
            DrawCircleV(mini.escudo, RAIO_ESCUDO + 5.0f, Fade((Color){80, 200, 255, 255}, 0.3f));
            DrawCircleV(mini.escudo, RAIO_ESCUDO, (Color){80, 200, 255, 255});
            DrawCircleLines((int)mini.escudo.x, (int)mini.escudo.y, RAIO_ESCUDO, WHITE);
            textoCentro(TextFormat("Nucleos aterrados: %d/4", mini.acertos), 640, 510, 20, WHITE);
            break;
        }
        case MINI_FAISCAS: {
            Rectangle a = arenaFaiscas();
            caixa(a, 0.03f, (Color){10, 16, 30, 255}, mini.flash > 0 ? (Color){255, 80, 80, 255} : (Color){90, 120, 170, 255});
            for (int i = 0; i < mini.nFaiscas; i++) {
                DrawCircleV(mini.fPos[i], RAIO_FAISCA + 5.0f, Fade((Color){255, 170, 40, 255}, 0.25f));
                DrawCircleV(mini.fPos[i], RAIO_FAISCA, (Color){255, 200, 60, 255});
                DrawCircleV(mini.fPos[i], RAIO_FAISCA * 0.45f, WHITE);
            }
            bool piscar = mini.invul > 0 && ((int)(mini.invul * 10.0f) % 2 == 0);
            if (!piscar) {
                DrawCircleV(mini.escudo, RAIO_ESCUDO + 4.0f, Fade((Color){80, 200, 255, 255}, 0.3f));
                DrawCircleV(mini.escudo, RAIO_ESCUDO, (Color){80, 200, 255, 255});
                DrawCircleLines((int)mini.escudo.x, (int)mini.escudo.y, RAIO_ESCUDO, WHITE);
            }
            for (int i = 0; i < MAX_GOLPES; i++)
                coracao(a.x + 22 + i * 30, a.y + 16, 8,
                        i < MAX_GOLPES - mini.golpes ? (Color){240, 60, 80, 255} : (Color){50, 55, 70, 255});
            break;
        }
        default:
            textoCentro(TextFormat("Acertos: %d/8", mini.acertos), 640, 240, 24, WHITE);
            for (int s = 0; s < 6; s++) {
                Rectangle rc = discRect(s);
                bool aceso = s == mini.alvo;
                Color bg = aceso ? (Color){255, 150, 40, 255} : (Color){36, 50, 78, 255};
                Color bd = mini.flash > 0 ? (Color){255, 80, 80, 255} : (aceso ? WHITE : (Color){90, 120, 170, 255});
                caixa(rc, 0.2f, bg, bd);
                textoCentro(aceso ? "DESLIGUE!" : "ok", (int)(rc.x + rc.width / 2), (int)rc.y + 28, aceso ? 22 : 20,
                            aceso ? WHITE : LIGHTGRAY);
            }
            break;
    }

    if (mini.resultado) {
        DrawRectangle(300, 330, 680, 110, Fade(BLACK, 0.85f));
        textoCentro(mini.resultado == 1 ? "SUCESSO!" : "FALHOU!", 640, 355, 60,
                    mini.resultado == 1 ? (Color){110, 240, 150, 255} : (Color){255, 100, 100, 255});
    }
}

static void desenharNome(void)
{
    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.85f));
    textoCentro(vitoria ? "FIM DE JOGO - VITORIA!" : "FIM DE JOGO", 640, 120, 54,
                vitoria ? (Color){110, 240, 150, 255} : (Color){255, 100, 100, 255});
    textoCentro(TextFormat("Pontuacao: %d", pontos), 640, 210, 36, YELLOW);
    textoCentro(TextFormat("Entra no Ranque: %s", nomeAba(rankAtual)), 640, 255, 18, (Color){150, 190, 230, 255});
    textoCentro("Digite seu nome para o ranking:", 640, 285, 22, WHITE);
    caixa(caixaNome, 0.15f, (Color){12, 20, 34, 255}, (Color){255, 215, 60, 255});
    DrawText(nomeJog, (int)caixaNome.x + 16, (int)caixaNome.y + 14, 34, WHITE);
    if (((int)(GetTime() * 2.0)) % 2 == 0)
        DrawRectangle((int)caixaNome.x + 18 + MeasureText(nomeJog, 34), (int)caixaNome.y + 12, 3, 36, WHITE);
    textoCentro(TextFormat("%d/%d letras  |  ENTER confirma", nomeLen, TAM_NOME), 640, 400, 16, LIGHTGRAY);
    desenharBotao(btnOkNome, "Confirmar", (Color){40, 140, 90, 255});
}

static void desenharFim(void)
{
    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.8f));
    if (vitoria && secretaVenceu) {
        textoCentro("CONQUISTA SECRETA!", 640, 90, 70, (Color){255, 215, 60, 255});
        textoCentro("Voce dominou a rede dos 27 estados do Brasil!", 640, 180, 24, WHITE);
    } else if (vitoria) {
        textoCentro("VITORIA!", 640, 90, 80, (Color){110, 240, 150, 255});
        textoCentro("Voce manteve o Brasil ligado em todas as fases!", 640, 180, 24, WHITE);
    } else {
        textoCentro("BLACKOUT GERAL", 640, 90, 80, (Color){255, 90, 90, 255});
        textoCentro("A rede nao aguentou. Tente de novo!", 640, 180, 24, WHITE);
    }
    textoCentro(TextFormat("%s: %d pontos", nomeJog[0] ? nomeJog : "Jogador", pontos), 640, 235, 34, YELLOW);
    if (posRanking > 0) textoCentro(TextFormat("Novo recorde! Posicao %d no Ranque (%s)", posRanking, nomeAba(rankAtual)), 640, 290, 22, (Color){110, 240, 150, 255});
    desenharRankingLista(640, 350, rankAtual, 5);
    desenharBotao(btnFimNovo, "Novo Jogo", (Color){40, 140, 90, 255});
    desenharBotao(btnFimMenu, "Menu", (Color){40, 100, 180, 255});

    if (podeSecreta) {
        float p = 0.5f + 0.5f * sinf((float)GetTime() * 4.0f);
        DrawRectangleRounded((Rectangle){btnSecreta.x - 6, btnSecreta.y - 6, btnSecreta.width + 12, btnSecreta.height + 12},
                             0.3f, 8, Fade((Color){255, 215, 60, 255}, 0.18f + 0.3f * p));
        desenharBotao(btnSecreta, "Fase secreta", (Color){150, 110, 20, 255});
    }
}

static void desenharPainelConfiguracao(void)
{
    if (!painelConfigAberto) return;

    DrawRectangle(0, 0, W, H, Fade(BLACK, 0.65f));

    caixa(painelConfig, 0.06f,
          (Color){14, 26, 48, 250},
          (Color){80, 130, 190, 255});

    textoCentro("CONFIGURACOES", 640, 170, 30,
                (Color){255, 215, 60, 255});

    desenharBotao(btnModoTela,
                  telaCheia ? "Modo: Tela cheia" : "Modo: Janela",
                  (Color){40, 100, 180, 255});

    desenharVolumes();

    desenharBotao(btnFecharConfig,
                  "Fechar",
                  (Color){160, 55, 55, 255});
}

static void desenharVolumes(void)
{
    caixa((Rectangle){330, 498, 620, 130}, 0.08f, (Color){14, 26, 48, 245}, (Color){60, 90, 130, 255});
    textoCentro("VOLUME", 640, 506, 20, (Color){255, 215, 60, 255});

    const char *nomes[2] = {"Musica", "Efeitos"};
    float vals[2] = {volMus, volSfx};
    Color cores[2] = {(Color){80, 170, 255, 255}, (Color){255, 190, 60, 255}};

    for (int i = 0; i < 2; i++) {
        Rectangle b = barraVol(i);
        bool ativo = (arrastaVol == i);
        DrawText(nomes[i], 355, (int)b.y - 3, 22, WHITE);
        DrawRectangleRounded((Rectangle){b.x - 2, b.y - 2, b.width + 4, b.height + 4}, 0.5f, 6,
                             ativo ? WHITE : (Color){60, 90, 130, 255});
        DrawRectangleRounded(b, 0.5f, 6, (Color){12, 20, 34, 255});
        if (vals[i] > 0.01f)
            DrawRectangleRounded((Rectangle){b.x, b.y, b.width * vals[i], b.height}, 0.5f, 6, cores[i]);
        float kx = b.x + b.width * vals[i];
        DrawCircle((int)kx, (int)(b.y + b.height / 2), ativo ? 12 : 10, WHITE);
        DrawCircle((int)kx, (int)(b.y + b.height / 2), ativo ? 8 : 6, cores[i]);
        DrawText(TextFormat("%d%%", (int)roundf(vals[i] * 100.0f)), 810, (int)b.y - 3, 22, cores[i]);
    }
}

static void desenharEngrenagem(void)
{
    Rectangle rc = btnEngrenagem;
    bool h = hover(rc);

    DrawRectangleRounded(
        (Rectangle){rc.x + 2, rc.y + 4, rc.width, rc.height},
        0.25f, 8, Fade(BLACK, 0.45f)
    );

    caixa(
        rc, 0.25f,
        h ? (Color){55, 75, 105, 255} : (Color){25, 38, 60, 255},
        h ? WHITE : (Color){100, 130, 170, 255}
    );

    textoCentro(
        "config",
        (int)(rc.x + rc.width / 2),
        (int)(rc.y + 14),
        18,
        WHITE
    );
}

void desenhar(void)
{
    ClearBackground(BLACK);

    switch (tela) {
        case TELA_MENU:
            desenharMenu();
            break;

        case TELA_AJUDA:
            desenharAjuda();
            break;

        case TELA_FASES:
            desenharFases();
            break;

        case TELA_RANK:
            fundoMenu((float)GetTime());
            desenharRank();
            break;

        case TELA_JOGO:
            cena();
            break;

        case TELA_PAUSA:
            cena();
            DrawRectangle(0, 0, W, H, Fade(BLACK, 0.65f));
            textoCentro("PAUSADO", 640, 200, 60, WHITE);
            desenharBotao(btnContinuar, "Continuar", (Color){40, 140, 90, 255});
            desenharBotao(btnNovoPausa, "Novo Jogo", (Color){120, 90, 170, 255});
            desenharBotao(btnMenuPausa, "Menu", (Color){160, 55, 55, 255});
            desenharVolumes();
            break;

        case TELA_TROCA:
            cena();
            desenharTroca();
            break;

        case TELA_MINI:
            cena();
            desenharMini();
            break;

        case TELA_NOME:
            cena();
            desenharNome();
            break;

        case TELA_FIM:
            cena();
            desenharFim();
            break;
    }

    desenharEngrenagem();
    desenharPainelConfiguracao();
}
