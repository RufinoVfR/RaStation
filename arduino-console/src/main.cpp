#include <Arduino.h>
#include "config.h"
#include "input.h"
#include "menu.h"
#include "games/snake.h"

static GameState currentState = STATE_MENU;
static unsigned long lastUpdate = 0;
static const unsigned long TICK_MS = 50;

void setup() {
  Serial.begin(9600);
  inputInit();
  menuInit();
}

void loop() {
  unsigned long now = millis();
  if (now - lastUpdate >= TICK_MS) {
    lastUpdate = now;

    switch (currentState) {
      case STATE_MENU: {
        GameState next = menuUpdate(now);
        if (next != STATE_MENU) {
          currentState = next;
          if (next == STATE_SNAKE) snakeInit();
        }
        break;
      }
      case STATE_SNAKE:
        snakeUpdate(now);
        if (snakeIsGameOver()) {
          showGameOver(snakeGetScore());
          currentState = STATE_MENU;
        }
        break;
      case STATE_PONG:
      case STATE_INVADERS:
      case STATE_GAMEOVER:
        // Jogos chegam nas próximas etapas
        break;
    }
  }
}
