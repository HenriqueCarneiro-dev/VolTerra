#include "tipos.h"
#include "globais.h"
#include "util.h"
#include "ui.h"

void telaCarregando(const char *t)
{
    float escala = fminf(
        (float)GetScreenWidth() / (float)W,
        (float)GetScreenHeight() / (float)H
    );
    Rectangle vp;
    vp.width = W * escala;
    vp.height = H * escala;
    vp.x = (GetScreenWidth() - vp.width) * 0.5f;
    vp.y = (GetScreenHeight() - vp.height) * 0.5f;

    BeginTextureMode(telaVirtual);
    ClearBackground((Color){8, 18, 40, 255});
    textoCentro(t, W / 2, 330, 30, WHITE);
    textoCentro("So leva um instante...", W / 2, 375, 18, LIGHTGRAY);
    EndTextureMode();

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(
        telaVirtual.texture,
        (Rectangle){0, 0, (float)W, -(float)H},
        vp,
        (Vector2){0, 0},
        0.0f,
        WHITE
    );
    EndDrawing();
}

void caixa(Rectangle rc, float rd, Color fill, Color borda)
{
    DrawRectangleRounded((Rectangle){rc.x - 2, rc.y - 2, rc.width + 4, rc.height + 4}, rd, 8, borda);
    DrawRectangleRounded(rc, rd, 8, fill);
}

void desenharBotao(Rectangle rc, const char *t, Color base)
{
    bool h = hover(rc);
    DrawRectangleRounded((Rectangle){rc.x + 2, rc.y + 5, rc.width, rc.height}, 0.3f, 8, Fade(BLACK, 0.4f));
    caixa(rc, 0.3f, h ? ColorBrightness(base, 0.2f) : base, h ? WHITE : Fade(WHITE, 0.4f));
    int tx = (int)(rc.x + rc.width / 2) - MeasureText(t, 23) / 2;
    DrawText(t, tx, (int)(rc.y + rc.height / 2) - 12, 23, WHITE);
}

void coracao(float x, float y, float s, Color c)
{
    DrawCircle((int)(x - s * 0.5f), (int)(y - s * 0.2f), s * 0.55f, c);
    DrawCircle((int)(x + s * 0.5f), (int)(y - s * 0.2f), s * 0.55f, c);
    DrawTriangle((Vector2){x - s * 1.03f, y}, (Vector2){x, y + s * 1.15f}, (Vector2){x + s * 1.03f, y}, c);
}

/* Preenchimento de Polígono por Scanline com Gradiente Dinâmico */
void preencherComGradiente(const Vector2 *v, int n, Color corTopo, Color corBase)
{
    float miny = v[0].y, maxy = v[0].y;
    for (int i = 1; i < n; i++) {
        if (v[i].y < miny) miny = v[i].y;
        if (v[i].y > maxy) maxy = v[i].y;
    }
    float h = maxy - miny;
    if (h <= 0) h = 1.0f;

    for (int y = (int)miny; y <= (int)maxy; y++) {
        float xs[64];
        int k = 0;
        float fy = y + 0.5f;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            if (((v[i].y > fy) != (v[j].y > fy)) && k < 64)
                xs[k++] = v[j].x + (fy - v[j].y) * (v[i].x - v[j].x) / (v[i].y - v[j].y);
        }
        for (int a = 1; a < k; a++) {
            float t = xs[a];
            int b = a - 1;
            while (b >= 0 && xs[b] > t) { xs[b + 1] = xs[b]; b--; }
            xs[b + 1] = t;
        }
        float factor = (y - miny) / h;
        Color cLinha = misturar(corTopo, corBase, factor);
        for (int a = 0; a + 1 < k; a += 2)
            DrawRectangle((int)xs[a], y, (int)(xs[a + 1] - xs[a]) + 1, 1, cLinha);
    }
}

static void preencher(const Vector2 *v, int n, Color c)
{
    preencherComGradiente(v, n, ColorBrightness(c, 0.15f), ColorBrightness(c, -0.15f));
}

void icone(int f, float x, float y, float t, float e)
{
    int ix = (int)x, iy = (int)y;

    DrawCircle(ix, iy, 22 * e, Fade((Color){10, 20, 38, 255}, 0.85f));
    DrawCircleLines(ix, iy, 22 * e, Fade(WHITE, 0.75f));

    switch (f) {
        case 0: {
            Color concreto = (Color){190, 200, 215, 255};
            Vector2 tl = {x - 8 * e, y - 11 * e}, tr = {x + 8 * e, y - 11 * e};
            Vector2 bl = {x - 14 * e, y + 6 * e}, br = {x + 14 * e, y + 6 * e};
            DrawTriangle(tl, bl, br, concreto);
            DrawTriangle(tl, br, tr, concreto);
            DrawRectangle((int)(x - 3 * e), (int)(y - 8 * e), (int)(6 * e), (int)(10 * e), (Color){60, 80, 110, 255});
            for (int i = 0; i < 3; i++) {
                float d = fmodf(t * 18.0f + i * 5.0f, 12.0f * e);
                DrawCircle(ix, (int)(y - 2 * e + d), (int)(1.6f * e + 0.5f), SKYBLUE);
            }
            for (int k = -14; k <= 12; k += 2) {
                float w = sinf(t * 4.0f + k * 0.6f) * 1.5f * e;
                DrawRectangle((int)(x + k * e), (int)(y + 8 * e + w), (int)(2 * e + 1), (int)(4 * e), (Color){50, 150, 245, 255});
            }
            break;
        }
        case 1: {
            Vector2 a = {x - 2.5f * e, y + 15 * e}, b = {x + 2.5f * e, y + 15 * e}, c = {x, y - 3 * e};
            DrawTriangle(a, b, c, RAYWHITE);
            DrawRectangle((int)(x - 7 * e), (int)(y + 14 * e), (int)(14 * e), (int)(2 * e + 1), (Color){120, 140, 170, 255});
            float a0 = t * 140.0f;
            for (int i = 0; i < 3; i++) {
                float ang = (a0 + i * 120.0f) * DEG2RAD;
                Vector2 ponta = {x + cosf(ang) * 15 * e, y - 3 * e + sinf(ang) * 15 * e};
                DrawLineEx((Vector2){x, y - 3 * e}, ponta, 2.6f * e, (Color){235, 245, 255, 255});
            }
            DrawCircle(ix, (int)(y - 3 * e), (int)(3 * e), (Color){255, 215, 60, 255});
            break;
        }
        case 2: {
            float sx = x - 8 * e, sy = y - 9 * e;
            for (int i = 0; i < 8; i++) {
                float ang = (t * 40.0f + i * 45.0f) * DEG2RAD;
                DrawLineEx((Vector2){sx + cosf(ang) * 5.5f * e, sy + sinf(ang) * 5.5f * e},
                           (Vector2){sx + cosf(ang) * 8.5f * e, sy + sinf(ang) * 8.5f * e}, 1.4f * e, YELLOW);
            }
            DrawCircle((int)sx, (int)sy, (int)(4.5f * e), (Color){255, 220, 60, 255});

            Color painel = (Color){25, 70, 160, 255};
            Vector2 tl = {x - 4 * e, y - 1 * e}, tr = {x + 14 * e, y - 1 * e};
            Vector2 bl = {x - 12 * e, y + 10 * e}, br = {x + 10 * e, y + 10 * e};
            DrawTriangle(tl, bl, br, painel);
            DrawTriangle(tl, br, tr, painel);
            Color grade = (Color){140, 200, 255, 255};
            DrawLineV(tl, tr, grade); DrawLineV(bl, br, grade);
            DrawLineV(tl, bl, grade); DrawLineV(tr, br, grade);
            DrawLineV((Vector2){x - 8 * e, y + 4.5f * e}, (Vector2){x + 12 * e, y + 4.5f * e}, grade);
            DrawLineV((Vector2){x + 0.3f * e, y - 1 * e}, (Vector2){x - 1.5f * e, y + 10 * e}, grade);
            DrawLineV((Vector2){x + 5 * e, y - 1 * e}, (Vector2){x + 4 * e, y + 10 * e}, grade);
            DrawLineEx((Vector2){x - 1 * e, y + 10 * e}, (Vector2){x - 1 * e, y + 15 * e}, 2 * e, (Color){150, 160, 180, 255});
            DrawRectangle((int)(x - 6 * e), (int)(y + 14 * e), (int)(10 * e), (int)(2 * e + 1), (Color){150, 160, 180, 255});
            break;
        }
        case 3: {
            for (int i = 0; i < 10; i++) {
                float u = (i - 4.5f) / 4.5f;
                float w = (6.0f + 4.5f * u * u) * e;
                DrawRectangle((int)(x - w), (int)(y - 8 * e + i * 2.4f * e), (int)(2 * w), (int)(2.6f * e + 1),
                              i % 2 ? (Color){205, 210, 220, 255} : (Color){185, 192, 205, 255});
            }
            for (int i = 0; i < 3; i++) {
                float p = fmodf(t * 0.7f + i * 0.33f, 1.0f);
                DrawCircle((int)(x + sinf(t * 2.0f + i) * 3 * e), (int)(y - 10 * e - p * 9 * e),
                           (int)((3.0f + p * 2.5f) * e), Fade(WHITE, 0.75f * (1.0f - p)));
            }
            DrawCircle(ix, (int)(y + 3 * e), (int)(2 * e + 0.5f), (Color){255, 215, 60, 255});
            DrawEllipseLines(ix, (int)(y + 3 * e), 5 * e, 2.2f * e, (Color){255, 215, 60, 255});
            break;
        }
        default: {
            Color verde = (Color){90, 215, 110, 255};
            DrawLineEx((Vector2){x, y + 4 * e}, (Vector2){x, y - 8 * e}, 2 * e, (Color){60, 160, 80, 255});
            DrawEllipse((int)(x - 6 * e), (int)(y - 7 * e), 6 * e, 3.2f * e, verde);
            DrawEllipse((int)(x + 6 * e), (int)(y - 11 * e), 6 * e, 3.2f * e, (Color){120, 235, 130, 255});
            float fl = 1.0f + 0.15f * sinf(t * 12.0f);
            DrawTriangle((Vector2){x - 3 * e, y + 5 * e}, (Vector2){x + 3 * e, y + 5 * e},
                         (Vector2){x, y - 2 * e * fl}, (Color){255, 140, 40, 255});
            DrawTriangle((Vector2){x - 1.6f * e, y + 5 * e}, (Vector2){x + 1.6f * e, y + 5 * e},
                         (Vector2){x, y + 0.5f * e}, (Color){255, 220, 80, 255});
            DrawRectangleRounded((Rectangle){x - 12 * e, y + 8 * e, 24 * e, 4.5f * e}, 0.5f, 4, (Color){130, 85, 50, 255});
            DrawRectangleRounded((Rectangle){x - 8 * e, y + 12 * e, 16 * e, 4 * e}, 0.5f, 4, (Color){105, 68, 40, 255});
            DrawCircle((int)(x - 12 * e), (int)(y + 10.2f * e), (int)(2.2f * e), (Color){190, 140, 90, 255});
            break;
        }
    }
}

Color corBatVisual(float b)
{
    if (b < LIM_BAIXO) return (Color){255, 70, 70, 255};
    if (b < LIM_BAIXO + 8) return (Color){255, 190, 60, 255};
    if (b > 100.0f) return (Color){255, 70, 70, 255};
    if (b > LIM_ALTO) return (Color){255, 140, 60, 255};
    if (b > LIM_ALTO - 6) return (Color){255, 210, 80, 255};
    return (Color){70, 225, 140, 255};
}

void setaEvento(float x, float y, bool sobe, float esc)
{
    Color c = sobe ? (Color){70, 235, 130, 255} : (Color){255, 80, 80, 255};
    float s = 9.0f * esc;
    DrawCircle((int)x, (int)y, s + 6, Fade(BLACK, 0.6f));
    DrawCircleLines((int)x, (int)y, s + 6, Fade(c, 0.9f));
    if (sobe) DrawTriangle((Vector2){x, y - s}, (Vector2){x - s, y + s * 0.7f}, (Vector2){x + s, y + s * 0.7f}, c);
    else      DrawTriangle((Vector2){x, y + s}, (Vector2){x + s, y - s * 0.7f}, (Vector2){x - s, y - s * 0.7f}, c);
}

void cronometro(float x, float y, float raio, float seg, bool black)
{
    float t = (float)GetTime();
    float frac = lim(seg / TEMPO_CRONO, 0.0f, 1.0f);
    Color c = black ? (Color){255, 70, 70, 255} : (Color){255, 150, 40, 255};
    if (seg < 5.0f && sinf(t * 14.0f) > 0.0f) c = WHITE;
    DrawCircle((int)x, (int)y, raio + 3, Fade(BLACK, 0.8f));
    DrawRing((Vector2){x, y}, raio - 4, raio, 0, 360, 48, Fade(WHITE, 0.2f));
    DrawRing((Vector2){x, y}, raio - 4, raio, -90.0f, -90.0f + 360.0f * frac, 48, c);
    const char *txt = TextFormat("%d", (int)ceilf(seg));
    int tam = (int)(raio * 0.95f);
    DrawText(txt, (int)x - MeasureText(txt, tam) / 2, (int)y - tam / 2, tam, WHITE);
}
