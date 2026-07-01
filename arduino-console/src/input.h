#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>

// Estado dos 4 botões após leitura + debounce.
// Usamos bitfields de 1 bit cada para caber tudo em 1 byte (economia de RAM).
struct InputState {
  uint8_t esquerda : 1;
  uint8_t direita  : 1;
  uint8_t cima     : 1;
  uint8_t baixo    : 1;
};

// Configura os pinos dos botões como entrada.
void inputSetup();

// Lê os 4 botões com debounce e devolve o estado atual.
// Deve ser chamada uma vez por tick do game loop (nunca dentro de um delay()).
InputState inputRead();

#endif // INPUT_H
