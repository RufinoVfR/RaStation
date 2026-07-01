# ELETRONICA.md — Documentação de Hardware
## Projeto: Arduino Console — Mini Console de Jogos LCD

> **Para o desenvolvedor de software (Claude Code):**
> Este documento descreve toda a camada de hardware do projeto.
> Antes de escrever qualquer linha de código que interaja com periféricos,
> leia este arquivo integralmente. As restrições aqui descritas são físicas
> e não podem ser contornadas via software.

---

## 1. Visão Geral do Sistema

```
                    ┌─────────────────────────────────┐
                    │         Arduino Uno R3           │
                    │         ATmega328P               │
                    │         16MHz / 5V               │
                    │                                  │
   LCD 16x2 ───────►  D2, D3, D4, D5, D11, D12       │
   Botão ESQ ──────►  D6                              │
   Botão DIR ──────►  D7                              │
   Botão CIMA ─────►  D8                              │
   Botão BAIXO ────►  D9                              │
   Buzzer ─────────►  D10                             │
                    │                                  │
                    │  5V  ──► alimentação geral       │
                    │  GND ──► referência geral        │
                    └─────────────────────────────────┘
```

---

## 2. Inventário de Componentes

| Componente | Modelo/Tipo | Tensão | Quantidade |
|---|---|---|---|
| Microcontrolador | Arduino Uno R3 (ATmega328P) | 5V | 1 |
| Display | LCD 16x2 (HD44780 compatível) | 5V | 1 |
| Botões | Push-button 4 pinos | 5V | 4 |
| Resistores pull-down | 10kΩ | — | 4 |
| Potenciômetro | 10kΩ linear | 5V | 1 |
| Buzzer | Passivo (requer sinal PWM) | 5V | 1 |
| Protoboard | Full size (630 furos) | — | 1 |
| Jumpers | Macho-Macho variados | — | ~25 |

> **⚠️ CRÍTICO PARA O SOFTWARE:**
> O buzzer é **passivo** — ele exige um sinal de frequência variável via `tone()`.
> Buzzer ativo responderia a apenas HIGH/LOW, o que **não é o caso aqui**.
> Usar `digitalWrite()` no buzzer passivo não produz som.

---

## 3. Protoboard — Estrutura e Trilhos

```
  + ●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●  ← 5V  (todos conectados)
  - ●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●  ← GND (todos conectados)

    a b c d e   f g h i j     ← colunas (cada LINHA é conectada internamente)
  1 ● ● ● ● ●│  ● ● ● ● ●
  2 ● ● ● ● ●│  ● ● ● ● ●    ← canal central (separa a-e de f-j)
  3 ● ● ● ● ●│  ● ● ● ● ●
  ...
  - ●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●  ← GND (todos conectados)
  + ●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●─●  ← 5V  (todos conectados)
```

**Regras de conexão da protoboard:**
- Furos na **mesma linha horizontal** (ex: a1, b1, c1, d1, e1) → **conectados entre si**
- Furos em **linhas diferentes** (ex: a1 e a2) → **não conectados**
- Lado **a-e** e lado **f-j** → **nunca conectados** (separados pelo canal)
- Trilhos **+ e −** → cada trilho é uma linha contínua horizontal

**Como o 5V e GND chegam na protoboard:**
```
Arduino 5V  ──► qualquer furo do trilho vermelho (+)
Arduino GND ──► qualquer furo do trilho azul    (-)
```
A partir daí, qualquer componente que precise de 5V ou GND
usa um jumper curto até o trilho correspondente.

---

## 4. Display LCD 16x2

### 4.1 Características físicas
- **Resolução:** 16 colunas × 2 linhas = 32 células de caractere
- **Cada célere:** matriz de pixels 5×8
- **Custom characters:** 8 slots (endereços 0-7) — redefiníveis por software
- **Interface:** modo 4-bit (apenas D4-D7 usados — D0-D3 ficam desconectados)
- **Controlador:** HD44780 (ou compatível) — amplamente documentado

### 4.2 Pinagem física do LCD

```
Pinos do LCD vistos de frente (pino 1 à ESQUERDA):

 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
VSS VDD  V0  RS  RW  EN  D0  D1  D2  D3  D4  D5  D6  D7   A   K
GND 5V  POT D12 GND D11  ─   ─   ─   ─   D5  D4  D3  D2  5V GND
```

### 4.3 Conexões físicas realizadas

| Pino LCD | Nome | Função | Conectado em | Notas |
|---|---|---|---|---|
| 1 | VSS | GND do display | Trilho GND | Alimentação |
| 2 | VDD | 5V do display | Trilho 5V | Alimentação |
| 3 | V0 | Contraste | Pino MEIO do potenciômetro | Ver seção 5 |
| 4 | RS | Register Select | D12 do Arduino | Dados vs Comando |
| 5 | RW | Read/Write | Trilho GND | Fixo em escrita |
| 6 | EN | Enable | D11 do Arduino | Clock de dados |
| 7 | D0 | Dado bit 0 | **NÃO CONECTADO** | Modo 4-bit |
| 8 | D1 | Dado bit 1 | **NÃO CONECTADO** | Modo 4-bit |
| 9 | D2 | Dado bit 2 | **NÃO CONECTADO** | Modo 4-bit |
| 10 | D3 | Dado bit 3 | **NÃO CONECTADO** | Modo 4-bit |
| 11 | D4 | Dado bit 4 | D5 do Arduino | Dados |
| 12 | D5 | Dado bit 5 | D4 do Arduino | Dados |
| 13 | D6 | Dado bit 6 | D3 do Arduino | Dados |
| 14 | D7 | Dado bit 7 | D2 do Arduino | Dados |
| 15 | A | Backlight + | Trilho 5V | LED backlight |
| 16 | K | Backlight − | Trilho GND | LED backlight |

### 4.4 Como inicializar no código

```cpp
#include <LiquidCrystal.h>

// LiquidCrystal(RS, EN, D4, D5, D6, D7)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  lcd.begin(16, 2); // 16 colunas, 2 linhas — SEMPRE assim
}
```

> **⚠️ ATENÇÃO:** O pino RW está fixado em GND no hardware.
> Isso significa que o LCD está **sempre em modo escrita**.
> Nunca tente ler o LCD via software — o hardware não permite.

### 4.5 Restrições de performance do LCD

| Operação | Tempo real | Impacto no software |
|---|---|---|
| `lcd.clear()` | ~2ms | **EVITAR em game loop** — trava por 2ms |
| `lcd.setCursor()` + `lcd.print()` | ~100μs | Aceitável por célula |
| Redesenho completo célula a célula | ~3ms | Limite máximo aceitável |

**Regra de ouro:** nunca chamar `lcd.clear()` durante o jogo.
Redesenhe apenas as células que mudaram entre frames.

---

## 5. Potenciômetro (Controle de Contraste)

### 5.1 Função
Controla a tensão no pino V0 do LCD, ajustando o contraste do display.
Sem ele (ou com V0 flutuante), o texto pode não aparecer.

### 5.2 Conexões

```
Trilho 5V  ──► pino ESQUERDO do potenciômetro
Trilho GND ──► pino DIREITO do potenciômetro
Pino LCD 3 ──► pino CENTRAL (saída variável)
```

### 5.3 Posicionamento na protoboard
O pino central do potenciômetro está na **mesma linha numerada**
que o pino 3 do LCD — conectados internamente pela protoboard,
sem jumper adicional necessário.

### 5.4 Impacto no software
**Nenhum.** O potenciômetro é ajuste físico manual.
O software não precisa ler nem controlar o contraste.
Não há código relacionado a este componente.

---

## 6. Botões Push-button

### 6.1 Estrutura física do botão

```
Vista superior do botão (4 pinos):

    A ──┬── A
        │          ← pressionar fecha o circuito entre A e B
    B ──┴── B

A e A são o mesmo ponto.
B e B são o mesmo ponto.
```

O botão **cavalga o canal central** da protoboard:
- Pinos A ficam no lado **a-e**
- Pinos B ficam no lado **f-j**

### 6.2 Circuito pull-down de cada botão

```
Trilho 5V
    │
 [botão]  ← cavalga o canal (A no lado a-e, B no lado f-j)
    │
    ├──── jumper ──► pino digital do Arduino
    │
 [10kΩ]   ← resistor pull-down (mesma linha do pino B)
    │
Trilho GND
```

**Comportamento elétrico:**
- Botão **solto:** pino digital lê **0V (LOW)** — resistor puxa pro GND
- Botão **pressionado:** pino digital lê **5V (HIGH)** — corrente flui direto

### 6.3 Mapeamento de pinos

| Botão | Função no jogo | Pino Arduino | Resistor |
|---|---|---|---|
| Esquerda | Snake: esq / Invaders: esq / Menu: navegar | D6 | 10kΩ → GND |
| Direita | Snake: dir / Invaders: dir / Menu: navegar | D7 | 10kΩ → GND |
| Cima | Snake: cima / Pong: cima / Invaders: atirar / Menu: confirmar | D8 | 10kΩ → GND |
| Baixo | Snake: baixo / Pong: baixo | D9 | 10kΩ → GND |

### 6.4 Como ler no código

```cpp
// No setup():
pinMode(6, INPUT);  // ESQ
pinMode(7, INPUT);  // DIR
pinMode(8, INPUT);  // CIMA
pinMode(9, INPUT);  // BAIXO

// No loop():
bool esq   = digitalRead(6) == HIGH;
bool dir   = digitalRead(7) == HIGH;
bool cima  = digitalRead(8) == HIGH;
bool baixo = digitalRead(9) == HIGH;
```

### 6.5 Debounce — OBRIGATÓRIO

Botões físicos causam "bouncing" — múltiplas leituras em millisegundos
ao apertar/soltar. Sem debounce, um aperto vira 5-20 leituras HIGH.

**Implementar debounce por software de 50ms usando millis():**

```cpp
// Exemplo de estrutura — implemente em input.cpp
struct Button {
  uint8_t pin;
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
};

// Um aperto só é válido se o estado HIGH persistiu por 50ms
const unsigned long DEBOUNCE_DELAY = 50;
```

> **⚠️ NUNCA use `delay()` para debounce** — trava o game loop inteiro.

---

## 7. Buzzer Passivo

### 7.1 Tipo e comportamento
- **Passivo** — requer sinal de frequência variável para vibrar
- Diferente do buzzer ativo (que toca com apenas HIGH/LOW)
- Responde à função `tone(pino, frequencia)` do Arduino
- Para silenciar: `noTone(pino)`

### 7.2 Conexão

```
Arduino D10 ──► pino positivo do buzzer
Trilho GND  ──► pino negativo do buzzer
```

Buzzer não tem polaridade crítica, mas a convenção acima é padrão.

### 7.3 Como usar no código

```cpp
// Toca nota Lá (440Hz) por 200ms — NÃO BLOQUEANTE
tone(10, 440, 200);

// Toca indefinidamente até noTone()
tone(10, 440);
noTone(10);

// ⚠️ NUNCA fazer isso — bloqueia o loop:
tone(10, 440);
delay(200);      // PROIBIDO
noTone(10);
```

**Implementação correta (não bloqueante):**
```cpp
// Controle por millis() — mesma lógica do game loop
unsigned long soundStart = 0;
int soundDuration = 0;

void startTone(int freq, int duration) {
  tone(10, freq);
  soundStart = millis();
  soundDuration = duration;
}

void updateSound(unsigned long now) {
  if (soundDuration > 0 && now - soundStart >= soundDuration) {
    noTone(10);
    soundDuration = 0;
  }
}
```

### 7.4 Frequências de referência

| Nota | Frequência | Nota | Frequência |
|---|---|---|---|
| C4 (Dó) | 262 Hz | C5 | 523 Hz |
| D4 (Ré) | 294 Hz | D5 | 587 Hz |
| E4 (Mi) | 330 Hz | E5 | 659 Hz |
| F4 (Fá) | 349 Hz | F5 | 698 Hz |
| G4 (Sol) | 392 Hz | G5 | 784 Hz |
| A4 (Lá) | 440 Hz | A5 | 880 Hz |
| B4 (Si) | 494 Hz | B5 | 988 Hz |

---

## 8. Pinos do Arduino — Mapa Completo

```
                    Arduino Uno R3
                   ┌─────────────┐
             Reset─┤             ├─ D13 (SCK)     — LIVRE
              3.3V─┤             ├─ D12 (MISO)    → LCD RS
                5V─┤             ├─ D11 (MOSI/PWM)→ LCD EN
               GND─┤             ├─ D10 (PWM)     → Buzzer
               GND─┤             ├─ D9  (PWM)     → Botão BAIXO
              Vin ─┤             ├─ D8             → Botão CIMA
                   │             ├─ D7             → Botão DIR
              A0  ─┤             ├─ D6             → Botão ESQ
              A1  ─┤             ├─ D5  (PWM)     → LCD D4
              A2  ─┤             ├─ D4             → LCD D5
              A3  ─┤             ├─ D3  (PWM)     → LCD D6
              A4  ─┤             ├─ D2             → LCD D7
              A5  ─┤             ├─ D1  (TX)      — LIVRE
                   └─────────────┘─ D0  (RX)      — LIVRE
```

**Pinos livres disponíveis para expansão futura:**
- **D0, D1** — evitar (conflito com Serial/USB durante upload)
- **D13** — tem LED onboard, usar com cautela
- **A0, A1, A2, A3, A4, A5** — todos livres (analógicos/digitais)

> **Joystick KY-023 (expansão futura):**
> VRX → A0, VRY → A1, SW → D13
> Já mapeado para não conflitar com pinos em uso.

---

## 9. Restrições de Hardware para o Software

### 9.1 Memória — CRÍTICO

| Recurso | Total | Limite seguro | Se ultrapassar |
|---|---|---|---|
| RAM (SRAM) | 2.048 bytes | 1.600 bytes (78%) | Crashes e comportamentos aleatórios |
| Flash | 32.256 bytes | 29.000 bytes (90%) | Não compila |
| EEPROM | 1.024 bytes | — | Disponível para recordes |

**Técnicas obrigatórias de economia de RAM:**
```cpp
// 1. Strings na Flash, não na RAM
lcd.print(F("Game Over"));          // F() macro

// 2. Strings estáticas em PROGMEM
const char msg[] PROGMEM = "Snake";

// 3. char[] em vez de String
char buffer[17];                     // CORRETO
String str = "texto";               // PROIBIDO

// 4. Posições empacotadas em 1 byte
// X (0-15) nos bits 7-4, Y (0-1) nos bits 3-0
uint8_t pos = (x << 4) | y;
uint8_t x = pos >> 4;
uint8_t y = pos & 0x0F;
```

### 9.2 Timing — CRÍTICO

```
PROIBIDO em qualquer parte do código:
  delay()       — trava TUDO: botões, display, som
  delayMicroseconds() acima de 100μs — mesmo problema

CORRETO — toda espera via millis():
  unsigned long now = millis();
  if (now - lastEvent >= interval) {
    lastEvent = now;
    // executa ação
  }
```

### 9.3 Conflitos de pinos conhecidos

| Pino | Em uso | Conflito potencial |
|---|---|---|
| D0, D1 | Serial USB | Não usar durante operação normal |
| D10 | Buzzer (tone) | tone() desativa PWM do D10 enquanto ativo |
| D11 | LCD EN | Sem conflito |
| D13 | Livre | LED onboard acende junto se usado como OUTPUT |

### 9.4 Custom Characters do LCD

- **Total disponível:** 8 slots (endereços 0–7)
- **Cada caractere:** matriz 5 colunas × 8 linhas de pixels
- **Limitação:** os 8 slots são compartilhados globalmente
- **Trocar entre jogos:** chamar `lcd.createChar()` novamente substitui o slot

```cpp
// Exemplo de custom char — bloco sólido
byte blocoSolido[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
lcd.createChar(0, blocoSolido); // slot 0
lcd.write(byte(0));             // exibe no cursor atual
```

---

## 10. Diagrama de Conexões Completo

```
Arduino Uno R3
      │
      ├─ 5V ──────────────────────────► Trilho + (vermelho) da protoboard
      │                                        │
      │                                        ├──► LCD pino 2  (VDD)
      │                                        ├──► LCD pino 15 (A - backlight)
      │                                        ├──► Potenciômetro pino direito
      │                                        └──► Botões (via pino A de cada botão)
      │
      ├─ GND ─────────────────────────► Trilho - (azul) da protoboard
      │                                        │
      │                                        ├──► LCD pino 1  (VSS)
      │                                        ├──► LCD pino 5  (RW)
      │                                        ├──► LCD pino 16 (K - backlight)
      │                                        ├──► Potenciômetro pino esquerdo
      │                                        ├──► Buzzer pino negativo
      │                                        └──► Resistores 10kΩ (pull-down)
      │
      ├─ D2  ──────────────────────────► LCD pino 14 (D7)
      ├─ D3  ──────────────────────────► LCD pino 13 (D6)
      ├─ D4  ──────────────────────────► LCD pino 12 (D5)
      ├─ D5  ──────────────────────────► LCD pino 11 (D4)
      │
      ├─ D6  ──────────────────────────► Botão ESQUERDA (pino B)
      ├─ D7  ──────────────────────────► Botão DIREITA  (pino B)
      ├─ D8  ──────────────────────────► Botão CIMA     (pino B)
      ├─ D9  ──────────────────────────► Botão BAIXO    (pino B)
      │
      ├─ D10 ──────────────────────────► Buzzer pino positivo
      │
      ├─ D11 ──────────────────────────► LCD pino 6  (EN)
      └─ D12 ──────────────────────────► LCD pino 4  (RS)

Potenciômetro:
  pino CENTRAL ────────────────────────► LCD pino 3  (V0 - contraste)
  (na mesma linha da protoboard que o pino 3 do LCD)
```

---

## 11. Validação do Hardware — Checklist

Antes de rodar qualquer código de jogo, confirme que cada item funciona:

```
[ ] Backlight do LCD acende ao ligar o Arduino
[ ] Texto aparece no LCD após girar o potenciômetro
[ ] Botão ESQ   → Serial Monitor mostra "ESQUERDA"
[ ] Botão DIR   → Serial Monitor mostra "DIREITA"
[ ] Botão CIMA  → Serial Monitor mostra "CIMA"
[ ] Botão BAIXO → Serial Monitor mostra "BAIXO"
[ ] Buzzer emite som ao chamar tone(10, 440, 200)
[ ] Nenhum botão fica "preso" em HIGH sem ser pressionado
```

Se algum item falhar, **não avançar para o código dos jogos.**
Resolver o problema de hardware primeiro.

---

## 12. Problemas Comuns e Soluções

| Sintoma | Causa provável | Solução |
|---|---|---|
| LCD acende mas sem texto | Contraste errado | Girar o potenciômetro |
| LCD sem texto e sem backlight | 5V ou GND do LCD solto | Verificar pinos 1, 2, 15, 16 |
| Texto aparece mas embaralhado | D2-D5 ou D11/D12 com mau contato | Verificar jumpers dos dados |
| Botão não responde | Resistor pull-down solto | Verificar resistor na mesma linha do pino B |
| Botão sempre pressionado | Fio de 5V ou resistor invertidos | Verificar circuito pull-down |
| Buzzer não emite som | `digitalWrite()` em vez de `tone()` | Usar `tone(10, frequencia)` |
| Arduino reinicia sozinho | RAM estourada | Verificar uso de memória com `pio run` |
| Caracteres estranhos no LCD | `String` fragmentando heap | Substituir por `char[]` + `F()` |
