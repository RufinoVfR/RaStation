#ifndef SOUND_H
#define SOUND_H

#include "config.h"

enum SoundEffect {
  SFX_MENU_MOVE,
  SFX_MENU_SELECT,
  SFX_SNAKE_EAT,
  SFX_SNAKE_DEAD,
  SFX_PONG_HIT,
  SFX_PONG_WALL,
  SFX_PONG_SCORE,
  SFX_INV_SHOOT,
  SFX_INV_HIT,
  SFX_INV_DEAD,
  SFX_GAMEOVER,
  SFX_LEVELUP,
};

void soundInit();

// Enfileira as notas do efeito. Um novo som sempre interrompe o que estiver
// tocando (prioridade ao mais recente) — nunca bloqueia, retorna na hora.
void playSound(SoundEffect sfx);

// Chamado a cada tick do loop principal. Nunca usa delay(): controla a
// duração de cada nota comparando millis() com o instante em que começou.
void updateSound(unsigned long now);

bool isSoundPlaying(); // true se ainda há som tocando ou na fila

// API de baixo nível, exposta para os testes de caixa branca.
void soundResetForTest();
void soundEnqueueForTest(int freq, int duration);
uint8_t soundGetQueueLength();
bool soundIsToneActive(); // true enquanto a nota atual não terminou sua duração

#endif // SOUND_H
