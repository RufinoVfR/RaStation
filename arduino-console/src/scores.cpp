#include <stdint.h>
#include <EEPROM.h>
#include "scores.h"

static const int ADDR_SNAKE = 0;    // 2 bytes (int16_t)
static const int ADDR_PONG = 2;     // 2 bytes
static const int ADDR_INVADERS = 4; // 2 bytes
static const int ADDR_VALID = 6;    // 1 byte
static const uint8_t VALID_BYTE = 0xAB;

// Armazena como int16_t (sempre 2 bytes) em vez de `int` — no AVR `int` já
// tem 2 bytes, mas no ambiente nativo dos testes (`native`) tem 4, o que
// bagunçaria os endereços fixos abaixo se usássemos `int` direto no EEPROM.

static int addrForGame(GameType game) {
  switch (game) {
    case GAME_SNAKE:    return ADDR_SNAKE;
    case GAME_PONG:     return ADDR_PONG;
    case GAME_INVADERS: return ADDR_INVADERS;
  }
  return ADDR_SNAKE;
}

// Se o byte de validação não estiver presente, a EEPROM nunca foi
// inicializada por este firmware (ou veio de fábrica com 0xFF) — zera os
// três recordes e grava o byte de validação.
static void ensureInitialized() {
  if (EEPROM.read(ADDR_VALID) != VALID_BYTE) {
    int16_t zero = 0;
    EEPROM.put(ADDR_SNAKE, zero);
    EEPROM.put(ADDR_PONG, zero);
    EEPROM.put(ADDR_INVADERS, zero);
    EEPROM.write(ADDR_VALID, VALID_BYTE);
  }
}

int loadHighScore(GameType game) {
  ensureInitialized();
  int16_t value;
  EEPROM.get(addrForGame(game), value);
  return value;
}

void saveHighScore(GameType game, int score) {
  ensureInitialized();
  int16_t current;
  EEPROM.get(addrForGame(game), current);
  if (score > current) {
    int16_t value = (int16_t)score;
    EEPROM.put(addrForGame(game), value);
  }
}

bool isNewRecord(GameType game, int score) {
  ensureInitialized();
  int16_t current;
  EEPROM.get(addrForGame(game), current);
  return score > current;
}
