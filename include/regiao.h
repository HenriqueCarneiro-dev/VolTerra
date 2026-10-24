/**
 * regiao.h
 * Uma região do país abastecida (ou não) pela usina.
 *
 * Este módulo só guarda e altera dados da região: não desenha nada na tela.
 */

#ifndef REGIAO_H
#define REGIAO_H

typedef enum {
    REGIAO_INATIVA,   // ainda não entrou no jogo (desbloqueia em noites futuras)
    REGIAO_OK,        // recebendo energia
    REGIAO_ALERTA,    // sem energia, contagem regressiva para o blackout
    REGIAO_BLACKOUT   // contagem zerou: região no escuro
} EstadoRegiao;

typedef struct {
    char nome[16];
    int consumo;          // energia consumida (MW) quando ligada
    int ligada;           // 1 = recebendo energia, 0 = desligada
    EstadoRegiao estado;
    int segundosAlerta;   // segundos restantes até o blackout (só em ALERTA)
} Regiao;

/** Prepara a região (inativa e desligada). */
void regiaoInit(Regiao *r, const char *nome, int consumo);

/** Coloca a região em jogo, desligada e já em alerta. */
void regiaoAtivar(Regiao *r, int tempoAlerta);

/** Liga a região (também restabelece uma região em blackout). */
void regiaoLigar(Regiao *r);

/** Desliga a região e inicia a contagem para o blackout. */
void regiaoDesligar(Regiao *r, int tempoAlerta);

/**
 * Avança um segundo na contagem regressiva.
 * @return 1 se a região entrou em blackout agora, 0 caso contrário.
 */
int regiaoAtualizarSegundo(Regiao *r);

#endif /* REGIAO_H */
