#include "tipos.h"
#include "globais.h"
#include "atualizar.h"
#include "audio.h"
#include "desenho_telas.h"
#include "ranking.h"
#include "regioes.h"

int main(void)
{
    InitWindow(W, H, "VolTerra");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    telaVirtual = LoadRenderTexture(W, H);
    SetTextureFilter(telaVirtual.texture, TEXTURE_FILTER_BILINEAR);

    iniciarAudio();
    iniciarRegioes();
    carregarRanking();
    carregarSegredo();

    while (!WindowShouldClose() && !sair) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f;

        float escala = fminf(
            (float)GetScreenWidth() / (float)W,
            (float)GetScreenHeight() / (float)H
        );

        viewport.width = W * escala;
        viewport.height = H * escala;
        viewport.x = (GetScreenWidth() - viewport.width) * 0.5f;
        viewport.y = (GetScreenHeight() - viewport.height) * 0.5f;

        atualizar(dt);
        atualizarMusica();

        BeginTextureMode(telaVirtual);
        ClearBackground(BLACK);
        desenhar();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        DrawTexturePro(
            telaVirtual.texture,
            (Rectangle){0, 0, (float)W, -(float)H},
            viewport,
            (Vector2){0, 0},
            0.0f,
            WHITE
        );

        EndDrawing();
    }

    liberarRanking();
    encerrarAudio();
    UnloadRenderTexture(telaVirtual);
    CloseWindow();
    return 0;
}
