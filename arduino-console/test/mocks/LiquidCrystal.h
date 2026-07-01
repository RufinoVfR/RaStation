#pragma once
#include <string.h>
#include <stdio.h>

class LiquidCrystal {
public:
  char screen[2][17]; // 2 linhas, 16 chars + null
  int cursorCol = 0, cursorRow = 0;

  LiquidCrystal(int rs, int en, int d4, int d5, int d6, int d7) {
    clear();
  }
  void begin(int cols, int rows) {}
  void clear() {
    for (int r = 0; r < 2; r++) {
      for (int c = 0; c < 16; c++) screen[r][c] = ' ';
      screen[r][16] = '\0';
    }
    cursorCol = cursorRow = 0;
  }
  void setCursor(int col, int row) { cursorCol = col; cursorRow = row; }
  void print(const char* s) {
    while (*s && cursorCol < 16) screen[cursorRow][cursorCol++] = *s++;
  }
  void print(int n) {
    char buf[8]; sprintf(buf, "%d", n);
    print(buf);
  }
  void write(uint8_t c) {
    if (cursorCol < 16) screen[cursorRow][cursorCol++] = (char)c;
  }
  void createChar(uint8_t slot, uint8_t* map) {}

  // Utilitário de teste: imprime o estado atual do LCD
  void dump() {
    printf("+----------------+\n");
    printf("|%s|\n", screen[0]);
    printf("|%s|\n", screen[1]);
    printf("+----------------+\n");
  }
  // Retorna true se a string aparece em qualquer linha
  bool contains(const char* s) {
    return strstr(screen[0], s) || strstr(screen[1], s);
  }
};
