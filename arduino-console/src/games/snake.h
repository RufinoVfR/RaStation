#ifndef SNAKE_H
#define SNAKE_H

#include <LiquidCrystal.h>
#include "../../include/config.h"
#include "../input.h"

// Tamanho máximo do corpo da cobra.
// Array pequeno de propósito: cada célula do LCD (16x2 = 32 posições)
// já é o limite físico do jogo, e o Uno tem só 2KB de RAM.
#define SNAKE_MAX_CORPO 16

// Cada posição é guardada em 1 byte: X (4 bits) + Y (4 bits).
// Isso evita gastar 2 ints (4 bytes) por segmento da cobra.
static uint8_t snakeCorpo[SNAKE_MAX_CORPO];
static uint8_t snakeCabeca = 0;   // índice circular da cabeça no array
static uint8_t snakeTamanho = 3;  // tamanho atual da cobra
static uint8_t snakeComidaPos = 0;
static int8_t snakeDirX = 1;
static int8_t snakeDirY = 0;
static bool snakeColidiu = false;

inline uint8_t snakePack(uint8_t x, uint8_t y) {
  return (uint8_t)((x & 0x0F) | ((y & 0x0F) << 4));
}

inline uint8_t snakeUnpackX(uint8_t pos) { return pos & 0x0F; }
inline uint8_t snakeUnpackY(uint8_t pos) { return (pos >> 4) & 0x0F; }

// Posiciona a comida em uma célula aleatória do grid.
inline void snakeGerarComida() {
  uint8_t x = random(0, LARGURA);
  uint8_t y = random(0, ALTURA);
  snakeComidaPos = snakePack(x, y);
}

// Configura custom characters e o estado inicial da cobra.
inline void snakeSetup(LiquidCrystal &lcd) {
  // Custom chars: 0 = cabeça, 1 = corpo, 2 = comida
  uint8_t charCabeca[8] = {0x00, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E, 0x00, 0x00};
  uint8_t charCorpo[8]  = {0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00};
  uint8_t charComida[8] = {0x00, 0x04, 0x0E, 0x1F, 0x0E, 0x04, 0x00, 0x00};
  lcd.createChar(0, charCabeca);
  lcd.createChar(1, charCorpo);
  lcd.createChar(2, charComida);

  snakeTamanho = 3;
  snakeCabeca = 0;
  for (uint8_t i = 0; i < snakeTamanho; i++) {
    snakeCorpo[i] = snakePack(3 - i, 0);
  }
  snakeDirX = 1;
  snakeDirY = 0;
  snakeColidiu = false;
  snakeGerarComida();
}

// Lê o input e atualiza a direção (não deixa inverter 180 graus).
inline void snakeInput(const InputState &input) {
  if (input.esquerda && snakeDirX != 1) { snakeDirX = -1; snakeDirY = 0; }
  else if (input.direita && snakeDirX != -1) { snakeDirX = 1; snakeDirY = 0; }
  else if (input.cima && snakeDirY != 1) { snakeDirX = 0; snakeDirY = -1; }
  else if (input.baixo && snakeDirY != -1) { snakeDirX = 0; snakeDirY = 1; }
}

// Avança a cobra uma célula, verifica colisão com parede, com o próprio
// corpo e com a comida. Usa o array como buffer circular (sem shift).
inline void snakeUpdate() {
  if (snakeColidiu) return;

  uint8_t atual = snakeCorpo[snakeCabeca];
  int8_t novoX = (int8_t)snakeUnpackX(atual) + snakeDirX;
  int8_t novoY = (int8_t)snakeUnpackY(atual) + snakeDirY;

  if (novoX < 0 || novoX >= LARGURA || novoY < 0 || novoY >= ALTURA) {
    snakeColidiu = true;
    return;
  }

  uint8_t novaPos = snakePack((uint8_t)novoX, (uint8_t)novoY);

  for (uint8_t i = 0; i < snakeTamanho; i++) {
    if (snakeCorpo[i] == novaPos) {
      snakeColidiu = true;
      return;
    }
  }

  bool comeu = (novaPos == snakeComidaPos);

  snakeCabeca = (snakeCabeca + 1) % SNAKE_MAX_CORPO;
  snakeCorpo[snakeCabeca] = novaPos;

  if (comeu && snakeTamanho < SNAKE_MAX_CORPO) {
    snakeTamanho++;
    snakeGerarComida();
  }
}

// Desenha a cobra e a comida no LCD usando os custom chars.
inline void snakeRender(LiquidCrystal &lcd) {
  lcd.clear();

  uint8_t x = snakeUnpackX(snakeComidaPos);
  uint8_t y = snakeUnpackY(snakeComidaPos);
  lcd.setCursor(x, y);
  lcd.write((uint8_t)2);

  for (uint8_t i = 0; i < snakeTamanho; i++) {
    uint8_t idx = (snakeCabeca + SNAKE_MAX_CORPO - i) % SNAKE_MAX_CORPO;
    uint8_t px = snakeUnpackX(snakeCorpo[idx]);
    uint8_t py = snakeUnpackY(snakeCorpo[idx]);
    lcd.setCursor(px, py);
    lcd.write((uint8_t)(i == 0 ? 0 : 1));
  }

  if (snakeColidiu) {
    lcd.setCursor(0, 1);
    lcd.print(F("Fim de jogo!"));
  }
}

inline bool snakeFimDeJogo() {
  return snakeColidiu;
}

#endif // SNAKE_H
