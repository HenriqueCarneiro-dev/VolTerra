#include "tipos.h"
#include "globais.h"
#include "dados.h"
#include "ui.h"
#include "util.h"
#include "audio.h"

void tocar(Sound s)
{
    if (!audioOk) return;
    SetSoundVolume(s, volSfx);
    PlaySound(s);
}

static float freqMidi(int m)
{
    return 440.0f * powf(2.0f, (m - 69) / 12.0f);
}

static Sound somDeBuffer(short *buf, int frames)
{
    Wave w;
    memset(&w, 0, sizeof(w));
    w.frameCount = (unsigned int)frames;
    w.sampleRate = SR;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = buf;
    Sound s = LoadSoundFromWave(w);
    free(buf);
    return s;
}

static float sn(double ph)
{
    return sinf((float)fmod(ph, 6.283185307179586));
}

static unsigned lcg(unsigned *s)
{
    *s = *s * 1664525u + 1013904223u;
    return (*s >> 16) & 0x7fff;
}

static void nota(float *b, int frames, double t0, double dur, double f, float vol,
                 float atk, float rel, float dec, float h2, float h3, float hd, float det)
{
    const double tp = 6.283185307179586;
    int n = (int)((dur + rel) * SR);
    long s0 = (long)(t0 * SR);
    for (int i = 0; i < n; i++) {
        double t = (double)i / SR;
        float env = t < atk ? (float)(t / atk) : 1.0f;
        env *= expf((float)(-dec * t));
        if (t > dur) {
            float q = 1.0f - (float)((t - dur) / rel);
            if (q < 0.0f) q = 0.0f;
            env *= q;
        }
        double ph = tp * f * t;
        float hf = expf((float)(-hd * t));
        float w = sn(ph) + h2 * hf * sn(2.0 * ph) + h3 * hf * sn(3.0 * ph);
        if (det > 0.0f) w += 0.7f * sn(tp * f * (1.0 + det) * t);
        b[(s0 + i) % frames] += vol * env * w;
    }
}

static Sound compor(const int *raiz, const int *tipo, int bars, float bpm, int arpDiv,
                    int probMel, unsigned seed, float volMel)
{
    static const int iv[3][4] = {{0, 3, 7, 10}, {0, 4, 7, 11}, {0, 4, 7, 10}};
    static const int ord[6] = {0, 1, 2, 3, 2, 1};

    double beat = 60.0 / bpm;
    double bar = 4.0 * beat;
    int frames = (int)(bars * bar * SR);

    float *dry = (float *)calloc((size_t)frames, sizeof(float));
    float *wet = (float *)calloc((size_t)frames, sizeof(float));
    float *eco = (float *)calloc((size_t)frames, sizeof(float));
    short *out = (short *)malloc(sizeof(short) * (size_t)frames);

    unsigned s = seed;
    int idx = 2;

    for (int b = 0; b < bars; b++) {
        double t0 = b * bar;
        int r0 = raiz[b];
        const int *v = iv[tipo[b]];

        for (int k = 0; k < 4; k++)
            nota(dry, frames, t0, bar, freqMidi(r0 + v[k]), 0.10f, 0.7f, 1.0f, 0.0f, 0.15f, 0.0f, 0.0f, 0.004f);

        nota(dry, frames, t0, 2.6 * beat, freqMidi(r0 - 12), 0.22f, 0.05f, 0.5f, 0.35f, 0.20f, 0.0f, 0.0f, 0.0f);
        nota(dry, frames, t0 + 2.0 * beat, 1.6 * beat, freqMidi(r0 - 12), 0.14f, 0.05f, 0.5f, 0.35f, 0.20f, 0.0f, 0.0f, 0.0f);

        int steps = 4 * arpDiv;
        double step = beat / arpDiv;
        for (int i = 0; i < steps; i++) {
            int m = r0 + v[ord[i % 6]] + 12;
            nota(wet, frames, t0 + i * step, 0.2, freqMidi(m), 0.07f, 0.01f, 1.2f, 3.0f, 0.30f, 0.10f, 4.0f, 0.0f);
        }

        int pool[6] = {r0 + v[0] + 12, r0 + v[1] + 12, r0 + v[2] + 12, r0 + v[3] + 12,
                       r0 + v[0] + 24, r0 + v[1] + 24};
        for (int slot = 0; slot < 8; slot++) {
            if (b % 4 == 3 && slot >= 4) continue;
            if ((int)(lcg(&s) % 100) >= probMel) continue;
            idx += (int)(lcg(&s) % 5) - 2;
            if (idx < 0) idx = 0;
            if (idx > 5) idx = 5;
            nota(wet, frames, t0 + slot * 0.5 * beat, 0.3, freqMidi(pool[idx]), volMel,
                 0.015f, 2.5f, 1.4f, 0.35f, 0.12f, 3.0f, 0.0f);
        }
    }

    int D = (int)(0.75 * beat * SR);
    for (int pass = 0; pass < 2; pass++) {
        for (int i = 0; i < frames; i++) {
            int k = (i - D + frames) % frames;
            eco[i] = wet[i] + 0.45f * eco[k];
        }
    }

    const float a = 0.6f;
    float y = 0.0f;
    int ini = frames > 2000 ? frames - 2000 : 0;
    for (int i = ini; i < frames; i++) y += a * ((dry[i] + eco[i]) - y);

    float pico = 0.0001f;
    for (int i = 0; i < frames; i++) {
        y += a * ((dry[i] + eco[i]) - y);
        dry[i] = y;
        if (fabsf(y) > pico) pico = fabsf(y);
    }
    float g = 0.70f / pico;
    for (int i = 0; i < frames; i++) out[i] = (short)(dry[i] * g * 32000.0f);

    free(dry);
    free(wet);
    free(eco);
    return somDeBuffer(out, frames);
}

static Sound gerarEfeito(float f0, float f1, float dur, int forma, float vol, int pulsos)
{
    int frames = (int)(dur * SR);
    short *buf = (short *)malloc(sizeof(short) * (size_t)frames);
    float ph = 0.0f;
    for (int i = 0; i < frames; i++) {
        float u = (float)i / (float)frames;
        float f = f0 + (f1 - f0) * u;
        ph += 2.0f * 3.14159265f * f / SR;
        float s = sinf(ph);
        if (forma == 1) s = (s > 0 ? 1.0f : -1.0f) * 0.6f;
        float env = u < 0.03f ? u / 0.03f : (1.0f - u);
        if (env > 1.0f) env = 1.0f;
        float gate = 1.0f;
        if (pulsos > 1) {
            float g = fmodf(u * pulsos, 1.0f);
            gate = g < 0.65f ? 1.0f : 0.0f;
        }
        buf[i] = (short)(s * env * gate * vol * 30000.0f);
    }
    return somDeBuffer(buf, frames);
}

static Sound carregarOuGerar(const char *arq, Sound gerado)
{
    if (FileExists(arq)) {
        Sound s = LoadSound(arq);
        if (s.frameCount > 0) { UnloadSound(gerado); return s; }
    }
    return gerado;
}

void iniciarAudio(void)
{
    InitAudioDevice();
    audioOk = IsAudioDeviceReady();
    if (!audioOk) return;

    sndClique = gerarEfeito(700.0f, 950.0f, 0.07f, 0, 0.5f, 1);
    sndBlack  = gerarEfeito(950.0f, 450.0f, 0.90f, 1, 0.35f, 2);
    sndSobre  = gerarEfeito(1500.0f, 1500.0f, 0.70f, 1, 0.30f, 6);
    sndEvento = gerarEfeito(600.0f, 950.0f, 0.40f, 0, 0.5f, 1);
    sndTick   = gerarEfeito(1100.0f, 1100.0f, 0.08f, 0, 0.6f, 1);
    sndOk     = gerarEfeito(500.0f, 1100.0f, 0.55f, 0, 0.5f, 1);
    sndFalha  = gerarEfeito(320.0f, 90.0f, 0.70f, 1, 0.35f, 1);
    sndApoio  = gerarEfeito(500.0f, 1200.0f, 0.25f, 0, 0.45f, 1);

    for (int i = 0; i < NUM_MUS; i++) musPronta[i] = false;
    musAtual = -1;
}

void encerrarAudio(void)
{
    if (!audioOk) return;
    UnloadSound(sndClique); UnloadSound(sndBlack); UnloadSound(sndSobre); UnloadSound(sndEvento);
    UnloadSound(sndTick); UnloadSound(sndOk); UnloadSound(sndFalha); UnloadSound(sndApoio);
    for (int i = 0; i < NUM_MUS; i++)
        if (musPronta[i]) UnloadSound(mus[i]);
    CloseAudioDevice();
}

static void garantirMusica(int i)
{
    if (musPronta[i]) return;
    telaCarregando(i == 0 ? "Preparando a musica do menu..." : "Carregando a musica da fase...");

    const Trilha *tr = &trilhas[i];
    mus[i] = compor(tr->raiz, tr->tipo, 16, tr->bpm, tr->arp, tr->prob, tr->seed, tr->vol);

    char arq[64];
    if (i == 0) snprintf(arq, sizeof(arq), "assets/musica_menu.wav");
    else if (i == NUM_MUS - 1) snprintf(arq, sizeof(arq), "assets/fasesecreta.wav");
    else snprintf(arq, sizeof(arq), "assets/fasesnormal.wav", i);
    mus[i] = carregarOuGerar(arq, mus[i]);
    musPronta[i] = true;
}

void atualizarMusica(void)
{
    if (!audioOk) return;
    int quer = 0;
    switch (tela) {
        case TELA_MENU: case TELA_AJUDA: case TELA_FASES: case TELA_RANK:
        case TELA_NOME: case TELA_FIM:
            quer = 0; break;
        default:
            quer = secreta ? (NUM_MUS - 1) : fase;
            break;
    }
    if (quer < 0) quer = 0;
    if (quer >= NUM_MUS) quer = NUM_MUS - 1;

    garantirMusica(quer);

    if (quer != musAtual) {
        if (musAtual >= 0 && musPronta[musAtual]) StopSound(mus[musAtual]);
        musAtual = quer;
    }

    float v = volMus;
    if (quer != 0) {
        v = volMus * 0.9f;
        if (tela == TELA_PAUSA) v = volMus * 0.30f;
        else if (tela == TELA_MINI) v = volMus * 0.55f;
    }
    SetSoundVolume(mus[quer], v);
    if (!IsSoundPlaying(mus[quer])) PlaySound(mus[quer]);
}

void avaliarAlerta(int i, float bat, float rate, float *minT)
{
    bool lo = bat < LIM_BAIXO, hi = bat > LIM_ALTO;
    bool agora = lo || hi;
    if (agora && !alertaPrev[i]) tocar(lo ? sndBlack : sndSobre);
    alertaPrev[i] = agora;
    float t = tempoCritico(bat, rate);
    if (t >= 0.0f && t < *minT) *minT = t;
}

void somContagem(float minT, float dt)
{
    if (minT <= 10.0f) {
        tickT -= dt;
        if (tickT <= 0.0f) {
            tocar(sndTick);
            tickT = minT < 4.0f ? 0.3f : (minT < 7.0f ? 0.6f : 1.0f);
        }
    } else {
        tickT = 0.0f;
    }
}
