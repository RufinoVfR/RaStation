#include "input.h"
#include "../include/config.h"

// Timestamps da última mudança de estado de cada botão, para debounce.
// static: vive só neste arquivo, não polui o escopo global do projeto.
static unsigned long lastChangeEsq = 0;
static unsigned long lastChangeDir = 0;
static unsigned long lastChangeCima = 0;
static unsigned long lastChangeBaixo = 0;

static uint8_t rawEsq = LOW, rawDir = LOW, rawCima = LOW, rawBaixo = LOW;
static uint8_t stableEsq = LOW, stableDir = LOW, stableCima = LOW, stableBaixo = LOW;

// Aplica debounce simples baseado em millis() a um único botão.
// Retorna o valor estável (já filtrado) e atualiza os "últimos" por referência.
static uint8_t debounce(int pino, uint8_t &rawAnterior, uint8_t &estavel, unsigned long &ultimaMudanca) {
  uint8_t leituraAtual = digitalRead(pino);

  if (leituraAtual != rawAnterior) {
    ultimaMudanca = millis();
    rawAnterior = leituraAtual;
  }

  if ((millis() - ultimaMudanca) > DEBOUNCE_MS) {
    estavel = leituraAtual;
  }

  return estavel;
}

void inputSetup() {
  pinMode(BTN_ESQ, INPUT);
  pinMode(BTN_DIR, INPUT);
  pinMode(BTN_CIMA, INPUT);
  pinMode(BTN_BAIXO, INPUT);
}

InputState inputRead() {
  InputState estado;

  estado.esquerda = debounce(BTN_ESQ, rawEsq, stableEsq, lastChangeEsq) == HIGH;
  estado.direita  = debounce(BTN_DIR, rawDir, stableDir, lastChangeDir) == HIGH;
  estado.cima     = debounce(BTN_CIMA, rawCima, stableCima, lastChangeCima) == HIGH;
  estado.baixo    = debounce(BTN_BAIXO, rawBaixo, stableBaixo, lastChangeBaixo) == HIGH;

  return estado;
}
