#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal.h>
#include "config.h"
#include "scores.h"

extern LiquidCrystal lcd;

void menuInit();

// Chamado a cada tick do game loop. Retorna o GameState a iniciar
// (STATE_SNAKE/PONG/INVADERS quando o jogador confirma um jogo),
// ou STATE_MENU quando ainda não há seleção.
GameState menuUpdate(unsigned long now);

int getSelectedIndex();

// Mostra a tela de fim de jogo (com "NOVO RECORDE!" por 2s se for o caso)
// e espera CIMA para voltar ao menu (tratado dentro de menuUpdate()).
void showGameOver(GameType game, int score);

#endif // MENU_H
