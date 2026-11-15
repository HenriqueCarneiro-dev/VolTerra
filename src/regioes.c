#include "tipos.h"
#include "globais.h"
#include "util.h"
#include "regioes.h"

static const float geoNorte[][2] = {
    {-73.8f,-7.4f},{-72.8f,-5.2f},{-70.1f,-4.2f},{-70.0f,-2.0f},{-69.4f,-0.8f},{-69.6f,0.7f},
    {-67.3f,2.2f},{-65.4f,1.5f},{-64.0f,4.1f},{-61.5f,4.5f},{-60.7f,5.2f},{-59.9f,4.0f},
    {-60.0f,2.0f},{-58.5f,1.3f},{-56.5f,1.9f},{-54.6f,2.3f},{-52.9f,2.2f},{-51.7f,4.2f},
    {-50.8f,2.0f},{-50.0f,0.6f},{-50.6f,-0.4f},{-49.0f,-1.2f},{-47.3f,-0.9f},{-46.2f,-1.1f},
    {-46.8f,-3.0f},{-48.3f,-5.2f},{-47.7f,-5.8f},{-46.6f,-8.5f},{-46.1f,-10.5f},{-46.3f,-12.5f},
    {-50.2f,-13.4f},{-50.5f,-11.0f},{-50.2f,-9.2f},{-52.0f,-9.0f},{-55.0f,-7.5f},{-58.0f,-8.8f},
    {-60.0f,-10.0f},{-60.5f,-13.5f},{-62.5f,-13.0f},{-65.3f,-9.8f},{-67.5f,-10.3f},
    {-70.5f,-11.0f},{-72.5f,-9.5f}
};

static const float geoNordeste[][2] = {
    {-46.2f,-1.1f},{-46.8f,-3.0f},{-48.3f,-5.2f},{-47.7f,-5.8f},{-46.6f,-8.5f},{-46.1f,-10.5f},
    {-46.3f,-12.5f},{-46.0f,-14.0f},{-44.2f,-15.5f},{-43.5f,-16.5f},{-41.8f,-17.9f},{-39.7f,-18.3f},
    {-39.2f,-16.5f},{-38.9f,-15.0f},{-38.5f,-13.0f},{-37.3f,-11.0f},{-35.8f,-9.8f},{-35.2f,-8.5f},
    {-34.85f,-7.2f},{-34.9f,-6.0f},{-35.3f,-5.2f},{-36.5f,-5.1f},{-37.2f,-4.8f},{-38.5f,-3.7f},
    {-40.0f,-2.9f},{-41.5f,-2.9f},{-42.8f,-2.7f},{-44.0f,-2.5f},{-44.8f,-1.6f}
};

static const float geoCentro[][2] = {
    {-60.5f,-13.5f},{-60.0f,-10.0f},{-58.0f,-8.8f},{-55.0f,-7.5f},{-52.0f,-9.0f},{-50.2f,-9.2f},
    {-50.5f,-11.0f},{-50.2f,-13.4f},{-46.3f,-12.5f},{-46.0f,-14.0f},{-46.8f,-16.0f},{-47.2f,-17.0f},
    {-48.2f,-18.3f},{-50.0f,-19.4f},{-51.0f,-20.2f},{-51.9f,-21.2f},{-52.9f,-22.4f},{-53.1f,-22.7f},
    {-54.6f,-24.0f},{-55.4f,-23.7f},{-55.8f,-22.2f},{-57.9f,-22.0f},{-57.7f,-20.0f},{-58.2f,-17.8f},
    {-60.2f,-16.3f},{-60.2f,-15.0f}
};

static const float geoSudeste[][2] = {
    {-46.0f,-14.0f},{-44.2f,-15.5f},{-43.5f,-16.5f},{-41.8f,-17.9f},{-39.7f,-18.3f},{-39.8f,-19.8f},
    {-40.9f,-21.0f},{-41.0f,-22.0f},{-42.0f,-22.9f},{-44.0f,-23.0f},{-45.4f,-23.8f},{-47.0f,-24.8f},
    {-48.1f,-25.0f},{-49.5f,-24.2f},{-51.5f,-22.9f},{-53.1f,-22.7f},{-52.9f,-22.4f},{-51.9f,-21.2f},
    {-51.0f,-20.2f},{-50.0f,-19.4f},{-48.2f,-18.3f},{-47.2f,-17.0f},{-46.8f,-16.0f}
};

static const float geoSul[][2] = {
    {-48.1f,-25.0f},{-49.5f,-24.2f},{-51.5f,-22.9f},{-53.1f,-22.7f},{-54.6f,-24.0f},{-54.6f,-25.5f},
    {-53.8f,-26.2f},{-54.2f,-27.3f},{-56.0f,-28.5f},{-57.6f,-30.2f},{-56.0f,-30.6f},{-54.6f,-31.5f},
    {-53.6f,-32.9f},{-53.4f,-33.7f},{-52.2f,-32.2f},{-50.8f,-30.5f},{-49.7f,-29.0f},{-48.8f,-27.8f},
    {-48.6f,-26.3f}
};

static void criarPoli(Regiao *g, const float (*d)[2], int n)
{
    g->n = n;
    for (int i = 0; i < n; i++) g->pt[i] = (Vector2){projX(d[i][0]), projY(d[i][1])};
}

static void setReg(int id, const char *nome, Color cor, int fonte, float cons, float lon, float lat)
{
    r[id].nome = nome;
    r[id].cor = cor;
    r[id].fonte = fonte;
    r[id].consBase = cons;
    r[id].c = (Vector2){projX(lon), projY(lat)};
}

void iniciarRegioes(void)
{
    memset(r, 0, sizeof(r));
    setReg(NORTE,    "Norte",        (Color){45, 160, 95, 255},  0, 0.92f, -62.0f, -4.6f);
    setReg(NORDESTE, "Nordeste",     (Color){235, 175, 45, 255}, 1, 0.98f, -42.5f, -8.2f);
    setReg(CENTRO,   "Centro-Oeste", (Color){225, 120, 50, 255}, 2, 0.93f, -54.0f, -14.5f);
    setReg(SUDESTE,  "Sudeste",      (Color){160, 95, 180, 255}, 3, 1.03f, -45.0f, -19.2f);
    setReg(SUL,      "Sul",          (Color){65, 115, 240, 255}, 4, 0.95f, -52.0f, -27.0f);

    criarPoli(&r[NORTE],    geoNorte,    (int)(sizeof(geoNorte)    / sizeof(geoNorte[0])));
    criarPoli(&r[NORDESTE], geoNordeste, (int)(sizeof(geoNordeste) / sizeof(geoNordeste[0])));
    criarPoli(&r[CENTRO],   geoCentro,   (int)(sizeof(geoCentro)   / sizeof(geoCentro[0])));
    criarPoli(&r[SUDESTE],  geoSudeste,  (int)(sizeof(geoSudeste)  / sizeof(geoSudeste[0])));
    criarPoli(&r[SUL],      geoSul,      (int)(sizeof(geoSul)      / sizeof(geoSul[0])));

    for (int i = 0; i < N; i++) {
        r[i].bat = 70.0f;
        r[i].ajuste = 1.0f;
        r[i].multProd = 1.0f;
        r[i].multCons = 1.0f;
        r[i].ativa = false;
    }
}

void construirGrade(void)
{
    for (int y = 0; y < GH; y++)
        for (int x = 0; x < GW; x++) gridEst[y][x] = -1;

    for (int y = 60 / CEL; y < GH; y++) {
        for (int x = 270 / CEL; x < 960 / CEL && x < GW; x++) {
            Vector2 p = {x * CEL + CEL / 2.0f, y * CEL + CEL / 2.0f};
            int rg = -1;
            for (int i = 0; i < N; i++)
                if (dentro(p, r[i].pt, r[i].n)) { rg = i; break; }
            if (rg < 0) continue;
            int melhor = -1;
            float md = 1e9f;
            for (int e = 0; e < NE; e++) {
                if (est[e].reg != rg) continue;
                float d = dist(p, est[e].c);
                if (d < md) { md = d; melhor = e; }
            }
            gridEst[y][x] = (short)melhor;
        }
    }
}
