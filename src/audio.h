#ifndef AUDIO_H
#define AUDIO_H

#include "tipos.h"

/* Audio gerado por codigo, musica por fase e alertas sonoros. */
void tocar(Sound s);
void iniciarAudio(void);
void encerrarAudio(void);
void atualizarMusica(void);
void avaliarAlerta(int i, float bat, float rate, float *minT);
void somContagem(float minT, float dt);

#endif /* AUDIO_H */
