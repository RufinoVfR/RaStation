#ifndef CONFIG_H
#define CONFIG_H

// LCD 16x2 - LiquidCrystal(RS, EN, D4, D5, D6, D7)
// Mapeamento original: RS=D12, EN=D11, D4=D5, D5=D4, D6=D3, D7=D2.
#define LCD_RS 12
#define LCD_EN 11
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

// Botões push-button (pull-down externo, solto = LOW, pressionado = HIGH)
// Os valores também servem como identificador de botão retornado por readButtons()
#define BTN_ESQ   6
#define BTN_DIR   7
#define BTN_CIMA  8
#define BTN_BAIXO 9
#define BTN_NONE  0

// Buzzer passivo (requer tone())
#define BUZZER_PIN 10

// Dimensões do display (LCD 20x4)
#define LARGURA 20
#define ALTURA  4

// Estados do sistema
enum GameState {
  STATE_MENU,
  STATE_SNAKE,
  STATE_PONG,
  STATE_INVADERS,
  STATE_GAMEOVER
};

#endif // CONFIG_H
