#pragma once
#include <string.h>
#include <stdio.h>
#include "config.h"

class LiquidCrystal {
public:
  char screen[ALTURA][LARGURA + 1]; // ALTURA linhas, LARGURA chars + null
  int cursorCol = 0, cursorRow = 0;

  LiquidCrystal(int rs, int en, int d4, int d5, int d6, int d7) {
    clear();
  }
  void begin(int cols, int rows) {}
  void clear() {
    for (int r = 0; r < ALTURA; r++) {
      for (int c = 0; c < LARGURA; c++) screen[r][c] = ' ';
      screen[r][LARGURA] = '\0';
    }
    cursorCol = cursorRow = 0;
  }
  void setCursor(int col, int row) { cursorCol = col; cursorRow = row; }
  void print(const char* s) {
    while (*s && cursorCol < LARGURA) screen[cursorRow][cursorCol++] = *s++;
  }
  void print(int n) {
    char buf[8]; sprintf(buf, "%d", n);
    print(buf);
  }
  void write(uint8_t c) {
    if (cursorCol < LARGURA) screen[cursorRow][cursorCol++] = (char)c;
  }
  void createChar(uint8_t slot, uint8_t* map) {}

  // Utilitário de teste: imprime o estado atual do LCD
  void dump() {
    printf("+");
    for (int c = 0; c < LARGURA; c++) printf("-");
    printf("+\n");
    for (int r = 0; r < ALTURA; r++) printf("|%s|\n", screen[r]);
    printf("+");
    for (int c = 0; c < LARGURA; c++) printf("-");
    printf("+\n");
  }
  // Retorna true se a string aparece em qualquer linha
  bool contains(const char* s) {
    for (int r = 0; r < ALTURA; r++) {
      if (strstr(screen[r], s)) return true;
    }
    return false;
  }
};
