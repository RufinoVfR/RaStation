#ifndef INVADERS_H
#define INVADERS_H

#include <LiquidCrystal.h>
#include "../../include/config.h"
#include "../input.h"

// Versão simplificada: só 2 linhas disponíveis no LCD, então os inimigos
// ficam fixos na linha 0 e o jogador atira de baixo para cima (linha 1).
//
// Os inimigos são guardados como um bitmask de 16 bits (1 bit por coluna)
// em vez de um array de structs — cabe em 2 bytes e cobre toda a LARGURA.
static uint16_t invadersVivos = 0xFFFF;
static uint8_t invadersNaveX = 0;
static bool invadersTiroAtivo = false;
static uint8_t invadersTiroX = 0;
static uint8_t invadersPlacar = 0;
static bool invadersVenceu = false;
static bool invadersPerdeu = false;

inline void invadersSetup(LiquidCrystal &lcd) {
  uint8_t charNave[8]     = {0x00, 0x0E, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00};
  uint8_t charInimigo[8]  = {0x0A, 0x1F, 0x15, 0x1F, 0x00, 0x00, 0x00, 0x00};
  uint8_t charProjetil[8] = {0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
  lcd.createChar(0, charNave);
  lcd.createChar(1, charInimigo);
  lcd.createChar(2, charProjetil);

  // Só cria inimigos até LARGURA bits (evita ler colunas fora do LCD).
  invadersVivos = (uint16_t)((1UL << LARGURA) - 1);
  invadersNaveX = LARGURA / 2;
  invadersTiroAtivo = false;
  invadersPlacar = 0;
  invadersVenceu = false;
  invadersPerdeu = false;
}

inline void invadersInput(const InputState &input) {
  if (input.esquerda && invadersNaveX > 0) {
    invadersNaveX--;
  }
  if (input.direita && invadersNaveX < LARGURA - 1) {
    invadersNaveX++;
  }
  if (input.cima && !invadersTiroAtivo) {
    invadersTiroAtivo = true;
    invadersTiroX = invadersNaveX;
  }
}

inline void invadersUpdate() {
  if (invadersVenceu || invadersPerdeu) return;

  if (invadersTiroAtivo) {
    if (invadersVivos & (1U << invadersTiroX)) {
      invadersVivos &= ~(1U << invadersTiroX);
      invadersPlacar++;
      invadersTiroAtivo = false;
    } else {
      invadersTiroAtivo = false;
    }
  }

  if (invadersVivos == 0) {
    invadersVenceu = true;
  }
}

inline void invadersRender(LiquidCrystal &lcd) {
  lcd.clear();

  for (uint8_t x = 0; x < LARGURA; x++) {
    if (invadersVivos & (1U << x)) {
      lcd.setCursor(x, 0);
      lcd.write((uint8_t)1);
    }
  }

  lcd.setCursor(invadersNaveX, ALTURA - 1);
  lcd.write((uint8_t)0);

  if (invadersVenceu) {
    lcd.setCursor(0, 0);
    lcd.print(F("Venceu! "));
    lcd.print(invadersPlacar);
  }
}

inline bool invadersFimDeJogo() {
  return invadersVenceu || invadersPerdeu;
}

#endif // INVADERS_H
