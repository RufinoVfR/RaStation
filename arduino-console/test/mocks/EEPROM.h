#pragma once
#include <stdint.h>
#include <stddef.h>

// static local dentro de função inline: garante uma única cópia da "EEPROM"
// compartilhada entre todos os .cpp linkados no binário de teste (ver
// comentário equivalente em Arduino.h).
inline uint8_t* _eepromState() {
  static uint8_t data[1024] = {0};
  return data;
}

struct EEPROMClass {
  uint8_t read(int addr) { return _eepromState()[addr]; }
  void write(int addr, uint8_t val) { _eepromState()[addr] = val; }
  template<typename T>
  void get(int addr, T& val) {
    uint8_t* p = (uint8_t*)&val;
    for (size_t i = 0; i < sizeof(T); i++) p[i] = _eepromState()[addr + i];
  }
  template<typename T>
  void put(int addr, const T& val) {
    const uint8_t* p = (const uint8_t*)&val;
    for (size_t i = 0; i < sizeof(T); i++) _eepromState()[addr + i] = p[i];
  }
  void clear() { for (int i = 0; i < 1024; i++) _eepromState()[i] = 0xFF; }
};
static EEPROMClass EEPROM;
