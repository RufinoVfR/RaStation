#ifndef CONFIG_H
#define CONFIG_H

// LCD 16x2 - LiquidCrystal(RS, EN, D4, D5, D6, D7)
// Usa A0-A5 (como pinos digitais) em vez de D2/D3/D4/D5/D11/D12 — um desses
// pinos digitais acabou danificado durante a montagem, então o LCD foi
// remapeado pros pinos analógicos, que funcionam normalmente como digitais.
#define LCD_RS A0
#define LCD_EN A1
#define LCD_D4 A2
#define LCD_D5 A3
#define LCD_D6 A4
#define LCD_D7 A5

// Botões push-button (pull-down externo, solto = LOW, pressionado = HIGH)
// Os valores também servem como identificador de botão retornado por readButtons()
#define BTN_ESQ   6
#define BTN_DIR   7
#define BTN_CIMA  8
#define BTN_BAIXO 9
#define BTN_NONE  0

// Buzzer passivo (requer tone())
#define BUZZER_PIN 10

// Dimensões do display
#define LARGURA 16
#define ALTURA  2

// Estados do sistema
enum GameState {
  STATE_MENU,
  STATE_SNAKE,
  STATE_PONG,
  STATE_INVADERS,
  STATE_GAMEOVER
};

#endif // CONFIG_H
