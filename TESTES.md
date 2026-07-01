# TESTES.md — Estratégia de Testes do Projeto
## Arduino Console — Mini Console de Jogos LCD

> **Para o Claude Code:**
> Este documento define toda a estratégia de testes do projeto.
> Leia-o integralmente antes de qualquer etapa do BACKLOG.
> Todo `### BANCO DE TESTES` do BACKLOG.md referencia este documento.

---

## 1. Por que testar software embarcado?

Arduino e sistemas embarcados têm características que tornam bugs muito
mais difíceis de detectar do que em software convencional:

| Problema | Consequência no Arduino |
|---|---|
| RAM estourada | Crashes silenciosos, comportamento aleatório |
| `delay()` no loop | Botões travados, display congelado |
| Ponteiro corrompido | Arduino reinicia sozinho sem mensagem de erro |
| Lógica de colisão errada | Bug só aparece após 2 minutos de jogo |
| `String` fragmentando heap | Crash imprevisível e intermitente |

Testar **antes de gravar no Arduino** economiza dezenas de ciclos de
"gravar → testar → perceber o bug → corrigir → gravar de novo".

---

## 2. Caixa Branca vs Caixa Preta

### 2.1 Testes de Caixa Branca (White-box)

Testam a **estrutura interna** do código — o testador conhece a implementação.

**Aplicação neste projeto:**
- Testar funções individualmente (unit tests)
- Verificar todos os caminhos de uma lógica (branches)
- Confirmar operações matemáticas e de bits
- Garantir que estruturas de dados funcionam corretamente

**Exemplos práticos:**
```
✓ packPos(3, 1) retorna 0x31
✓ unpackCol(0x31) retorna 3
✓ unpackRow(0x31) retorna 1
✓ circularBuffer.push() incrementa length corretamente
✓ debounce ignora transições mais rápidas que 50ms
✓ snake não aceita direção oposta à atual (180°)
✓ bitmask de inimigos: matar inimigo 2 → bit 2 vira 0
```

### 2.2 Testes de Caixa Preta (Black-box)

Testam o **comportamento externo** — o testador trata o sistema como
uma caixa: entra input, verifica output, sem olhar para dentro.

**Aplicação neste projeto:**
- Simular sequências de botões e verificar estado resultante
- Verificar que o jogo termina nas condições certas
- Confirmar que pontuação está correta após N ações
- Validar transições de estado (menu → jogo → game over → menu)

**Exemplos práticos:**
```
✓ Snake: após 5 comidas, pontuação == 50
✓ Pong: após placar 5x0, GameState == GAMEOVER
✓ Menu: DIR × 3 retorna ao item 0 (wraparound com 3 jogos)
✓ Invaders: matar todos os 6 inimigos → inicia onda 2
✓ Input: sequência HIGH(50ms)→LOW → detecta exatamente 1 aperto
```

---

## 3. O que o Claude Code consegue testar automaticamente

### 3.1 Sem Arduino conectado (100% automático)

```
CATEGORIA A — Análise Estática (grep/awk no código fonte)
  ✓ Ausência de delay() em qualquer arquivo
  ✓ Ausência de String (maiúsculo) como tipo de variável
  ✓ Presença de F() em literais passados ao lcd.print()
  ✓ Include guards em todos os .h
  ✓ Uso de RAM e Flash (via `pio run`)

CATEGORIA B — Unit Tests nativos (pio test -e native)
  ✓ Toda lógica pura de jogo (sem hardware)
  ✓ Estruturas de dados (buffer circular, bitmask)
  ✓ Cálculos de posição, colisão, pontuação
  ✓ Máquinas de estado (transições válidas e inválidas)
  ✓ Lógica de debounce com millis() simulado
  ✓ Fila de som (enqueue, dequeue, overflow)
  ✓ Operações EEPROM simuladas

CATEGORIA C — Simulação ASCII
  ✓ Renderização frame-a-frame do estado do jogo
  ✓ Valida visualmente a lógica antes de gravar
```

### 3.2 Com Arduino conectado (verificação manual)

```
CATEGORIA D — Hardware
  ✓ Botões respondem corretamente
  ✓ LCD exibe texto sem embaralhamento
  ✓ Buzzer emite sons no tempo certo
  ✓ Performance: jogo roda sem engasgos
```

---

## 4. Infraestrutura de Testes

### 4.1 Configuração do platformio.ini

```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
monitor_speed = 9600

; Ambiente nativo para rodar testes no PC (sem Arduino)
[env:native]
platform = native
build_flags =
  -std=c++11
  -DNATIVE_TEST
  -DUNIT_TEST
lib_extra_dirs = test/mocks
```

### 4.2 Estrutura de pastas de testes

```
arduino-console/
├── src/                     ← código de produção
├── test/
│   ├── mocks/               ← stubs das APIs do Arduino
│   │   ├── Arduino.h        ← millis(), tone(), digitalRead(), etc.
│   │   ├── LiquidCrystal.h  ← mock do LCD
│   │   └── EEPROM.h         ← mock da EEPROM
│   ├── test_input/
│   │   └── test_input.cpp
│   ├── test_snake/
│   │   └── test_snake.cpp
│   ├── test_pong/
│   │   └── test_pong.cpp
│   ├── test_invaders/
│   │   └── test_invaders.cpp
│   ├── test_sound/
│   │   └── test_sound.cpp
│   └── test_scores/
│       └── test_scores.cpp
```

### 4.3 Mocks das APIs do Arduino

Os mocks permitem rodar código Arduino no PC. Claude Code deve criá-los
e mantê-los ao longo do projeto.

```cpp
// test/mocks/Arduino.h
#pragma once
#include <stdint.h>
#include <stdlib.h>

// Constantes
#define HIGH 1
#define LOW  0
#define INPUT  0
#define OUTPUT 1
#define INPUT_PULLUP 2

// millis() simulado — controlável nos testes
static unsigned long _mock_millis = 0;
inline unsigned long millis() { return _mock_millis; }
inline void advanceTime(unsigned long ms) { _mock_millis += ms; }
inline void resetTime() { _mock_millis = 0; }

// Pinos simulados
static int _pin_state[20] = {0};
inline void pinMode(int pin, int mode) {}
inline int digitalRead(int pin) { return _pin_state[pin]; }
inline void digitalWrite(int pin, int val) { _pin_state[pin] = val; }
inline void setPin(int pin, int val) { _pin_state[pin] = val; }

// Analógico simulado
static int _analog_state[6] = {512, 512, 512, 512, 512, 512};
inline int analogRead(int pin) { return _analog_state[pin]; }
inline void setAnalog(int pin, int val) { _analog_state[pin] = val; }

// Som simulado
static int _last_tone_freq = 0;
static bool _tone_active = false;
inline void tone(int pin, int freq, int dur = 0) {
  _last_tone_freq = freq;
  _tone_active = true;
}
inline void noTone(int pin) { _tone_active = false; }
inline int getLastToneFreq() { return _last_tone_freq; }
inline bool isToneActive() { return _tone_active; }

// Misc
inline long random(long max) { return rand() % max; }
inline long random(long min, long max) { return min + rand() % (max - min); }
inline void randomSeed(unsigned long seed) { srand(seed); }
```

```cpp
// test/mocks/LiquidCrystal.h
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
```

```cpp
// test/mocks/EEPROM.h
#pragma once
#include <stdint.h>

static uint8_t _eeprom[1024] = {0};

struct EEPROMClass {
  uint8_t read(int addr) { return _eeprom[addr]; }
  void write(int addr, uint8_t val) { _eeprom[addr] = val; }
  template<typename T>
  void get(int addr, T& val) {
    uint8_t* p = (uint8_t*)&val;
    for (size_t i = 0; i < sizeof(T); i++) p[i] = _eeprom[addr + i];
  }
  template<typename T>
  void put(int addr, const T& val) {
    const uint8_t* p = (const uint8_t*)&val;
    for (size_t i = 0; i < sizeof(T); i++) _eeprom[addr + i] = p[i];
  }
  void clear() { for (int i = 0; i < 1024; i++) _eeprom[i] = 0xFF; }
};
static EEPROMClass EEPROM;
```

### 4.4 Framework de testes: Unity

PlatformIO inclui Unity nativamente para testes embarcados.

```cpp
// Exemplo de arquivo de teste
#include <unity.h>
#include "Arduino.h"       // mock
#include "../../src/games/snake.h"

void setUp() {
  resetTime();             // reseta millis() simulado
}

void tearDown() {}         // limpeza após cada teste

void test_pack_position() {
  uint8_t pos = packPos(5, 1);
  TEST_ASSERT_EQUAL(5, unpackCol(pos));
  TEST_ASSERT_EQUAL(1, unpackRow(pos));
}

void test_snake_no_reverse() {
  Snake snake;
  snake.init();
  snake.setDirection(DIR_RIGHT);
  snake.setDirection(DIR_LEFT); // deve ser ignorado
  TEST_ASSERT_EQUAL(DIR_RIGHT, snake.getDirection());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_pack_position);
  RUN_TEST(test_snake_no_reverse);
  return UNITY_END();
}
```

### 4.5 Comandos de teste

```bash
# Rodar todos os testes nativos (sem Arduino)
pio test -e native

# Rodar testes de um módulo específico
pio test -e native --filter test_snake

# Compilar para Arduino (verifica memória)
pio run

# Análise estática (rodar após cada etapa)
grep -rn "delay(" src/ && echo "FALHA: delay() encontrado" || echo "OK: sem delay()"
grep -rn " String " src/ && echo "FALHA: String encontrado" || echo "OK: sem String"
```

---

## 5. Análise Estática Automática

Claude Code deve rodar estes checks após **cada implementação**:

```bash
#!/bin/bash
# Salvar como test/static_analysis.sh

echo "=== ANÁLISE ESTÁTICA ==="

# 1. delay() proibido
if grep -rn "delay(" src/ --include="*.cpp" --include="*.h" | grep -v "//"; then
  echo "❌ FALHA: delay() encontrado"
else
  echo "✅ sem delay()"
fi

# 2. String (tipo) proibido
if grep -rn "\bString\b" src/ --include="*.cpp" --include="*.h" | grep -v "//"; then
  echo "❌ FALHA: tipo String encontrado"
else
  echo "✅ sem String"
fi

# 3. lcd.print sem F() para literais
if grep -rn 'lcd\.print("' src/ --include="*.cpp"; then
  echo "❌ FALHA: lcd.print() com literal sem F()"
else
  echo "✅ F() usado corretamente"
fi

# 4. Include guards em todos os .h
for f in src/**/*.h include/*.h; do
  if ! grep -q "#ifndef" "$f" 2>/dev/null; then
    echo "❌ FALHA: $f sem include guard"
  fi
done
echo "✅ include guards verificados"

# 5. Memória
echo ""
echo "=== USO DE MEMÓRIA ==="
pio run 2>&1 | grep -E "RAM:|Flash:"
```

---

## 6. Template de Banco de Testes

Este é o template que aparece no final de cada etapa do BACKLOG.md:

```
### BANCO DE TESTES

Após implementar, execute obrigatoriamente na seguinte ordem:

ANÁLISE ESTÁTICA:
[ ] bash test/static_analysis.sh — zero falhas
[ ] pio run — RAM < [LIMITE_DA_ETAPA] bytes

TESTES UNITÁRIOS (Caixa Branca):
[ ] Criar test/test_[modulo]/test_[modulo].cpp
[ ] pio test -e native --filter test_[modulo]
[ ] Todos os testes passando (0 falhas)

FUNÇÕES OBRIGATORIAMENTE TESTADAS:
[ ] [função 1]: [cenário] → [resultado esperado]
[ ] [função 2]: [cenário] → [resultado esperado]
...

TESTES DE INTEGRAÇÃO (Caixa Preta):
[ ] [cenário de input] → [estado/output esperado]
[ ] [cenário de input] → [estado/output esperado]
...

SIMULAÇÃO ASCII:
[ ] Gerar e exibir N frames do estado do jogo/módulo

NÃO avançar para a próxima etapa com qualquer item em vermelho.
```

---

## 7. Política de Qualidade por Etapa

| Etapa | RAM máxima | Testes unitários | Cobertura mínima |
|---|---|---|---|
| 1 — Setup | 300 bytes | 0 (só estática) | 100% estática |
| 2 — Hello World | 600 bytes | debounce, prioridade | 100% input.cpp |
| 3 — Menu | 800 bytes | estados, wraparound | 100% menu.cpp |
| 4 — Snake | 1.200 bytes | colisão, buffer, bits | 100% snake lógica |
| 5 — Pong | 1.400 bytes | física, placar, CPU | 100% pong lógica |
| 6 — Invaders | 1.600 bytes | bitmask, tiro, ondas | 100% invaders lógica |
| 7 — Som | 1.700 bytes | fila, overflow, interrupção | 100% sound.cpp |
| 8 — Polish | 1.800 bytes | EEPROM, recorde, splash | 100% scores.cpp |

> **Regra:** se a RAM ultrapassar o limite da etapa, **parar e otimizar**
> antes de continuar. Não há margem de segurança acumulada.
