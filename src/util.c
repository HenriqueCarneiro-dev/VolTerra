#include "tipos.h"
#include "globais.h"
#include "audio.h"
#include "util.h"

Rectangle btnFase(int i) { return (Rectangle){250.0f + i * 160.0f, 300, 140, 100}; }

Rectangle abaRect(int i, int n)
{
    float total = n * 130.0f - 10.0f;
    return (Rectangle){640.0f - total / 2.0f + i * 130.0f, 150, 120, 44};
}

int abasVisiveis(void)
{
    return rankingLista[NUM_RANK - 1] ? NUM_RANK : NUM_RANK - 1;
}

Rectangle barraVol(int i) { return (Rectangle){480, 548.0f + i * 50.0f, 300, 14}; }

float projX(float lon) { return 285.0f + (lon + 74.0f) * 15.8f; }

float projY(float lat) { return 90.0f + (5.5f - lat) * 15.8f; }

float velFase(void)       { return secreta ? 0.5f : 0.75f + 0.15f * (fase - 1); }

float intensidade(void)   { return secreta ? 0.8f : 1.0f + 0.08f * (fase - 1); }

int regioesNaFase(int f)  { return f <= 1 ? 3 : (f == 2 ? 4 : 5); }

int maxEventos(void)      { return secreta ? 2 : 1 + fase / 2; }

int diasFase(void)        { return secreta ? SEC_DIAS : DIAS; }

float diaDur(void)        { return secreta ? SEC_DIA_SEG : DIA_SEG; }

float lim(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

float dist(Vector2 a, Vector2 b) { return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y)); }

Color misturar(Color a, Color b, float t)
{
    return (Color){(unsigned char)(a.r + (b.r - a.r) * t), (unsigned char)(a.g + (b.g - a.g) * t),
                   (unsigned char)(a.b + (b.b - a.b) * t), 255};
}

Vector2 mouseVirtual(void)
{
    Vector2 m = GetMousePosition();

    float escalaX = viewport.width / (float)W;
    float escalaY = viewport.height / (float)H;

    if (escalaX <= 0.0f || escalaY <= 0.0f)
        return (Vector2){-10000.0f, -10000.0f};

    return (Vector2){
        (m.x - viewport.x) / escalaX,
        (m.y - viewport.y) / escalaY
    };
}

void textoCentro(const char *s, int x, int y, int tam, Color c)
{
    DrawText(s, x - MeasureText(s, tam) / 2, y, tam, c);
}

void mensagem(const char *s, const char *titulo, float t, int cor)
{
    if (s) snprintf(msg, sizeof(msg), "%s", s);
    if (titulo) snprintf(tituloMsg, sizeof(tituloMsg), "%s", titulo);
    msgT = t + 2.5f;
    corMsg = cor;
}

void aviso(const char *s)
{
    snprintf(toast, sizeof(toast), "%s", s);
    toastT = 4.0f;
}

bool clicou(Rectangle rc)
{
    bool ok = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouseVirtual(), rc);
    if (ok) tocar(sndClique);
    return ok;
}

bool hover(Rectangle rc)
{
    return CheckCollisionPointRec(mouseVirtual(), rc);
}

bool dentro(Vector2 p, const Vector2 *v, int n)
{
    bool ok = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((v[i].y > p.y) != (v[j].y > p.y)) &&
            p.x < (v[j].x - v[i].x) * (p.y - v[i].y) / (v[j].y - v[i].y) + v[i].x) ok = !ok;
    }
    return ok;
}

int regiaoSob(Vector2 m)
{
    for (int i = 0; i < N; i++)
        if (r[i].ativa && dentro(m, r[i].pt, r[i].n)) return i;
    return -1;
}

int estadoSob(Vector2 m)
{
    int cx = (int)(m.x / CEL), cy = (int)(m.y / CEL);
    if (cx < 0 || cy < 0 || cx >= GW || cy >= GH) return -1;
    return gridEst[cy][cx];
}

float tempoCritico(float bat, float rate)
{
    if (bat < LIM_BAIXO && rate < -0.001f) return bat / (-rate);
    if (bat > LIM_ALTO && rate > 0.001f) return (BAT_MAX - bat) / rate;
    return -1.0f;
}

float rateReg(int i)
{
    Regiao *g = &r[i];
    return (g->ajuste * g->multProd - g->consBase * g->multCons) * 4.0f * velFase();
}

float rateEst(int i)
{
    Estado *e = &est[i];
    return (e->ajuste * r[e->reg].multProd - e->cons * r[e->reg].multCons) * 4.0f * velFase();
}

const char *nomeAlvo(int i) { return secreta ? est[i].nome : r[i].nome; }

Rectangle linhaReg(int k) { return (Rectangle){955, 118.0f + k * 54.0f, 300, 48}; }
