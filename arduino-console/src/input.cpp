#include <Arduino.h>
#include "input.h"

static const unsigned long DEBOUNCE_DELAY = 50;

struct ButtonState {
  uint8_t pin;
  bool lastRaw;
  bool stable;
  unsigned long lastChangeTime;
};

static ButtonState btnEsq   = { BTN_ESQ,   false, false, 0 };
static ButtonState btnDir   = { BTN_DIR,   false, false, 0 };
static ButtonState btnCima  = { BTN_CIMA,  false, false, 0 };
static ButtonState btnBaixo = { BTN_BAIXO, false, false, 0 };

static void resetButton(ButtonState &b) {
  b.lastRaw = false;
  b.stable = false;
  b.lastChangeTime = 0;
}

void inputInit() {
  pinMode(BTN_ESQ, INPUT);
  pinMode(BTN_DIR, INPUT);
  pinMode(BTN_CIMA, INPUT);
  pinMode(BTN_BAIXO, INPUT);

  resetButton(btnEsq);
  resetButton(btnDir);
  resetButton(btnCima);
  resetButton(btnBaixo);
}

// Retorna true se uma transição solto->pressionado acabou de ser confirmada
static bool updatePressed(ButtonState &b, unsigned long now) {
  bool raw = digitalRead(b.pin) == HIGH;
  bool justPressed = false;

  if (raw != b.lastRaw) {
    // reinicia a janela de debounce a cada mudança observada no pino
    b.lastRaw = raw;
    b.lastChangeTime = now;
  } else if (now - b.lastChangeTime >= DEBOUNCE_DELAY && b.stable != raw) {
    b.stable = raw;
    justPressed = b.stable;
  }
  return justPressed;
}

uint8_t readButtons(unsigned long now) {
  bool cima  = updatePressed(btnCima, now);
  bool baixo = updatePressed(btnBaixo, now);
  bool esq   = updatePressed(btnEsq, now);
  bool dir   = updatePressed(btnDir, now);

  if (cima)  return BTN_CIMA;
  if (baixo) return BTN_BAIXO;
  if (esq)   return BTN_ESQ;
  if (dir)   return BTN_DIR;
  return BTN_NONE;
}

uint8_t getHeldButton() {
  if (btnCima.stable)  return BTN_CIMA;
  if (btnBaixo.stable) return BTN_BAIXO;
  if (btnEsq.stable)   return BTN_ESQ;
  if (btnDir.stable)   return BTN_DIR;
  return BTN_NONE;
}
