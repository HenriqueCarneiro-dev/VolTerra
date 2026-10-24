#include <string.h>

#include "regiao.h"

void regiaoInit(Regiao *r, const char *nome, int consumo)
{
    strncpy(r->nome, nome, sizeof(r->nome) - 1);
    r->nome[sizeof(r->nome) - 1] = '\0';
    r->consumo = consumo;
    r->ligada = 0;
    r->estado = REGIAO_INATIVA;
    r->segundosAlerta = 0;
}

void regiaoAtivar(Regiao *r, int tempoAlerta)
{
    if (r->estado != REGIAO_INATIVA) return;

    r->ligada = 0;
    r->estado = REGIAO_ALERTA;
    r->segundosAlerta = tempoAlerta;
}

void regiaoLigar(Regiao *r)
{
    if (r->estado == REGIAO_INATIVA) return;

    r->ligada = 1;
    r->estado = REGIAO_OK;
    r->segundosAlerta = 0;
}

void regiaoDesligar(Regiao *r, int tempoAlerta)
{
    if (r->estado == REGIAO_INATIVA || r->estado == REGIAO_BLACKOUT) return;

    r->ligada = 0;
    r->estado = REGIAO_ALERTA;
    r->segundosAlerta = tempoAlerta;
}

int regiaoAtualizarSegundo(Regiao *r)
{
    if (r->estado != REGIAO_ALERTA) return 0;

    r->segundosAlerta--;
    if (r->segundosAlerta <= 0)
    {
        r->segundosAlerta = 0;
        r->estado = REGIAO_BLACKOUT;
        return 1;
    }
    return 0;
}
