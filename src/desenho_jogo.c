#include <string.h>
#include "tipos.h"
#include "globais.h"
#include "dados.h"
#include "eventos.h"
#include "ui.h"
#include "util.h"
#include "desenho_jogo.h"

void desenharMapa(void)
{
    float t = (float)GetTime();
    Vector2 m = mouseVirtual();
    int sob = regiaoSob(m);

    /* 1. Sombra de Profundidade 3D (Deslocada) de cada Região */
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        Vector2 shadowPt[56];
        for (int k = 0; k < g->n; k++) {
            shadowPt[k] = (Vector2){g->pt[k].x + 6.0f, g->pt[k].y + 8.0f};
        }
        preencherComGradiente(shadowPt, g->n, (Color){5, 10, 20, 180}, (Color){0, 0, 5, 200});
    }

    /* 2. Preenchimento de Território com Gradientes Dinâmicos de Iluminação */
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        Color fillTop = (Color){20, 32, 50, 255};
        Color fillBot = (Color){10, 18, 30, 255};

        if (g->ativa) {
            fillTop = ColorBrightness(g->cor, 0.15f);
            fillBot = ColorBrightness(g->cor, -0.25f);
            if (i == selecionada) {
                fillTop = ColorBrightness(fillTop, 0.30f);
                fillBot = ColorBrightness(fillBot, 0.15f);
            }
            if (arrasta >= 0 && sob == i && i != arrasta) {
                fillTop = ColorBrightness(fillTop, 0.45f);
                fillBot = ColorBrightness(fillBot, 0.25f);
            }
        }
        preencherComGradiente(g->pt, g->n, fillTop, fillBot);
    }

    /* 3. Divisas Internas dos Estados (Linhas Pontilhadas Finas) */
    for (int i = 0; i < NE; i++) {
        int reg = estDef[i].reg;
        if (r[reg].ativa) {
            Vector2 pos = (Vector2){projX(estDef[i].lon), projY(estDef[i].lat)};
            DrawCircleV(pos, 2.0f, Fade(WHITE, 0.35f));
        }
    }

    /* 4. Fronteiras Regionais em Alta Definição e Glow Neon */
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        bool sel = g->ativa && i == selecionada;
        Color lc = g->ativa ? (Color){15, 30, 55, 255} : (Color){60, 75, 95, 255};
        if (sel) lc = WHITE;
        if (arrasta >= 0 && sob == i && i != arrasta) lc = (Color){255, 255, 140, 255};

        for (int k = 0; k < g->n; k++) {
            Vector2 p1 = g->pt[k];
            Vector2 p2 = g->pt[(k + 1) % g->n];
            if (sel) {
                DrawLineEx(p1, p2, 5.0f, Fade(g->cor, 0.4f));
                DrawLineEx(p1, p2, 2.5f, WHITE);
            } else {
                DrawLineEx(p1, p2, 2.0f, lc);
            }
        }
    }

    /* 5. Indicadores de Eventos Climáticos / Energéticos */
    for (int i = 0; i < N; i++) {
        if (!r[i].ativa) continue;
        float net = 0;
        for (int e = 0; e < nev; e++)
            net += (evDefs[evs[e].id].prod[i] - evDefs[evs[e].id].cons[i]) * intensidade();
        if (fabsf(net) > 0.04f)
            setaEvento(r[i].c.x + 40, r[i].c.y - 24, net > 0, 1.0f + 0.15f * sinf(t * 6.0f));
    }

    /* 6. Centros Operacionais, Medidores de Bateria e Alertas */
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        if (!g->ativa) continue;
        float cx = g->c.x, cy = g->c.y;

        bool lo = g->bat < LIM_BAIXO, hi = g->bat > LIM_ALTO;
        if (lo || hi) {
            Color ac = lo ? (Color){255, 60, 60, 255} : (Color){255, 150, 40, 255};
            float pulso = 0.5f + 0.5f * sinf(t * 10.0f);
            DrawCircleLines((int)cx, (int)cy, 52 + pulso * 8, Fade(ac, 0.5f + 0.5f * pulso));
            DrawCircleLines((int)cx, (int)cy, 54 + pulso * 8, Fade(ac, 0.5f + 0.5f * pulso));
            const char *txt = lo ? "ALERTA: BLACKOUT" : "ALERTA: SOBRECARGA";
            int tw = MeasureText(txt, 14);
            DrawRectangle((int)cx - tw / 2 - 6, (int)cy + 36, tw + 12, 22, Fade(ac, 0.9f));
            DrawText(txt, (int)cx - tw / 2, (int)cy + 40, 14, WHITE);
        }

        icone(g->fonte, cx, cy - 24, t, 0.9f);
        textoCentro(g->nome, (int)cx, (int)cy - 2, 16, WHITE);

        Rectangle br = {cx - 45, cy + 16, 90, 14};
        DrawRectangleRounded((Rectangle){br.x - 2, br.y - 2, br.width + 4, br.height + 4}, 0.4f, 6, Fade(BLACK, 0.8f));
        DrawRectangleRounded(br, 0.4f, 6, (Color){30, 40, 55, 255});
        DrawRectangleRounded((Rectangle){br.x, br.y, fmaxf(4.0f, br.width * g->bat / BAT_MAX), br.height}, 0.4f, 6,
                             corBatVisual(g->bat));
        DrawRectangle((int)(br.x + br.width * LIM_BAIXO / BAT_MAX), (int)br.y - 2, 2, (int)br.height + 4, WHITE);
        DrawRectangle((int)(br.x + br.width * LIM_ALTO / BAT_MAX), (int)br.y - 2, 2, (int)br.height + 4, WHITE);
        textoCentro(TextFormat("%.0f%%", g->bat), (int)cx, (int)br.y + 1, 12, WHITE);

        float seg = tempoCritico(g->bat, rateReg(i));
        if (seg >= 0.0f && seg <= TEMPO_CRONO) cronometro(cx - 74, cy + 6, 22, seg, lo);
    }

    /* 7. Arraste e Linhas de Conexão Curvadas (VFX de Apoio Elétrico) */
    if (arrasta >= 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 start = r[arrasta].c;
        Vector2 ctrl = (Vector2){(start.x + m.x) * 0.5f, fminf(start.y, m.y) - 40.0f};
        DrawSplineSegmentBezierCubic(start, ctrl, ctrl, m, 4.0f, Fade((Color){255, 220, 80, 255}, 0.9f));
        DrawCircleV(m, 8.0f, YELLOW);
        DrawCircleLines((int)m.x, (int)m.y, 12, WHITE);
    }

    if (fluxoT > 0) {
        float p = 1.0f - fluxoT / 1.2f;
        Vector2 a = r[fluxoO].c, b = r[fluxoD].c;
        Vector2 ctrl = (Vector2){(a.x + b.x) * 0.5f, fminf(a.y, b.y) - 30.0f};

        for (int k = 0; k < 6; k++) {
            float u = p * 1.3f - k * 0.07f;
            if (u <= 0 || u >= 1) continue;
            float omt = 1.0f - u;
            Vector2 pos = {
                omt * omt * a.x + 2 * omt * u * ctrl.x + u * u * b.x,
                omt * omt * a.y + 2 * omt * u * ctrl.y + u * u * b.y
            };
            DrawCircleV(pos, 6.0f, (Color){255, 230, 80, 255});
            DrawCircleV(pos, 3.0f, WHITE);
        }
    }
}

static void desenharMapaSecreto(void)
{
    float t = (float)GetTime();
    Vector2 m = mouseVirtual();
    int sob = estadoSob(m);
    float pulso = 0.5f + 0.5f * sinf(t * 10.0f);

    Color colEst[NE];
    for (int i = 0; i < NE; i++) {
        Color base = ColorBrightness(r[est[i].reg].cor, ((i * 7) % 5 - 2) * 0.07f);
        Color c = misturar(base, corBatVisual(est[i].bat), 0.30f);
        if (est[i].bat < LIM_BAIXO || est[i].bat > LIM_ALTO)
            c = misturar(c, corBatVisual(est[i].bat), 0.35f * pulso);
        if (i == selEst) c = ColorBrightness(c, 0.25f);
        if (i == sob) c = ColorBrightness(c, arrasta >= 0 ? 0.35f : 0.2f);
        colEst[i] = c;
    }

    int x0 = 270 / CEL, x1 = 960 / CEL;
    for (int y = 60 / CEL; y < GH; y++) {
        for (int x = x0; x < x1 && x < GW; x++) {
            int e = gridEst[y][x];
            if (e < 0) continue;
            DrawRectangle(x * CEL, y * CEL, CEL, CEL, colEst[e]);
        }
    }

    Color escuro = (Color){10, 18, 32, 255};
    for (int y = 60 / CEL; y < GH - 1; y++) {
        for (int x = x0; x < x1 && x < GW - 1; x++) {
            int a = gridEst[y][x];
            int dir = gridEst[y][x + 1], baixo = gridEst[y + 1][x];
            if (a != dir && !(a < 0 && dir < 0)) {
                bool s = (a == selEst || dir == selEst);
                if (a >= 0 && dir >= 0) DrawRectangle(x * CEL + CEL - 1, y * CEL, s ? 2 : 1, CEL, s ? WHITE : escuro);
                else if (s) DrawRectangle(x * CEL + CEL - 1, y * CEL, 2, CEL, WHITE);
            }
            if (a != baixo && !(a < 0 && baixo < 0)) {
                bool s = (a == selEst || baixo == selEst);
                if (a >= 0 && baixo >= 0) DrawRectangle(x * CEL, y * CEL + CEL - 1, CEL, s ? 2 : 1, s ? WHITE : escuro);
                else if (s) DrawRectangle(x * CEL, y * CEL + CEL - 1, CEL, 2, WHITE);
            }
        }
    }

    for (int i = 0; i < N; i++)
        for (int k = 0; k < r[i].n; k++)
            DrawLineEx(r[i].pt[k], r[i].pt[(k + 1) % r[i].n], 2.5f, escuro);

    if (arrasta >= 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        DrawLineEx(est[arrasta].c, m, 4.0f, Fade(WHITE, 0.85f));
        DrawCircle((int)m.x, (int)m.y, 8, YELLOW);
    }
    if (fluxoT > 0) {
        float p = 1.0f - fluxoT / 1.0f;
        for (int k = 0; k < 5; k++) {
            float u = p * 1.3f - k * 0.07f;
            if (u <= 0 || u >= 1) continue;
            Vector2 a = est[fluxoO].c, b = est[fluxoD].c;
            DrawCircle((int)(a.x + (b.x - a.x) * u), (int)(a.y + (b.y - a.y) * u), 5, (Color){255, 230, 80, 255});
        }
    }

    for (int i = 0; i < NE; i++) {
        Estado *e = &est[i];
        bool lo = e->bat < LIM_BAIXO, hi = e->bat > LIM_ALTO;
        if (lo || hi) {
            Color ac = lo ? (Color){255, 60, 60, 255} : (Color){255, 150, 40, 255};
            DrawCircleLines((int)e->c.x, (int)e->c.y, 14 + pulso * 5, Fade(ac, 0.5f + 0.5f * pulso));
        }
        int tw = MeasureText(e->sig, 12);
        DrawText(e->sig, (int)e->c.x - tw / 2 + 1, (int)e->c.y - 6 + 1, 12, Fade(BLACK, 0.8f));
        DrawText(e->sig, (int)e->c.x - tw / 2, (int)e->c.y - 6, 12, WHITE);
        DrawRectangle((int)e->c.x - 12, (int)e->c.y + 8, 24, 5, Fade(BLACK, 0.75f));
        DrawRectangle((int)e->c.x - 11, (int)e->c.y + 9, (int)(22 * e->bat / BAT_MAX), 3, corBatVisual(e->bat));
    }

    for (int i = 0; i < NE; i++) {
        Estado *e = &est[i];
        float seg = tempoCritico(e->bat, rateEst(i));
        if (seg >= 0.0f && seg <= TEMPO_CRONO)
            cronometro(e->c.x, e->c.y - 26, 14, seg, e->bat < LIM_BAIXO);
    }

    for (int i = 0; i < N; i++) {
        float net = 0;
        for (int e = 0; e < nev; e++)
            net += (evDefs[evs[e].id].prod[i] - evDefs[evs[e].id].cons[i]) * intensidade();
        if (fabsf(net) > 0.04f)
            setaEvento(r[i].c.x, r[i].c.y - 34, net > 0, 1.0f + 0.15f * sinf(t * 6.0f));
    }

    if (sob >= 0) {
        Estado *e = &est[sob];
        const char *txt = TextFormat("%s (%s) %.0f%%", e->nome, abrev[e->reg], e->bat);
        int tw = MeasureText(txt, 16);
        float bx = lim(m.x + 14, 290, 940 - tw - 12), by = m.y - 34;
        DrawRectangle((int)bx, (int)by, tw + 12, 24, Fade(BLACK, 0.85f));
        DrawText(txt, (int)bx + 6, (int)by + 4, 16, WHITE);
    }
}

static void desenharEventos(void)
{
    float t = (float)GetTime();
    caixa((Rectangle){10, 80, 265, 540}, 0.03f, (Color){14, 26, 48, 240}, (Color){60, 90, 130, 255});
    DrawText("EVENTOS", 24, 92, 20, RAYWHITE);
    float y = 122;
    if (nev == 0) DrawText("Tudo calmo por enquanto.", 24, (int)y, 15, LIGHTGRAY);

    for (int e = 0; e < nev; e++) {
        int id = evs[e].id;
        char lin[N][80];
        float nets[N];
        int cnt = 0;
        for (int i = 0; i < N; i++)
            if (r[i].ativa && linhaEfeito(lin[cnt], 80, id, i, &nets[cnt])) cnt++;
        float h = 56.0f + cnt * 18.0f;
        caixa((Rectangle){20, y, 245, h}, 0.1f, (Color){28, 42, 70, 255}, (Color){240, 170, 50, 255});
        DrawText(evDefs[id].nome, 30, (int)y + 8, 18, (Color){255, 200, 80, 255});
        float f = lim(evs[e].t / evs[e].dur, 0.0f, 1.0f);
        DrawRectangle(30, (int)y + 32, 225, 6, (Color){20, 28, 45, 255});
        DrawRectangle(30, (int)y + 32, (int)(225 * f), 6, (Color){240, 170, 50, 255});
        for (int k = 0; k < cnt; k++)
            DrawText(lin[k], 30, (int)y + 44 + k * 18, 14, nets[k] > 0 ? (Color){110, 240, 150, 255} : (Color){255, 120, 120, 255});
        y += h + 8;
    }

    if (spawnT < 6.0f && nev < maxEventos() && sinf(t * 8.0f) > -0.3f)
        DrawText("EVENTO CHEGANDO!", 24, 595, 18, (Color){255, 200, 60, 255});
    if (toastT > 0) DrawText(toast, 14, 632, 15, (Color){255, 220, 120, 255});
}

static void desenharControleAjuste(float ajuste)
{
    DrawText("Producao (W/S, roda; R = reset):", 960, 540, 14, (Color){150, 170, 200, 255});
    desenharBotao(btnMenos, "-", (Color){170, 60, 60, 255});
    desenharBotao(btnMais, "+", (Color){50, 150, 90, 255});
    desenharBotao(btnReset, "Reset", (Color){70, 90, 150, 255});
    DrawText(TextFormat("%.0f%%", ajuste * 100), 1168, 574, 26, WHITE);
    DrawRectangle(960, 622, 290, 10, (Color){12, 20, 34, 255});
    DrawRectangle(960, 622, (int)(290 * ajuste / 1.5f), 10, (Color){80, 170, 255, 255});
}

static void desenharPainelDireito(void)
{
    caixa((Rectangle){945, 85, 320, 625}, 0.03f, (Color){14, 26, 48, 240}, (Color){60, 90, 130, 255});
    DrawText("REGIOES", 960, 92, 18, RAYWHITE);

    int qtd = regioesNaFase(fase);
    for (int k = 0; k < qtd; k++) {
        int id = ordemFase[k];
        Rectangle rc = linhaReg(k);
        bool sel = id == selecionada;
        caixa(rc, 0.2f, sel ? (Color){40, 62, 100, 255} : (Color){24, 36, 60, 255},
              sel ? WHITE : (hover(rc) ? (Color){120, 150, 200, 255} : (Color){50, 70, 100, 255}));
        DrawRectangle((int)rc.x + 8, (int)rc.y + 8, 8, 32, r[id].cor);
        DrawText(r[id].nome, (int)rc.x + 26, (int)rc.y + 6, 17, WHITE);
        Rectangle br = {rc.x + 26, rc.y + 28, 200, 10};
        DrawRectangleRounded(br, 0.5f, 6, (Color){12, 20, 34, 255});
        DrawRectangleRounded((Rectangle){br.x, br.y, fmaxf(4.0f, br.width * r[id].bat / BAT_MAX), br.height}, 0.5f, 6,
                             corBatVisual(r[id].bat));
        DrawRectangle((int)(br.x + br.width * LIM_BAIXO / BAT_MAX), (int)br.y - 2, 2, 14, WHITE);
        DrawRectangle((int)(br.x + br.width * LIM_ALTO / BAT_MAX), (int)br.y - 2, 2, 14, WHITE);
        DrawText(TextFormat("%.0f%%", r[id].bat), (int)rc.x + 238, (int)rc.y + 14, 20, corBatVisual(r[id].bat));
    }

    Regiao *g = &r[selecionada];
    float prod = g->ajuste * g->multProd, cons = g->consBase * g->multCons;
    float saldo = (prod - cons) * 4.0f * velFase();

    DrawText("REGIAO SELECIONADA", 960, 396, 14, (Color){150, 170, 200, 255});
    DrawText(g->nome, 960, 416, 26, g->cor);
    DrawText(TextFormat("Bateria: %.0f%%   (seguro 30-80)", g->bat), 960, 452, 17, WHITE);
    DrawText(TextFormat("Producao: %.0f%%   Consumo: %.0f%%", prod * 100, cons * 100), 960, 476, 16, LIGHTGRAY);
    DrawText(TextFormat("Saldo: %+.1f%%/s", saldo), 960, 500, 18,
             saldo > 0.05f ? (Color){255, 200, 80, 255} : (saldo < -0.05f ? (Color){255, 110, 110, 255} : (Color){110, 240, 150, 255}));
    desenharControleAjuste(g->ajuste);

    DrawText("Segure numa regiao e arraste ate outra", 960, 645, 14, LIGHTGRAY);
    DrawText("para apoiar (vizinha +8%, longe +5%).", 960, 663, 14, LIGHTGRAY);
    DrawRectangle(960, 688, 290, 8, (Color){12, 20, 34, 255});
    if (cooldown > 0) DrawRectangle(960, 688, (int)(290 * (1.0f - cooldown / 1.0f)), 8, (Color){255, 220, 80, 255});
    else DrawRectangle(960, 688, 290, 8, (Color){70, 225, 140, 255});
}

static void desenharPainelSecreto(void)
{
    caixa((Rectangle){945, 85, 320, 625}, 0.03f, (Color){14, 26, 48, 240}, (Color){255, 215, 60, 255});
    DrawText("FASE SECRETA", 960, 96, 22, (Color){255, 215, 60, 255});

    int alertas = 0;
    float media[N] = {0};
    int cnt[N] = {0};
    for (int i = 0; i < NE; i++) {
        if (est[i].bat < LIM_BAIXO || est[i].bat > LIM_ALTO) alertas++;
        media[est[i].reg] += est[i].bat;
        cnt[est[i].reg]++;
    }
    DrawText(TextFormat("Estados em alerta: %d / %d", alertas, NE), 960, 130, 18,
             alertas ? (Color){255, 150, 60, 255} : (Color){110, 240, 150, 255});

    DrawText("MEDIA POR REGIAO", 960, 168, 14, (Color){150, 170, 200, 255});
    for (int i = 0; i < N; i++) {
        float mm = cnt[i] ? media[i] / cnt[i] : 0;
        DrawRectangle(960, 192 + i * 28, 8, 20, r[i].cor);
        DrawText(r[i].nome, 976, 194 + i * 28, 16, WHITE);
        DrawRectangle(1090, 196 + i * 28, 120, 10, (Color){12, 20, 34, 255});
        DrawRectangle(1090, 196 + i * 28, (int)fmaxf(3.0f, 120 * mm / BAT_MAX), 10, corBatVisual(mm));
        DrawText(TextFormat("%.0f", mm), 1218, 192 + i * 28, 16, corBatVisual(mm));
    }

    Estado *g = &est[selEst];
    float prod = g->ajuste * r[g->reg].multProd, cons = g->cons * r[g->reg].multCons;
    float saldo = (prod - cons) * 4.0f * velFase();

    DrawText("ESTADO SELECIONADO", 960, 350, 14, (Color){150, 170, 200, 255});
    DrawText(g->nome, 960, 370, 26, r[g->reg].cor);
    DrawText(TextFormat("Regiao: %s", r[g->reg].nome), 960, 404, 16, LIGHTGRAY);
    DrawText(TextFormat("Bateria: %.0f%%   (seguro 30-80)", g->bat), 960, 430, 17, WHITE);
    DrawText(TextFormat("Producao: %.0f%%   Consumo: %.0f%%", prod * 100, cons * 100), 960, 456, 16, LIGHTGRAY);
    DrawText(TextFormat("Saldo: %+.1f%%/s", saldo), 960, 482, 18,
             saldo > 0.05f ? (Color){255, 200, 80, 255} : (saldo < -0.05f ? (Color){255, 110, 110, 255} : (Color){110, 240, 150, 255}));
    desenharControleAjuste(g->ajuste);

    DrawText("Clique num estado para seleciona-lo.", 960, 645, 14, LIGHTGRAY);
    DrawText("Arraste de um estado a outro: apoio.", 960, 663, 14, LIGHTGRAY);
    DrawRectangle(960, 688, 290, 8, (Color){12, 20, 34, 255});
    if (cooldown > 0) DrawRectangle(960, 688, (int)(290 * (1.0f - cooldown / 0.5f)), 8, (Color){255, 220, 80, 255});
    else DrawRectangle(960, 688, 290, 8, (Color){70, 225, 140, 255});
}

static void desenharCombo(void)
{
    Color cores[COMBO_MAX + 1] = {
        {150, 170, 200, 255}, {150, 170, 200, 255}, {110, 240, 150, 255},
        {255, 220, 70, 255}, {255, 150, 50, 255}, {255, 80, 110, 255}
    };
    Color c = cores[combo];
    Rectangle rc = {985, 8, 110, 34};
    float pulso = 0.5f + 0.5f * sinf((float)GetTime() * 8.0f);

    caixa(rc, 0.25f, (Color){20, 32, 54, 255}, combo > 1 ? Fade(c, combo == COMBO_MAX ? 0.6f + 0.4f * pulso : 1.0f) : (Color){60, 90, 130, 255});
    DrawText("BONUS", (int)rc.x + 8, (int)rc.y + 4, 12, (Color){150, 170, 200, 255});
    const char *tx = TextFormat("x%d", combo);
    DrawText(tx, (int)(rc.x + rc.width) - 10 - MeasureText(tx, 26), (int)rc.y + 3, 26, c);

    float f = combo >= COMBO_MAX ? 1.0f : lim(comboT / COMBO_SEG, 0.0f, 1.0f);
    DrawRectangle((int)rc.x + 8, (int)rc.y + 22, 50, 5, (Color){12, 20, 34, 255});
    DrawRectangle((int)rc.x + 8, (int)rc.y + 22, (int)(50 * f), 5, c);
}

static void desenharHUD(void)
{
    DrawRectangleGradientV(0, 0, W, 64, (Color){8, 16, 34, 255}, (Color){14, 30, 58, 255});
    DrawRectangle(0, 64, W, 2, (Color){50, 90, 140, 255});
    DrawText("VOLTERRA", 22, 16, 32, (Color){255, 215, 60, 255});
    if (secreta) textoCentro("FASE SECRETA", 420, 10, 24, (Color){255, 215, 60, 255});
    else textoCentro(TextFormat("FASE %d/%d", fase, FASES), 420, 10, 24, WHITE);
    textoCentro(TextFormat("Dia %d/%d", dia, diasFase()), 420, 38, 18, LIGHTGRAY);

    float prog = ((dia - 1) * diaDur() + tempoDia) / (diasFase() * diaDur());
    DrawRectangleRounded((Rectangle){520, 22, 250, 18}, 0.5f, 6, (Color){20, 32, 54, 255});
    DrawRectangleRounded((Rectangle){520, 22, fmaxf(6.0f, 250 * lim(prog, 0, 1)), 18}, 0.5f, 6, (Color){80, 170, 255, 255});
    for (int i = 1; i < diasFase(); i++) DrawRectangle(520 + 250 * i / diasFase(), 22, 2, 18, (Color){14, 26, 48, 255});
    textoCentro("progresso da fase", 645, 44, 12, (Color){150, 170, 200, 255});

    DrawText("VIDAS", 810, 8, 14, (Color){150, 170, 200, 255});
    for (int i = 0; i < vidasMax; i++) {
        float hx = 826.0f + i * 28.0f;
        coracao(hx, 32, 9, i < vidas ? (Color){240, 60, 80, 255} : (Color){50, 55, 70, 255});
        if (i < vidas) DrawCircle((int)hx - 7, 27, 2, Fade(WHITE, 0.7f));
    }

    desenharCombo();

    const char *p = TextFormat("%d", pontos);
    DrawText("PONTOS", 1260 - MeasureText("PONTOS", 14), 8, 14, (Color){150, 170, 200, 255});
    DrawText(p, 1260 - MeasureText(p, 30), 26, 30, WHITE);
    DrawText("[P] ou ESC: pausa e volume", 985, 46, 13, (Color){120, 140, 170, 255});
}

static void desenharBanner(void)
{
    if (msgT <= 0) return;
    Color base[4] = {{40, 140, 230, 255}, {255, 170, 40, 255}, {50, 200, 120, 255}, {235, 60, 60, 255}};
    Color c = base[corMsg & 3];
    float a = msgT < 0.6f ? msgT / 0.6f : 1.0f;

    /* Rodape, sob o mapa (entre o painel esquerdo e o painel direito) */
    Rectangle b = {290, H - 86, 630, 74};

    /* brilho externo (camadas translucidas) + borda colorida + fundo escuro */
    for (int g = 4; g >= 1; g--)
        DrawRectangleRounded((Rectangle){b.x - g * 2, b.y - g * 2, b.width + g * 4, b.height + g * 4},
                             0.2f, 8, Fade(c, 0.05f * a));
    caixa(b, 0.2f, Fade((Color){10, 16, 30, 255}, 0.96f * a), Fade(c, a));
    DrawRectangleRounded(b, 0.2f, 8, Fade(ColorBrightness(c, -0.75f), 0.35f * a));

    /* icone a esquerda: triangulo de alerta (avisos) ou circulo "i" (info) */
    float ix = b.x + 42, iy = b.y + b.height / 2;
    if ((corMsg & 3) == 1 || (corMsg & 3) == 3) {
        Vector2 p1 = {ix, iy - 26}, p2 = {ix - 28, iy + 22}, p3 = {ix + 28, iy + 22};
        DrawTriangle(p1, p2, p3, Fade(c, 0.35f * a));
        DrawTriangleLines(p1, p2, p3, Fade(c, a));
        textoCentro("!", (int)ix, (int)iy - 8, 28, Fade(WHITE, a));
    } else {
        DrawCircleV((Vector2){ix, iy}, 24, Fade(c, 0.35f * a));
        DrawCircleLines((int)ix, (int)iy, 24, Fade(c, a));
        textoCentro("i", (int)ix, (int)iy - 12, 24, Fade(WHITE, a));
    }

    /* titulo (colorido) e mensagem (ate 2 linhas), alinhados a esquerda */
    float tx = b.x + 88, maxw = b.width - 88 - 18;
    DrawText(tituloMsg, (int)tx, (int)b.y + 8, 22, Fade(ColorBrightness(c, 0.25f), a));
    Color ct = Fade((Color){235, 240, 250, 255}, a);
    const int tam = 15;
    if (MeasureText(msg, tam) <= maxw) {
        DrawText(msg, (int)tx, (int)b.y + 38, tam, ct);
    } else {
        char l1[200], l2[200];
        snprintf(l1, sizeof(l1), "%s", msg);
        int corte = (int)strlen(l1);
        while (corte > 0 && MeasureText(l1, tam) > maxw) {
            while (corte > 0 && l1[corte - 1] != ' ') corte--;
            if (corte > 0) l1[--corte] = '\0';
        }
        if (corte <= 0) { snprintf(l1, sizeof(l1), "%s", msg); corte = (int)strlen(l1); }
        snprintf(l2, sizeof(l2), "%s", msg + corte + (msg[corte] == ' ' ? 1 : 0));
        while (strlen(l2) > 3 && MeasureText(l2, tam) > maxw) l2[strlen(l2) - 1] = '\0';
        DrawText(l1, (int)tx, (int)b.y + 36, tam, ct);
        DrawText(l2, (int)tx, (int)b.y + 54, tam, ct);
    }
    if (corMsg == 1 && msgT > 4.2f)
        DrawRectangleLinesEx((Rectangle){0, 0, W, H}, 8, Fade((Color){255, 170, 40, 255}, (msgT - 4.2f) / 0.8f));
}

/* Fundo Marítimo Cibernético / Oceanográfico com Grade & Malha */
void desenharFundoMaritimo(void)
{
    float t = (float)GetTime();

    /* 1. Gradiente Oceânico (Azul Abissal Teccen) */
    DrawRectangleGradientV(0, 0, W, H, (Color){6, 14, 32, 255}, (Color){10, 30, 56, 255});

    /* 2. Malha Geográfica Digital (Linhas Lat/Lon) */
    for (int x = 200; x < 950; x += 50) {
        DrawLineEx((Vector2){(float)x, 0}, (Vector2){(float)x, (float)H}, 1.0f, Fade((Color){30, 70, 120, 255}, 0.15f));
    }
    for (int y = 0; y < H; y += 50) {
        DrawLineEx((Vector2){200, (float)y}, (Vector2){950, (float)y}, 1.0f, Fade((Color){30, 70, 120, 255}, 0.15f));
    }

    /* 3. Anéis de Marés e Efeito de Espuma Marítima (Coastline Waves) */
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        for (int k = 0; k < g->n; k += 3) {
            Vector2 p = g->pt[k];
            float wave = sinf(t * 2.5f + p.x * 0.05f + p.y * 0.05f) * 4.0f;
            DrawCircleV(p, 6.0f + wave, Fade((Color){40, 120, 210, 255}, 0.12f));
        }
    }
}

void cena(void)
{
    desenharFundoMaritimo();
    if (secreta) desenharMapaSecreto();
    else desenharMapa();
    desenharEventos();
    if (secreta) desenharPainelSecreto();
    else desenharPainelDireito();
    desenharHUD();
    desenharBanner();
}