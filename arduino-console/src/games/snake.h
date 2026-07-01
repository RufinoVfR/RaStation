#ifndef SNAKE_H
#define SNAKE_H

#include "config.h"

enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

// Posição empacotada em 1 byte: bits 7-4 = coluna (0-15), bits 3-0 = linha (0-1).
uint8_t packPos(uint8_t col, uint8_t row);
uint8_t unpackCol(uint8_t pos);
uint8_t unpackRow(uint8_t pos);

// Ciclo de vida do jogo (usado pelo main.cpp).
void snakeInit();
void snakeUpdate(unsigned long now);
void snakeDraw();
bool snakeIsGameOver();
int snakeGetScore();

// API de baixo nível, exposta para os testes de caixa branca (buffer
// circular, direção, colisão) sem precisar passar pela contagem regressiva
// nem pelo timing completo de snakeUpdate().
void snakeResetBuffer();
void snakePush(uint8_t pos);
void snakeMoveStep();
void snakeSetDirection(Direction dir);
void snakeForceDirection(Direction dir); // ignora a regra anti-180; só para setup de testes
Direction snakeGetDirection();
uint8_t snakeGetHead();
uint8_t snakeGetLength();
uint8_t snakeGetSegment(uint8_t indexFromHead); // 0 = cabeça, length-1 = cauda
unsigned long snakeGetMoveInterval();
uint8_t snakeGetFoodPos();
void snakeSetFoodPos(uint8_t pos);
void snakeSpawnFood();
void snakeOnFoodEaten(); // efeitos de comer: pontuação, velocidade, nova comida

#endif // SNAKE_H
