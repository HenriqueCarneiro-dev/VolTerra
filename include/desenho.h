/**
 * desenho.h
 * Tudo que escreve na tela usando a cli-lib (screen.h).
 *
 * Os textos em tela ficam sem acento de propósito: printf com largura fixa
 * conta bytes, e caracteres UTF-8 desalinham as colunas.
 */

#ifndef DESENHO_H
#define DESENHO_H

#include "jogo.h"

/** Tela inicial com o nome do jogo e instruções rápidas. */
void desenharTitulo(void);

/** Painel principal: noite, energia, regiões e mensagem. */
void desenharJogo(const EstadoJogo *j);

/** Tela de vitória ou derrota com a pontuação final. */
void desenharFim(const EstadoJogo *j);

#endif /* DESENHO_H */
