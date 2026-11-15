#include "tipos.h"
#include "globais.h"
#include "audio.h"
#include "ranking.h"

int inserirRanking(int idx, const char *nome, int p)
{
    if (idx < 0 || idx >= NUM_RANK) return 0;
    No *novo = (No *)malloc(sizeof(No));
    if (!novo) return 0;
    snprintf(novo->nome, sizeof(novo->nome), "%s", nome);
    novo->pontos = p;
    novo->prox = NULL;

    No **pp = &rankingLista[idx];
    int pos = 1;
    while (*pp && (*pp)->pontos >= p) { pp = &(*pp)->prox; pos++; }
    novo->prox = *pp;
    *pp = novo;

    int cnt = 0;
    No *a = rankingLista[idx];
    while (a) {
        cnt++;
        if (cnt == MAX_RANK) {
            No *x = a->prox;
            a->prox = NULL;
            while (x) { No *t = x->prox; free(x); x = t; }
            break;
        }
        a = a->prox;
    }
    return pos <= MAX_RANK ? pos : 0;
}

void liberarRanking(void)
{
    for (int i = 0; i < NUM_RANK; i++) {
        No *a = rankingLista[i];
        while (a) { No *t = a->prox; free(a); a = t; }
        rankingLista[i] = NULL;
    }
}

void carregarRanking(void)
{
    FILE *f = fopen("ranking_fases.txt", "r");
    if (!f) return;
    char nome[16];
    int idx, p;
    while (fscanf(f, "%d %15s %d", &idx, nome, &p) == 3) inserirRanking(idx, nome, p);
    fclose(f);
}

void salvarRanking(void)
{
    FILE *f = fopen("ranking_fases.txt", "w");
    if (!f) return;
    for (int i = 0; i < NUM_RANK; i++)
        for (No *a = rankingLista[i]; a; a = a->prox) fprintf(f, "%d %s %d\n", i, a->nome, a->pontos);
    fclose(f);
}

void carregarSegredo(void)
{
    FILE *f = fopen("segredo.txt", "r");
    if (!f) return;
    segredoOk = true;
    fclose(f);
}

void salvarSegredo(void)
{
    segredoOk = true;
    FILE *f = fopen("segredo.txt", "w");
    if (!f) return;
    fprintf(f, "1\n");
    fclose(f);
}

void fimDePartida(bool venceu)
{
    vitoria = venceu;
    arrasta = -1;
    posRanking = 0;
    nomeLen = 0;
    nomeJog[0] = '\0';
    rankAtual = secreta ? (NUM_RANK - 1) : (fase - 1);
    tocar(venceu ? sndOk : sndFalha);
    tela = (pontos > 0) ? TELA_NOME : TELA_FIM;
}
