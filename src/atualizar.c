#include "tipos.h"
#include "globais.h"
#include "audio.h"
#include "jogo.h"
#include "minigames.h"
#include "ranking.h"
#include "util.h"
#include "atualizar.h"

static void atualizarNome(void)
{
    int c;
    while ((c = GetCharPressed()) > 0) {
        if (c >= 32 && c <= 125 && nomeLen < TAM_NOME) {
            nomeJog[nomeLen++] = (c == 32) ? '_' : (char)c;
            nomeJog[nomeLen] = '\0';
        }
    }
    if (IsKeyPressed(KEY_BACKSPACE) && nomeLen > 0) nomeJog[--nomeLen] = '\0';

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || clicou(btnOkNome)) {
        if (nomeLen == 0) snprintf(nomeJog, sizeof(nomeJog), "Jogador");
        posRanking = inserirRanking(rankAtual, nomeJog, pontos);
        salvarRanking();
        tela = TELA_FIM;
    }
}

static void atualizarVolumes(void)
{
    Vector2 m = mouseVirtual();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (int i = 0; i < 2; i++) {
            Rectangle b = barraVol(i);
            Rectangle z = {b.x - 14, b.y - 14, b.width + 28, b.height + 28};
            if (CheckCollisionPointRec(m, z)) arrastaVol = i;
        }
    }

    if (arrastaVol >= 0) {
        Rectangle b = barraVol(arrastaVol);
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            float v = lim((m.x - b.x) / b.width, 0.0f, 1.0f);
            if (v < 0.02f) v = 0.0f;
            if (v > 0.98f) v = 1.0f;
            if (arrastaVol == 0) volMus = v;
            else volSfx = v;
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (arrastaVol == 1) tocar(sndClique);
            arrastaVol = -1;
        }
    }
}

void atualizar(float dt)
{
    if (clicou(btnEngrenagem)) {
        painelConfigAberto = !painelConfigAberto;
        arrastaVol = -1;
        return;
    }

    if (painelConfigAberto) {
        atualizarVolumes();

        if (arrastaVol >= 0) return;

        if (clicou(btnModoTela)) {
            ToggleFullscreen();
            telaCheia = !telaCheia;
            return;
        }

        if (clicou(btnFecharConfig) ||
            IsKeyPressed(KEY_ESCAPE) ||
            IsKeyPressed(KEY_ENTER)) {
            painelConfigAberto = false;
            return;
        }

        return;
    }
    
    switch (tela) {
        case TELA_MENU:
            if (clicou(btnNovo) || IsKeyPressed(KEY_ENTER)) novoJogo();
            else if (clicou(btnFases)) tela = TELA_FASES;
            else if (clicou(btnRanque)) { abaRank = 0; tela = TELA_RANK; }
            else if (clicou(btnComo)) { paginaAjuda = 0; tela = TELA_AJUDA; }
            else if (clicou(btnSair)) sair = true;
            break;
        case TELA_AJUDA:
            if (paginaAjuda > 0 && clicou(btnAnt)) paginaAjuda--;
            if (paginaAjuda < 2 && clicou(btnProx)) paginaAjuda++;
            if (clicou(btnVoltar) || IsKeyPressed(KEY_ESCAPE)) tela = TELA_MENU;
            break;
        case TELA_FASES:
            for (int i = 0; i < FASES; i++)
                if (clicou(btnFase(i))) { novoJogoFase(i + 1); return; }
            if (clicou(btnVoltarFas) || IsKeyPressed(KEY_ESCAPE)) tela = TELA_MENU;
            break;
        case TELA_RANK: {
            int nAbas = abasVisiveis();
            for (int i = 0; i < nAbas; i++)
                if (clicou(abaRect(i, nAbas))) abaRank = i;
            if (IsKeyPressed(KEY_RIGHT) && abaRank < nAbas - 1) abaRank++;
            if (IsKeyPressed(KEY_LEFT) && abaRank > 0) abaRank--;
            if (clicou(btnVoltar) || IsKeyPressed(KEY_ESCAPE)) tela = TELA_MENU;
            break;
        }
        case TELA_JOGO:
            if (secreta) atualizarSecreta(dt);
            else atualizarJogo(dt);
            break;
        case TELA_PAUSA:
            atualizarVolumes();
            if (arrastaVol >= 0) break;
            if (clicou(btnContinuar) || IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) tela = TELA_JOGO;
            else if (clicou(btnNovoPausa)) novoJogo();
            else if (clicou(btnMenuPausa)) voltarMenu();
            break;
        case TELA_TROCA:
            if (clicou(btnOkTroca) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) tela = TELA_JOGO;
            break;
        case TELA_MINI:
            atualizarMini(dt);
            break;
        case TELA_NOME:
            atualizarNome();
            break;
        case TELA_FIM:
            if (podeSecreta && clicou(btnSecreta)) { iniciarSecreta(); return; }
            if (clicou(btnFimNovo)) novoJogo();
            else if (clicou(btnFimMenu) || IsKeyPressed(KEY_ENTER)) voltarMenu();
            break;
    }
}
