#include "tipos.h"
#include "globais.h"
#include "dados.h"
#include "audio.h"
#include "util.h"
#include "eventos.h"

bool linhaEfeito(char *buf, int sz, int id, int i, float *net)
{
    float k = intensidade();
    float p = evDefs[id].prod[i] * k, c = evDefs[id].cons[i] * k;
    *net = p - c;
    if (fabsf(p) < 0.01f && fabsf(c) < 0.01f) return false;
    int n = snprintf(buf, sz, "%s:", abrev[i]);
    if (fabsf(p) >= 0.01f) n += snprintf(buf + n, sz - n, " prod %+d%%", (int)roundf(p * 100));
    if (fabsf(c) >= 0.01f) n += snprintf(buf + n, sz - n, " cons %+d%%", (int)roundf(c * 100));
    return true;
}

static void spawnEvento(void)
{
    int id = -1;
    for (int tent = 0; tent < 40 && id < 0; tent++) {
        int c = GetRandomValue(0, NUM_EV - 1);
        bool rep = false;
        for (int i = 0; i < nev; i++) if (evs[i].id == c) rep = true;
        if (rep) continue;
        float rel = 0.0f;
        for (int i = 0; i < N; i++) {
            if (!r[i].ativa) continue;
            rel += fabsf(evDefs[c].prod[i]) + fabsf(evDefs[c].cons[i]);
        }
        if (rel > 0.05f) id = c;
    }
    if (id < 0) { spawnT = 4.0f; return; }

    evs[nev].id = id;
    evs[nev].dur = 25.0f;
    evs[nev].t = 25.0f;
    nev++;
    tocar(sndEvento);

    char resumo[200] = "";
    int cnt = 0;
    for (int i = 0; i < N && cnt < 3; i++) {
        char lin[80];
        float net;
        if (!r[i].ativa || !linhaEfeito(lin, sizeof(lin), id, i, &net)) continue;
        if (cnt) strncat(resumo, "  |  ", sizeof(resumo) - strlen(resumo) - 1);
        strncat(resumo, lin, sizeof(resumo) - strlen(resumo) - 1);
        cnt++;
    }
    mensagem(resumo, evDefs[id].nome, 5.0f, 1);
    if (secreta) spawnT = 22.0f + GetRandomValue(0, 50) / 10.0f;
    else spawnT = 24.0f - 2.5f * (fase - 1) + GetRandomValue(0, 40) / 10.0f;
}

static void atualizarMultiplicadores(void)
{
    float intens = intensidade();
    for (int i = 0; i < N; i++) {
        float mp = 0.92f, mc = 0.91f;
        for (int e = 0; e < nev; e++) {
            mp += evDefs[evs[e].id].prod[i] * intens;
            mc += evDefs[evs[e].id].cons[i] * intens;
        }
        mc *= 1.0f + 0.025f * sinf(tempoTotal * 0.35f + i * 1.9f);
        if (mp < 0.25f) mp = 0.25f;
        if (mc < 0.35f) mc = 0.35f;
        r[i].multProd = mp;
        r[i].multCons = mc;
    }
}

void tickEventos(float dt)
{
    for (int e = 0; e < nev; ) {
        evs[e].t -= dt;
        if (evs[e].t <= 0) { evs[e] = evs[nev - 1]; nev--; }
        else e++;
    }
    spawnT -= dt;
    if (spawnT <= 0) {
        if (nev < maxEventos()) spawnEvento();
        else spawnT = 3.0f;
    }
    atualizarMultiplicadores();
}
