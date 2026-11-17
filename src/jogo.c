#include "tipos.h"
#include "globais.h"
#include "dados.h"
#include "audio.h"
#include "eventos.h"
#include "minigames.h"
#include "ranking.h"
#include "regioes.h"
#include "util.h"
#include "jogo.h"

void resetarCombo(bool avisar)
{
    if (avisar && combo > 1) aviso("Bonus perdido! Voltou para x1.");
    combo = 1;
    comboT = 0;
}

static void avancarCombo(float dt)
{
    if (combo >= COMBO_MAX) { comboT = 0; return; }
    comboT += dt;
    if (comboT >= COMBO_SEG) {
        comboT = 0;
        combo++;
        aviso(TextFormat("Bonus x%d!", combo));
        tocar(sndApoio);
    }
}

void voltarMenu(void)
{
    podeSecreta = false;
    secreta = false;
    secretaVenceu = false;
    iniciarRegioes();
    tela = TELA_MENU;
}

static void prepararFase(void)
{
    int qtd = regioesNaFase(fase);
    for (int i = 0; i < N; i++) r[i].ativa = false;
    for (int k = 0; k < qtd; k++) {
        int id = ordemFase[k];
        r[id].ativa = true;
        r[id].bat = 70.0f;
        r[id].ajuste = 1.0f;
        r[id].multProd = 1.0f;
        r[id].multCons = 1.0f;
    }
    if (!r[selecionada].ativa) selecionada = ordemFase[0];

    dia = 1;
    tempoDia = 0;
    nev = 0;
    spawnT = 16.0f - 1.0f * (fase - 1);
    cooldown = 0;
    fluxoT = 0;
    msgT = 0;
    toastT = 0;
    arrasta = -1;
    tickT = 0;
    memset(alertaPrev, 0, sizeof(alertaPrev));
}

void novoJogoFase(int f)
{
    podeSecreta = false;
    secreta = false;
    secretaVenceu = false;
    iniciarRegioes();
    fase = f;
    vidas = 3;
    vidasMax = 3;
    pontos = 0;
    pontosAcum = 0;
    tempoTotal = 0;
    bonusFase = 0;
    introJogo = true;
    selecionada = SUDESTE;
    resetarCombo(false);
    prepararFase();
    tela = TELA_TROCA;
}

void novoJogo(void)
{
    novoJogoFase(1);
}

void iniciarSecreta(void)
{
    secreta = true;
    podeSecreta = false;
    fase = FASES;
    vidasMax = 5;
    vidas = 5;
    resetarCombo(false);
    for (int i = 0; i < N; i++) {
        r[i].ativa = true;
        r[i].multProd = 1.0f;
        r[i].multCons = 1.0f;
    }
    for (int i = 0; i < NE; i++) {
        est[i].nome = estDef[i].nome;
        est[i].sig = estDef[i].sig;
        est[i].reg = estDef[i].reg;
        est[i].c = (Vector2){projX(estDef[i].lon), projY(estDef[i].lat)};
        est[i].bat = 65.0f;
        est[i].ajuste = 1.0f;
        est[i].cons = r[estDef[i].reg].consBase + (((i * 5) % 7) - 3) * 0.008f;
    }
    construirGrade();
    selEst = 23;
    dia = 1;
    tempoDia = 0;
    nev = 0;
    spawnT = 14.0f;
    cooldown = 0;
    fluxoT = 0;
    msgT = 0;
    toastT = 0;
    arrasta = -1;
    tickT = 0;
    memset(alertaPrev, 0, sizeof(alertaPrev));
    introJogo = false;
    tela = TELA_TROCA;
}

static void prosseguirParaProximaFase(void)
{
    fase++;
    introJogo = true;
    prepararFase();
    tela = TELA_TROCA;
}

static void avancarFase(void)
{
    bonusFase = 500 * fase + vidas * 200;
    pontos += bonusFase;

    if (fase >= FASES) {
        podeSecreta = (vidas >= vidasMax);
        fimDePartida(true);
        return;
    }

    introJogo = false;
    tela = TELA_TROCA;
}

static void transferir(int o, int d)
{
    if (cooldown > 0) { aviso("Aguarde para enviar de novo."); return; }
    if (r[o].bat < 25.0f) { aviso("Origem sem energia suficiente."); return; }
    float ganho = vizinho[o][d] ? 8.0f : 5.0f;
    r[o].bat -= 10.0f;
    r[d].bat = lim(r[d].bat + ganho, 0.0f, BAT_MAX);
    cooldown = 1.0f;
    fluxoO = o;
    fluxoD = d;
    fluxoT = 1.2f;
    tocar(sndApoio);
}

static void transferirE(int o, int d)
{
    if (cooldown > 0) { aviso("Aguarde para enviar de novo."); return; }
    if (est[o].bat < 25.0f) { aviso("Origem sem energia suficiente."); return; }
    est[o].bat -= 10.0f;
    est[d].bat = lim(est[d].bat + 8.0f, 0.0f, BAT_MAX);
    cooldown = 0.5f;
    fluxoO = o;
    fluxoD = d;
    fluxoT = 1.0f;
    tocar(sndApoio);
}

void prosseguirFase(void)
{
    fase++;
    introJogo = true;
    prepararFase();
    tela = TELA_TROCA;
}

void atualizarJogo(float dt)
{
    float vf = velFase();
    Vector2 m = mouseVirtual();

    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { arrasta = -1; tela = TELA_PAUSA; return; }

    int qtd = regioesNaFase(fase);
    for (int k = 0; k < qtd; k++)
        if (clicou(linhaReg(k))) selecionada = ordemFase[k];

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int s = regiaoSob(m);
        if (s >= 0) { selecionada = s; arrasta = s; }
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && arrasta >= 0) {
        int d = regiaoSob(m);
        if (d >= 0 && d != arrasta) transferir(arrasta, d);
        arrasta = -1;
    }

    Regiao *sel = &r[selecionada];
    float wheel = GetMouseWheelMove();
    if (wheel != 0) sel->ajuste += wheel * 0.05f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) sel->ajuste += 0.5f * dt;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) sel->ajuste -= 0.5f * dt;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && hover(btnMais)) sel->ajuste += 0.5f * dt;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && hover(btnMenos)) sel->ajuste -= 0.5f * dt;
    if (clicou(btnMais)) sel->ajuste += 0.03f;
    if (clicou(btnMenos)) sel->ajuste -= 0.03f;
    if (IsKeyPressed(KEY_R) || clicou(btnReset)) sel->ajuste = 1.0f;
    sel->ajuste = lim(sel->ajuste, 0.2f, 1.5f);

    tempoTotal += dt;
    tempoDia += dt;
    if (cooldown > 0) cooldown -= dt;
    if (msgT > 0) msgT -= dt;
    if (toastT > 0) toastT -= dt;
    if (fluxoT > 0) fluxoT -= dt;

    tickEventos(dt);

    bool tudoOk = true;
    float minT = 999.0f;
    for (int i = 0; i < N; i++) {
        Regiao *g = &r[i];
        if (!g->ativa) continue;
        float prod = g->ajuste * g->multProd;
        float cons = g->consBase * g->multCons;
        g->bat = lim(g->bat + (prod - cons) * 4.0f * vf * dt, 0.0f, BAT_MAX);

        if (g->bat < LIM_BAIXO || g->bat > LIM_ALTO) tudoOk = false;
        avaliarAlerta(i, g->bat, (prod - cons) * 4.0f * vf, &minT);
        if (g->bat <= 0.0f)     { iniciarMinigame(i, true);  return; }
        if (g->bat >= BAT_MAX)  { iniciarMinigame(i, false); return; }
    }
    somContagem(minT, dt);

    if (tudoOk) {
        avancarCombo(dt);
        pontosAcum += dt * 5.0f * fase * combo;
        int inteiro = (int)pontosAcum;
        pontos += inteiro;
        pontosAcum -= inteiro;
    }

    if (tempoDia >= DIA_SEG) {
        tempoDia = 0;
        dia++;
        if (dia > DIAS) { avancarFase(); return; }
        mensagem(TextFormat("Dia %d de %d", dia, DIAS), "NOVO DIA", 2.5f, 0);
    }
}

void atualizarSecreta(float dt)
{
    Vector2 m = mouseVirtual();

    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { arrasta = -1; tela = TELA_PAUSA; return; }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int s = estadoSob(m);
        if (s >= 0) { selEst = s; arrasta = s; }
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && arrasta >= 0) {
        int d = estadoSob(m);
        if (d >= 0 && d != arrasta) transferirE(arrasta, d);
        arrasta = -1;
    }

    Estado *sel = &est[selEst];
    float wheel = GetMouseWheelMove();
    if (wheel != 0) sel->ajuste += wheel * 0.05f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) sel->ajuste += 0.5f * dt;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) sel->ajuste -= 0.5f * dt;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && hover(btnMais)) sel->ajuste += 0.5f * dt;
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && hover(btnMenos)) sel->ajuste -= 0.5f * dt;
    if (clicou(btnMais)) sel->ajuste += 0.03f;
    if (clicou(btnMenos)) sel->ajuste -= 0.03f;
    if (IsKeyPressed(KEY_R) || clicou(btnReset)) sel->ajuste = 1.0f;
    sel->ajuste = lim(sel->ajuste, 0.2f, 1.5f);

    tempoTotal += dt;
    tempoDia += dt;
    if (cooldown > 0) cooldown -= dt;
    if (msgT > 0) msgT -= dt;
    if (toastT > 0) toastT -= dt;
    if (fluxoT > 0) fluxoT -= dt;

    tickEventos(dt);

    float vf = velFase();
    bool tudoOk = true;
    float minT = 999.0f;
    for (int i = 0; i < NE; i++) {
        Estado *e = &est[i];
        float prod = e->ajuste * r[e->reg].multProd;
        float cons = e->cons * r[e->reg].multCons;
        e->bat = lim(e->bat + (prod - cons) * 4.0f * vf * dt, 0.0f, BAT_MAX);
        if (e->bat < LIM_BAIXO || e->bat > LIM_ALTO) tudoOk = false;
        avaliarAlerta(i, e->bat, (prod - cons) * 4.0f * vf, &minT);
        if (e->bat <= 0.0f)    { iniciarMinigame(i, true);  return; }
        if (e->bat >= BAT_MAX) { iniciarMinigame(i, false); return; }
    }
    somContagem(minT, dt);

    if (tudoOk) {
        avancarCombo(dt);
        pontosAcum += dt * 20.0f * combo;
        int inteiro = (int)pontosAcum;
        pontos += inteiro;
        pontosAcum -= inteiro;
    }

    if (tempoDia >= SEC_DIA_SEG) {
        tempoDia = 0;
        dia++;
        if (dia > SEC_DIAS) {
            pontos += 5000;
            secretaVenceu = true;
            salvarSegredo();
            fimDePartida(true);
            return;
        }
        mensagem(TextFormat("Dia %d de %d", dia, SEC_DIAS), "NOVO DIA", 2.5f, 0);
    }
}
