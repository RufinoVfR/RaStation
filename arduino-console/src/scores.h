#ifndef SCORES_H
#define SCORES_H

#include "config.h"

enum GameType { GAME_SNAKE, GAME_PONG, GAME_INVADERS };

int loadHighScore(GameType game);
void saveHighScore(GameType game, int score); // só grava se for maior que o atual
bool isNewRecord(GameType game, int score);

#endif // SCORES_H
