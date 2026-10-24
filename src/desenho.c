#include <stdarg.h>
#include <stdio.h>

#include "screen.h"
#include "desenho.h"

#define LARGURA_LINHA 70
#define BARRA_LARGURA 20

/** Escreve uma linha formatada em (x,y), preenchida com espaços até `largura`. */
static void escreve(int x, int y, screenColor fg, screenColor bg,
                    int largura, const char *fmt, ...)
{
    char buf[128];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    screenSetColor(fg, bg);
    screenGotoxy(x, y);
    printf("%-*s", largura, buf);
}

static void limpaLinha(int y)
{
    escreve(4, y, WHITE, BLACK, LARGURA_LINHA, "");
}

static void desenharBarraEnergia(const EstadoJogo *j, int y)
{
    int usada = jogoEnergiaUsada(j);
    int cheio = j->producao > 0 ? usada * BARRA_LARGURA / j->producao : 0;
    char barra[BARRA_LARGURA + 1];

    if (cheio > BARRA_LARGURA) cheio = BARRA_LARGURA;
    for (int i = 0; i < BARRA_LARGURA; i++)
        barra[i] = (i < cheio) ? '#' : '-';
    barra[BARRA_LARGURA] = '\0';

    escreve(4, y, LIGHTCYAN, BLACK, LARGURA_LINHA,
            "Energia: [%s] %3d / %3d MW", barra, usada, j->producao);
}

static void desenharRegiao(const EstadoJogo *j, int i, int y)
{
    const Regiao *r = &j->regioes[i];
    screenColor fg = WHITE;
    screenColor bg = (i == j->selecionada) ? BLUE : BLACK;
    char status[24];
    char cursor = (i == j->selecionada) ? '>' : ' ';

    switch (r->estado)
    {
        case REGIAO_INATIVA:
            fg = DARKGRAY;
            snprintf(status, sizeof(status), "(bloqueada)");
            break;
        case REGIAO_OK:
            fg = LIGHTGREEN;
            snprintf(status, sizeof(status), "ABASTECIDA");
            break;
        case REGIAO_ALERTA:
            fg = (r->segundosAlerta <= 3) ? LIGHTRED : YELLOW;
            snprintf(status, sizeof(status), "ALERTA %2ds", r->segundosAlerta);
            break;
        case REGIAO_BLACKOUT:
            fg = RED;
            snprintf(status, sizeof(status), "BLACKOUT!");
            break;
    }

    escreve(4, y, fg, bg, LARGURA_LINHA, "%c %d  %-13s %3d MW   %-6s  %s",
            cursor, i + 1, r->nome, r->consumo,
            r->ligada ? "[ ON ]" : "[ OFF]", status);

    if (r->estado == REGIAO_ALERTA && r->segundosAlerta <= 3)
    {
        // Pisca só a linha em perigo: reescreve com o atributo de blink.
        screenSetBlink();
        screenGotoxy(4, y);
        printf("%c %d", cursor, i + 1);
    }
}

void desenharTitulo(void)
{
    for (int y = 2; y <= 22; y++) limpaLinha(y);

    escreve(22, 6, LIGHTCYAN, BLACK, 40, "V O L T E R R A");
    escreve(14, 8, WHITE, BLACK, 56, "CENTRAL DE ENERGIA - gerencie a usina do pais");
    escreve(8, 11, YELLOW, BLACK, 64,
            "Distribua a cota entre as regioes e evite os blackouts.");
    escreve(8, 12, YELLOW, BLACK, 64,
            "Regiao desligada entra em alerta e, no fim da contagem, apaga.");
    escreve(8, 15, LIGHTGREEN, BLACK, 64, "ENTER = comecar     Q = sair");
    screenUpdate();
}

void desenharJogo(const EstadoJogo *j)
{
    escreve(4, 2, LIGHTCYAN, BLACK, LARGURA_LINHA,
            "VOLTERRA - CENTRAL DE ENERGIA");
    escreve(4, 3, WHITE, BLACK, LARGURA_LINHA,
            "Noite %d/%d   Tempo: %2ds   Pontos: %-6d Blackouts: %d/%d",
            j->noite, MAX_NOITES, DURACAO_NOITE - j->segundosNoite,
            j->pontos, j->blackouts, MAX_BLACKOUTS);

    desenharBarraEnergia(j, 5);

    escreve(4, 7, LIGHTGRAY, BLACK, LARGURA_LINHA,
            "  #  REGIAO        CONSUMO   LIGACAO   STATUS");

    for (int i = 0; i < j->numRegioes; i++)
        desenharRegiao(j, i, 8 + i);

    limpaLinha(14);
    escreve(4, 15, LIGHTMAGENTA, BLACK, LARGURA_LINHA, "%s", j->mensagem);

    escreve(4, 20, DARKGRAY, BLACK, LARGURA_LINHA,
            "Setas ou W/S: escolher   ENTER/ESPACO: ligar/desligar");
    escreve(4, 21, DARKGRAY, BLACK, LARGURA_LINHA,
            "1-5: atalho para a regiao   Q: sair");

    screenSetColor(WHITE, BLACK);
}

void desenharFim(const EstadoJogo *j)
{
    for (int y = 2; y <= 22; y++) limpaLinha(y);

    if (j->fase == JOGO_VITORIA)
    {
        escreve(18, 7, LIGHTGREEN, BLACK, 50, "O PAIS SOBREVIVEU A TODAS AS NOITES!");
    }
    else
    {
        escreve(26, 7, LIGHTRED, BLACK, 40, "BLACKOUT GERAL!");
        escreve(16, 9, WHITE, BLACK, 52, "Muitas regioes ficaram no escuro.");
    }

    escreve(26, 12, YELLOW, BLACK, 40, "Pontos: %d", j->pontos);
    escreve(18, 15, LIGHTGRAY, BLACK, 52, "R = jogar de novo     Q = sair");

    screenSetColor(WHITE, BLACK);
    screenUpdate();
}
