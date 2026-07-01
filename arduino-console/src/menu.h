#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal.h>
#include "../include/config.h"
#include "input.h"

// Inicializa o índice de seleção do menu.
void menuSetup();

// Atualiza a seleção do menu com base no input do usuário e desenha na tela.
// Retorna STATE_MENU enquanto o jogador navega, ou o estado do jogo escolhido
// quando o botão de confirmação (DIR) é pressionado.
GameState menuUpdate(LiquidCrystal &lcd, const InputState &input);

#endif // MENU_H
