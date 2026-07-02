#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// Constantes
#define HIGH 1
#define LOW  0
#define INPUT  0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Pinos analógicos usados como digitais (mesma numeração do Arduino Uno)
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19

// PROGMEM/flash: no PC não existe memória de programa separada da RAM,
// então essas macros só precisam "não fazer nada" além de manter a mesma
// assinatura de tipos que o código real (AVR) usa via <avr/pgmspace.h>.
#define PROGMEM
#define PSTR(s) (s)
#define F(s) (s)
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#define strlen_P(s) strlen(s)
#define strcpy_P(dst, src) strcpy(dst, src)
#define memcpy_P(dst, src, n) memcpy(dst, src, n)

// Este mock é incluído por vários .cpp (src/*.cpp e test/*.cpp) que são
// linkados juntos no mesmo binário de teste. Uma variável `static` no escopo
// do arquivo geraria uma cópia por unidade de compilação, e setPin() num
// arquivo não seria visto pelo digitalRead() de outro. Por isso o estado
// mutável fica em `static` local dentro de função `inline`: o linker funde
// as instâncias da função e todas as unidades passam a compartilhar a mesma
// variável.

// millis() simulado — controlável nos testes
inline unsigned long& _millisState() {
  static unsigned long value = 0;
  return value;
}
inline unsigned long millis() { return _millisState(); }
inline void advanceTime(unsigned long ms) { _millisState() += ms; }
inline void resetTime() { _millisState() = 0; }

// Pinos simulados
inline int* _pinState() {
  static int state[20] = {0};
  return state;
}
inline void pinMode(int pin, int mode) {}
inline int digitalRead(int pin) { return _pinState()[pin]; }
inline void digitalWrite(int pin, int val) { _pinState()[pin] = val; }
inline void setPin(int pin, int val) { _pinState()[pin] = val; }

// Analógico simulado
inline int* _analogState() {
  static int state[6] = {512, 512, 512, 512, 512, 512};
  return state;
}
inline int analogRead(int pin) { return _analogState()[pin]; }
inline void setAnalog(int pin, int val) { _analogState()[pin] = val; }

// Som simulado
struct ToneState { int lastFreq = 0; bool active = false; };
inline ToneState& _toneState() {
  static ToneState state;
  return state;
}
inline void tone(int pin, int freq, int dur = 0) {
  _toneState().lastFreq = freq;
  _toneState().active = true;
}
inline void noTone(int pin) { _toneState().active = false; }
inline int getLastToneFreq() { return _toneState().lastFreq; }
inline bool isToneActive() { return _toneState().active; }

// Misc
inline long random(long max) { return rand() % max; }
inline long random(long min, long max) { return min + rand() % (max - min); }
inline void randomSeed(unsigned long seed) { srand(seed); }
