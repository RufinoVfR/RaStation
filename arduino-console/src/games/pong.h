#ifndef PONG_H
#define PONG_H

#include "config.h"

enum Winner { WINNER_NONE, WINNER_PLAYER, WINNER_CPU };

// Ciclo de vida do jogo (usado pelo main.cpp).
void pongInit();
void pongUpdate(unsigned long now);
void pongDraw();
bool pongIsGameOver();
Winner pongGetWinner();
int pongGetPlayerScore();
int pongGetCpuScore();

// API de baixo nível, exposta para os testes de caixa branca — física,
// pontuação e velocidade isoladas do input e do timing completo.
void pongResetForTest();
void pongStep();               // um passo de física (sem CPU, sem input)
void pongResetBall();
void pongOnPointScored(Winner scorer);

float pongGetBallX();
float pongGetBallY();
float pongGetVelX();
float pongGetVelY();
void pongSetBall(float x, float y, float velX, float velY);

uint8_t pongGetPlayerRow(); // 0..ALTURA-1
uint8_t pongGetCpuRow();
void pongSetPlayerRow(uint8_t row);
void pongSetCpuRow(uint8_t row);
void pongUpdateCpuForTest(); // aplica um tick de reação da CPU isoladamente
void pongMovePlayer(int8_t delta); // delta negativo = CIMA, positivo = BAIXO (com clamp)

unsigned long pongGetSpeed();

#endif // PONG_H
