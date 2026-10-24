#include <stdio.h>
#include <unistd.h>

#include "screen.h"
#include "keyboard.h"
#include "timer.h"

#include "jogo.h"
#include "desenho.h"

#define TICK_MS            100   // duração de um tick do loop
#define TICKS_POR_SEGUNDO  10

// Códigos extras para as setas (valores fora do alcance de um char).
#define TECLA_CIMA   1000
#define TECLA_BAIXO  1001

/**
 * Lê uma tecla sem bloquear. Setas chegam como ESC '[' 'A'/'B'.
 * Retorna 0 se não houver tecla.
 */
static int lerTecla(void)
{
    if (!keyhit()) return 0;

    int ch = readch();

    if (ch == 27 && keyhit())
    {
        int seg = readch();
        if (seg == '[' && keyhit())
        {
            int cod = readch();
            if (cod == 'A') return TECLA_CIMA;
            if (cod == 'B') return TECLA_BAIXO;
        }
        return 0;
    }
    return ch;
}

static void tratarTecla(EstadoJogo *j, int tecla)
{
    switch (tecla)
    {
        case TECLA_CIMA:
        case 'w': case 'W':
            jogoMover(j, -1);
            break;
        case TECLA_BAIXO:
        case 's': case 'S':
            jogoMover(j, +1);
            break;
        case 10: case 13: case ' ':
            jogoAlternar(j);
            break;
        case '1': case '2': case '3': case '4': case '5':
            jogoSelecionar(j, tecla - '1');
            jogoAlternar(j);
            break;
        case 'q': case 'Q':
            j->fase = JOGO_SAIR;
            break;
        default:
            break;
    }
}

/** Espera ENTER (retorna 1) ou Q (retorna 0). */
static int esperarTitulo(void)
{
    for (;;)
    {
        int t = lerTecla();
        if (t == 10 || t == 13) return 1;
        if (t == 'q' || t == 'Q') return 0;
        usleep(10000);
    }
}

/** Roda uma partida até vitória, derrota ou saída. */
static void jogar(EstadoJogo *j)
{
    int ticks = 0;

    desenharJogo(j);
    screenUpdate();

    while (j->fase == JOGO_RODANDO)
    {
        int redesenhar = 0;

        int tecla = lerTecla();
        if (tecla != 0)
        {
            tratarTecla(j, tecla);
            redesenhar = 1;
        }

        if (timerTimeOver() == 1)
        {
            ticks++;
            if (ticks % TICKS_POR_SEGUNDO == 0)
            {
                jogoAtualizarSegundo(j);
                redesenhar = 1;
            }
        }

        if (redesenhar && j->fase == JOGO_RODANDO)
        {
            desenharJogo(j);
            screenUpdate();
        }

        usleep(10000); // evita ocupar 100% da CPU
    }
}

/** Mostra a tela final. Retorna 1 se o jogador quer jogar de novo. */
static int telaFinal(const EstadoJogo *j)
{
    desenharFim(j);

    for (;;)
    {
        int t = lerTecla();
        if (t == 'r' || t == 'R') return 1;
        if (t == 'q' || t == 'Q') return 0;
        usleep(10000);
    }
}

int main(void)
{
    int jogarDeNovo;

    screenInit(1);
    keyboardInit();
    timerInit(TICK_MS);

    // A cli-lib não inicializa peekCharacter com -1: o primeiro keyhit()
    // devolve uma tecla fantasma (0). Esvaziamos o buffer antes de começar.
    while (keyhit()) readch();

    desenharTitulo();
    if (!esperarTitulo())
    {
        keyboardDestroy();
        screenDestroy();
        timerDestroy();
        return 0;
    }

    do
    {
        EstadoJogo *j = jogoCriar();
        if (j == NULL)
        {
            keyboardDestroy();
            screenDestroy();
            timerDestroy();
            fprintf(stderr, "Sem memoria para iniciar o jogo.\n");
            return 1;
        }

        jogar(j);

        jogarDeNovo = 0;
        if (j->fase != JOGO_SAIR)
            jogarDeNovo = telaFinal(j);

        jogoDestruir(j);
    } while (jogarDeNovo);

    keyboardDestroy();
    screenDestroy();
    timerDestroy();

    return 0;
}
