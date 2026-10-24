#include <stdio.h>
#include <stdlib.h>

#include "jogo.h"

// Ordem em que as regiões entram no jogo (noite 1 começa com as 3 primeiras).
static const struct {
    const char *nome;
    int consumo;
} CONFIG[NUM_REGIOES] = {
    {"Sudeste",      50},
    {"Nordeste",     35},
    {"Sul",          30},
    {"Centro-Oeste", 20},
    {"Norte",        25}
};

static void jogoMensagem(EstadoJogo *j, const char *fmt, const char *nome)
{
    snprintf(j->mensagem, sizeof(j->mensagem), fmt, nome);
}

static int jogoCabeNaCota(const EstadoJogo *j, const Regiao *r)
{
    return jogoEnergiaUsada(j) + r->consumo <= j->producao;
}

/** Configura a noite n: cota, regiões ativas e tempo de alerta. */
static void jogoIniciarNoite(EstadoJogo *j, int n)
{
    int ativas = n + 2;
    if (ativas > NUM_REGIOES) ativas = NUM_REGIOES;

    j->noite = n;
    j->segundosNoite = 0;
    j->producao = 90 + 10 * (n - 1);

    for (int i = 0; i < ativas; i++)
    {
        Regiao *r = &j->regioes[i];
        if (r->estado != REGIAO_INATIVA) continue;

        regiaoAtivar(r, jogoTempoAlerta(j));
        if (jogoCabeNaCota(j, r)) regiaoLigar(r);

        if (n > 1) jogoMensagem(j, "Nova regiao conectada: %s", r->nome);
    }
}

EstadoJogo *jogoCriar(void)
{
    EstadoJogo *j = calloc(1, sizeof(EstadoJogo));
    if (j == NULL) return NULL;

    j->regioes = malloc(NUM_REGIOES * sizeof(Regiao));
    if (j->regioes == NULL)
    {
        free(j);
        return NULL;
    }

    j->numRegioes = NUM_REGIOES;
    for (int i = 0; i < NUM_REGIOES; i++)
        regiaoInit(&j->regioes[i], CONFIG[i].nome, CONFIG[i].consumo);

    j->fase = JOGO_RODANDO;
    jogoIniciarNoite(j, 1);
    snprintf(j->mensagem, sizeof(j->mensagem),
             "Noite 1: a cota nao cobre todas as regioes!");
    return j;
}

void jogoDestruir(EstadoJogo *j)
{
    if (j == NULL) return;
    free(j->regioes);
    free(j);
}

int jogoEnergiaUsada(const EstadoJogo *j)
{
    int total = 0;
    for (int i = 0; i < j->numRegioes; i++)
        if (j->regioes[i].ligada) total += j->regioes[i].consumo;
    return total;
}

int jogoTempoAlerta(const EstadoJogo *j)
{
    int t = 16 - 2 * j->noite;
    return t < 6 ? 6 : t;
}

void jogoMover(EstadoJogo *j, int delta)
{
    int idx = j->selecionada;

    for (int tentativas = 0; tentativas < j->numRegioes; tentativas++)
    {
        idx = (idx + delta + j->numRegioes) % j->numRegioes;
        if (j->regioes[idx].estado != REGIAO_INATIVA)
        {
            j->selecionada = idx;
            return;
        }
    }
}

void jogoSelecionar(EstadoJogo *j, int idx)
{
    if (idx < 0 || idx >= j->numRegioes) return;
    if (j->regioes[idx].estado == REGIAO_INATIVA) return;
    j->selecionada = idx;
}

void jogoAlternar(EstadoJogo *j)
{
    Regiao *r = &j->regioes[j->selecionada];

    if (r->estado == REGIAO_INATIVA) return;

    if (r->ligada)
    {
        regiaoDesligar(r, jogoTempoAlerta(j));
        jogoMensagem(j, "%s desligada: contagem iniciada", r->nome);
    }
    else if (jogoCabeNaCota(j, r))
    {
        int estavaApagada = (r->estado == REGIAO_BLACKOUT);
        regiaoLigar(r);
        jogoMensagem(j, estavaApagada ? "%s restabelecida" : "%s ligada",
                     r->nome);
    }
    else
    {
        jogoMensagem(j, "Sem cota para %s! Desligue outra regiao", r->nome);
    }
}

void jogoAtualizarSegundo(EstadoJogo *j)
{
    if (j->fase != JOGO_RODANDO) return;

    for (int i = 0; i < j->numRegioes; i++)
    {
        Regiao *r = &j->regioes[i];

        if (regiaoAtualizarSegundo(r))
        {
            j->blackouts++;
            j->pontos -= 50;
            if (j->pontos < 0) j->pontos = 0;
            jogoMensagem(j, "BLACKOUT em %s!", r->nome);
        }
        else if (r->ligada)
        {
            j->pontos += 10;
        }
    }

    if (j->blackouts >= MAX_BLACKOUTS)
    {
        j->fase = JOGO_DERROTA;
        return;
    }

    j->segundosNoite++;
    if (j->segundosNoite >= DURACAO_NOITE)
    {
        j->pontos += 100 * j->noite;

        if (j->noite >= MAX_NOITES)
            j->fase = JOGO_VITORIA;
        else
            jogoIniciarNoite(j, j->noite + 1);
    }
}
