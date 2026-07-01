#ifndef CONFIG_H
#define CONFIG_H

// =====================================================
// Pinagem do LCD 16x2 (modo 4-bit)
// Ordem: RS, EN, D4, D5, D6, D7
// =====================================================
#define LCD_RS 12
#define LCD_EN 11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

// =====================================================
// Pinagem dos botões (pull-down externo, HIGH = pressionado)
// =====================================================
#define BTN_ESQ   6
#define BTN_DIR   7
#define BTN_CIMA  8
#define BTN_BAIXO 9

// =====================================================
// Constantes de jogo
// =====================================================
#define LARGURA 16 // colunas do LCD
#define ALTURA  2  // linhas do LCD

// Velocidade do game loop (ms por tick), usada com millis()
#define GAME_SPEED 300

// Debounce dos botões (ms)
#define DEBOUNCE_MS 50

// =====================================================
// Estados do sistema
// Compartilhado entre main.cpp e menu.cpp para permitir
// que o menu decida a transição de estado.
// =====================================================
enum GameState {
  STATE_MENU,
  STATE_SNAKE,
  STATE_PONG,
  STATE_INVADERS,
  STATE_GAMEOVER
};

#endif // CONFIG_H
