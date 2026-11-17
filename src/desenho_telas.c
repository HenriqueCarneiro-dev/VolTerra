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

static void desenharMenu(void)
{
    float t = (float)GetTime();

    for (int i = 0; i < 46; i++) {
        float seed = i * 37.1f;
        float x = fmodf(seed * 13.7f, (float)W);
        float vel = 18.0f + (i % 7) * 7.0f;
        float y = H + 20.0f - fmodf(t * vel + seed * 9.3f, H + 40.0f);
        float tam = 1.5f + (i % 3);
        float piscar = 0.35f + 0.35f * sinf(t * 3.0f + i);
        DrawCircle((int)(x + sinf(t + i) * 8.0f), (int)y, tam, Fade((Color){255, 220, 90, 255}, piscar));
    }

    float pulso = 0.5f + 0.5f * sinf(t * 2.0f);
    textoCentro("VOLTERRA", 643, 103, 92, Fade(BLACK, 0.6f));
    textoCentro("VOLTERRA", 640, 100, 92, Fade((Color){255, 150, 30, 255}, 0.25f + 0.25f * pulso));
    textoCentro("VOLTERRA", 640, 98, 92, (Color){255, 215, 60, 255});
    textoCentro("VOLTERRA", 640, 100, 92, (Color){255, 215, 60, 255});

    DrawRectangle(440, 198, 400, 3, Fade((Color){60, 90, 130, 255}, 0.8f));
    float hx = 440.0f + fmodf(t * 220.0f, 400.0f);
    DrawRectangleGradientH((int)fmaxf(440.0f, hx - 60), 198, 60, 3, Fade(YELLOW, 0.0f), YELLOW);

    textoCentro("Gerencie a energia do Brasil", 640, 210, 26, RAYWHITE);
    textoCentro("Evite blackouts. Evite sobrecargas. Mantenha o pais ligado.", 640, 242, 16, LIGHTGRAY);

    caixa((Rectangle){470, 268, 340, 322}, 0.06f, (Color){12, 22, 42, 235},
          Fade((Color){255, 215, 60, 255}, 0.55f + 0.2f * pulso));
    desenharBotao(btnNovo, "Novo Jogo", (Color){40, 140, 90, 255});
    desenharBotao(btnFases, "Escolher fase", (Color){120, 90, 170, 255});
    desenharBotao(btnRanque, "Ranque", (Color){190, 140, 30, 255});
    desenharBotao(btnComo, "Como jogar", (Color){40, 100, 180, 255});
    desenharBotao(btnSair, "Sair", (Color){160, 55, 55, 255});

    if (segredoOk) textoCentro("* Conquista secreta desbloqueada *", 640, 600, 15, (Color){255, 215, 60, 255});

    for (int i = 0; i < 5; i++) icone(i, 640.0f + (i - 2) * 76.0f, 650.0f, t, 1.0f);
    textoCentro("ENTER: novo jogo   |   P ou ESC: pausa e volume durante a partida", 640, 690, 14,
                (Color){120, 140, 170, 255});
}

static void desenharFases(void)
{
    textoCentro("ESCOLHER FASE", 640, 110, 56, (Color){255, 215, 60, 255});
    textoCentro("A partida comeca na fase escolhida, com 3 vidas e 0 pontos.", 640, 200, 20, LIGHTGRAY);
    for (int i = 0; i < FASES; i++) {
        Rectangle rc = btnFase(i);
        bool h = hover(rc);
        caixa(rc, 0.15f, h ? (Color){50, 80, 130, 255} : (Color){28, 44, 76, 255}, h ? WHITE : (Color){90, 120, 170, 255});
        textoCentro(TextFormat("%d", i + 1), (int)(rc.x + rc.width / 2), (int)rc.y + 12, 48, WHITE);
        textoCentro(TextFormat("%d regioes", regioesNaFase(i + 1)), (int)(rc.x + rc.width / 2), (int)rc.y + 70, 16, LIGHTGRAY);
    }
    desenharBotao(btnVoltarFas, "Voltar", (Color){160, 55, 55, 255});
}

static void desenharAjuda(void)
{
    textoCentro("COMO JOGAR", 640, 40, 44, (Color){255, 215, 60, 255});
    const char *tit[3] = {"1. Objetivo", "2. Controles", "3. Riscos e minigames"};
    const char *txt[3][7] = {
        {"Cada regiao tem uma bateria de 0% a 110%. Mantenha todas entre 30% e 80%.",
         "Acima de 80%: ALERTA de sobrecarga. Ao chegar em 110%, um minigame dispara.",
         "Abaixo de 30%: ALERTA de blackout. Ao chegar em 0%, um minigame dispara.",
         "Um cronometro no mapa mostra quantos segundos faltam para o estouro.",
         "Setas verdes no mapa indicam sobra de energia; vermelhas indicam falta.",
         "Sobreviva as 5 fases (4 dias de 30s). Cada fase comeca com uma pausa.",
         "Sem alertas, o bonus de pontos sobe de x1 ate x5. Qualquer alerta zera o bonus."},
        {"Clique numa regiao (ou na lista da direita) para seleciona-la.",
         "Ajuste a producao com W/S, roda do mouse ou botoes + e -. R (ou Reset) volta a 100%.",
         "Reduza a producao quando a bateria estiver alta; aumente quando estiver baixa.",
         "Para APOIAR outra regiao: segure o botao numa regiao e arraste ate a outra.",
         "O apoio tira 10% da origem: entrega 8% a vizinha ou 5% a uma regiao distante.",
         "Pausa: tecla P ou ESC. No menu de pausa voce ajusta o volume da musica e dos efeitos.",
         "Regioes novas liberam na fase 2 (4 regioes) e na fase 3 (5 regioes)."},
        {"Ao atingir 0% ou 110%, voce precisa vencer um minigame para estabilizar.",
         "Blackout: ligar os fios, religar disjuntores 1 a 6, ou girar o gerador (cliques).",
         "Sobrecarga: esfriar turbinas, parar o marcador, desligar os alvos, ou desviar das faiscas.",
         "Vencer estabiliza a regiao e da pontos. Falhar custa um coracao.",
         "Sem coracoes, o jogo acaba. Voce comeca com 3.",
         "No fim da partida, digite seu nome: a pontuacao entra no Ranque da fase.",
         "Dizem que quem nao perde nenhuma vida descobre algo especial..."}
    };
    caixa((Rectangle){190, 110, 900, 500}, 0.03f, (Color){14, 26, 48, 245}, (Color){60, 90, 130, 255});
    textoCentro(tit[paginaAjuda], 640, 135, 30, (Color){120, 200, 255, 255});
    for (int i = 0; i < 7; i++) DrawText(txt[paginaAjuda][i], 215, 200 + i * 50, 19, RAYWHITE);
    textoCentro(TextFormat("Pagina %d/3", paginaAjuda + 1), 640, 590, 16, LIGHTGRAY);
    if (paginaAjuda > 0) desenharBotao(btnAnt, "Anterior", (Color){60, 80, 130, 255});
    if (paginaAjuda < 2) desenharBotao(btnProx, "Proxima", (Color){60, 80, 130, 255});
    desenharBotao(btnVoltar, "Menu", (Color){160, 55, 55, 255});
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
        case MINI_FIOS:      ins = "Arraste cada fio da esquerda ate a ponta da mesma cor, a direita."; break;
        case MINI_DISJUNTOR: ins = "Clique nos disjuntores em ordem: 1, 2, 3, 4, 5, 6. Erro reinicia a sequencia."; break;
        case MINI_GERADOR:   ins = "Gire o gerador: clique no botao ou aperte ESPACO rapido ate encher 100%."; break;
        case MINI_TURBINAS:  ins = "Clique nas turbinas para esfria-las. Nao deixe nenhuma chegar a 100%!"; break;
        case MINI_SINCRONIA: ins = "Pare o marcador na zona verde (ESPACO ou botao). Acerte 3 vezes."; break;
        case MINI_FAISCAS:   ins = "Desvie das faiscas com o mouse ate o tempo acabar. 3 toques = falha."; break;
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
        case MINI_FIOS:
            DrawText("Origem", 400, 262, 18, LIGHTGRAY);
            DrawText("Destino", 810, 262, 18, LIGHTGRAY);
            for (int i = 0; i < 4; i++) {
                if (mini.destino[i] >= 0)
                    DrawLineEx(noEsq(i), noDir(mini.destino[i]), 7.0f, fios[mini.ordem[i]]);
            }
            if (mini.arrasta >= 0) DrawLineEx(noEsq(mini.arrasta), m, 7.0f, fios[mini.ordem[mini.arrasta]]);
            for (int i = 0; i < 4; i++) {
                DrawCircle((int)noEsq(i).x, (int)noEsq(i).y, 22, WHITE);
                DrawCircle((int)noEsq(i).x, (int)noEsq(i).y, 18, fios[mini.ordem[i]]);
                DrawCircle((int)noDir(i).x, (int)noDir(i).y, 22, WHITE);
                DrawCircle((int)noDir(i).x, (int)noDir(i).y, 18, fios[i]);
            }
            if (mini.flash > 0) DrawText("Cor errada! -2s", 560, 510, 20, (Color){255, 100, 100, 255});
            break;
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
        case MINI_GERADOR:
            textoCentro(TextFormat("Carga: %.0f%%", mini.carga), 640, 300, 28, WHITE);
            caixa(barraSinc, 0.2f, (Color){12, 20, 34, 255}, (Color){90, 120, 170, 255});
            DrawRectangle((int)barraSinc.x + 4, (int)barraSinc.y + 4,
                          (int)((barraSinc.width - 8) * mini.carga / 100.0f), (int)barraSinc.height - 8,
                          mini.carga < 35 ? (Color){255, 90, 90, 255} : (Color){255, 215, 60, 255});
            desenharBotao(btnSinc, "GIRAR!", (Color){40, 100, 180, 255});
            break;
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
        DrawRectangleRoundedLines((Rectangle){btnSecreta.x - 6, btnSecreta.y - 6, btnSecreta.width + 12, btnSecreta.height + 12},
                                  0.3f, 8, Fade((Color){255, 215, 60, 255}, 0.4f + 0.6f * p));
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

    DrawRectangleRounded(
        rc,
        0.25f, 8,
        h ? (Color){55, 75, 105, 255} : (Color){25, 38, 60, 245}
    );

    DrawRectangleRoundedLines(
        rc, 0.25f, 8,
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
            desenharFundoMaritimo();
            desenharMapa();
            DrawRectangle(0, 0, W, H, Fade(BLACK, 0.55f));
            desenharMenu();
            break;

        case TELA_AJUDA:
            desenharFundoMaritimo();
            desenharAjuda();
            break;

        case TELA_FASES:
            desenharFundoMaritimo();
            desenharFases();
            break;

        case TELA_RANK:
            desenharFundoMaritimo();
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
