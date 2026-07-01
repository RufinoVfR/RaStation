#ifndef INPUT_H
#define INPUT_H

#include "config.h"

void inputInit();

// Evento único (edge-triggered) com debounce de 50ms.
// Retorna o pino do botão cujo aperto acabou de ser confirmado, ou BTN_NONE.
// Enquanto o botão continuar pressionado, chamadas seguintes retornam BTN_NONE
// (não repete). Prioridade quando há mais de um pressionado ao mesmo tempo:
// CIMA > BAIXO > ESQ > DIR.
uint8_t readButtons(unsigned long now);

// Estado atual (nível, já debounced) do botão pressionado, ou BTN_NONE.
// Não consome evento — útil para feedback visual contínuo (ex.: tela de teste).
// Reflete o estado calculado na última chamada de readButtons().
uint8_t getHeldButton();

#endif // INPUT_H
