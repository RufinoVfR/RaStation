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

// Botão único de maior prioridade quando vários são pressionados ao mesmo
// tempo. ": uint8_t" força o enum a ocupar 1 byte em vez do int padrão.
enum Botao : uint8_t {
  BOTAO_NENHUM,
  BOTAO_CIMA,
  BOTAO_BAIXO,
  BOTAO_ESQUERDA,
  BOTAO_DIREITA
};

// Configura os pinos dos botões como entrada.
void inputSetup();

// Lê os 4 botões com debounce e devolve o estado atual.
// Deve ser chamada uma vez por tick do game loop (nunca dentro de um delay()).
InputState inputRead();

// Resolve qual botão reportar quando mais de um está pressionado ao mesmo
// tempo. Prioridade: cima > baixo > esquerda > direita.
Botao inputBotaoPrioritario(const InputState &input);

#endif // INPUT_H
