#ifndef INVADERS_H
#define INVADERS_H

#include "config.h"

// Ciclo de vida do jogo (usado pelo main.cpp).
void invadersInit();
void invadersUpdate(unsigned long now);
void invadersDraw();
bool invadersIsGameOver();
int invadersGetScore();
uint8_t invadersGetWave();

// API de baixo nível, exposta para os testes de caixa branca.
void invadersResetForTest();

// Bitmask dos 6 inimigos (bit i = inimigo i vivo)
bool invadersIsAlive(uint8_t index);
void invadersKillEnemy(uint8_t index);
bool invadersAllDead();
uint8_t invadersCountAlive();

uint8_t invadersGetGroupCol();
void invadersSetGroupCol(uint8_t col);
uint8_t invadersGetGroupRow(); // 0 = linha dos inimigos, 1 = alcançou a linha do jogador
void invadersSetGroupRow(uint8_t row);
void invadersCheckDescent(); // aplica game over se o grupo alcançou a linha do jogador

uint8_t invadersGetShipCol();
void invadersSetShipCol(uint8_t col);

void invadersShoot(); // um projétil por vez
bool invadersPlayerProjectileActive();
uint8_t invadersGetPlayerProjectileCol();
void invadersSetPlayerProjectile(uint8_t col, bool active);
void invadersCheckPlayerCollision(); // projétil do jogador x inimigos, por coluna

void invadersEnemyShoot();
bool invadersEnemyProjectileActive();
uint8_t invadersGetEnemyProjectileCol();
void invadersSetEnemyProjectile(uint8_t col, bool active);
void invadersCheckShipCollision(); // projétil inimigo x nave

unsigned long invadersGetMoveInterval();
unsigned long invadersGetShootInterval();
void invadersAdvanceWave(); // reinicia os inimigos e aumenta a dificuldade (mesma lógica de "todos mortos")

#endif // INVADERS_H
