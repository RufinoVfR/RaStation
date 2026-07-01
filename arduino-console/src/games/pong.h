#ifndef PONG_H
#define PONG_H

#include <LiquidCrystal.h>
#include "../../include/config.h"
#include "../input.h"

// Estado do Pong: 1 linha = barra do jogador, a bola quica entre as colunas.
// Tudo em bytes pequenos para caber na RAM do Uno.
static uint8_t pongBolaX = 0;
static uint8_t pongBolaY = 0;
static int8_t pongBolaDirX = 1;
static int8_t pongBolaDirY = 1;
static uint8_t pongBarraX = 0; // coluna esquerda da barra do jogador (linha ALTURA-1)
static const uint8_t PONG_BARRA_LARGURA = 3;
static uint8_t pongPlacar = 0;
static bool pongFimDeJogo = false;

inline void pongSetup(LiquidCrystal &lcd) {
  uint8_t charBola[8]  = {0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00};
  uint8_t charBarra[8] = {0x00, 0x00, 0x00, 0x1F, 0x1F, 0x00, 0x00, 0x00};
  lcd.createChar(0, charBola);
  lcd.createChar(1, charBarra);

  pongBolaX = LARGURA / 2;
  pongBolaY = 0;
  pongBolaDirX = 1;
  pongBolaDirY = 1;
  pongBarraX = LARGURA / 2 - PONG_BARRA_LARGURA / 2;
  pongPlacar = 0;
  pongFimDeJogo = false;
}

inline void pongInput(const InputState &input) {
  if (input.esquerda && pongBarraX > 0) {
    pongBarraX--;
  }
  if (input.direita && pongBarraX + PONG_BARRA_LARGURA < LARGURA) {
    pongBarraX++;
  }
}

// Move a bola, rebate nas paredes X e no topo, e verifica se o jogador
// rebateu com a barra na última linha ou perdeu o ponto.
inline void pongUpdate() {
  if (pongFimDeJogo) return;

  int8_t novoX = (int8_t)pongBolaX + pongBolaDirX;
  int8_t novoY = (int8_t)pongBolaY + pongBolaDirY;

  if (novoX < 0 || novoX >= LARGURA) {
    pongBolaDirX = -pongBolaDirX;
    novoX = (int8_t)pongBolaX + pongBolaDirX;
  }

  if (novoY < 0) {
    pongBolaDirY = -pongBolaDirY;
    novoY = (int8_t)pongBolaY + pongBolaDirY;
  }

  if (novoY >= ALTURA - 1) {
    // Chegou na linha da barra: verifica se rebateu.
    if (novoX >= pongBarraX && novoX < pongBarraX + PONG_BARRA_LARGURA) {
      pongBolaDirY = -pongBolaDirY;
      novoY = (int8_t)pongBolaY + pongBolaDirY;
      pongPlacar++;
    } else {
      pongFimDeJogo = true;
      return;
    }
  }

  pongBolaX = (uint8_t)novoX;
  pongBolaY = (uint8_t)novoY;
}

inline void pongRender(LiquidCrystal &lcd) {
  lcd.clear();

  lcd.setCursor(pongBolaX, pongBolaY);
  lcd.write((uint8_t)0);

  for (uint8_t i = 0; i < PONG_BARRA_LARGURA; i++) {
    lcd.setCursor(pongBarraX + i, ALTURA - 1);
    lcd.write((uint8_t)1);
  }

  if (pongFimDeJogo) {
    lcd.setCursor(0, 0);
    lcd.print(F("Fim! Placar: "));
    lcd.print(pongPlacar);
  }
}

inline bool pongGameOver() {
  return pongFimDeJogo;
}

#endif // PONG_H
