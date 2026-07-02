#include <Arduino.h>
#include <string.h>
#include "sound.h"

struct Note {
  int16_t freq;
  int16_t duration;
};

static const uint8_t QUEUE_CAPACITY = 8;

// Notas de cada efeito, em PROGMEM (ficam na Flash, não na RAM).
static const Note NOTES_MENU_MOVE[]   PROGMEM = { {440, 30} };
static const Note NOTES_MENU_SELECT[] PROGMEM = { {800, 50}, {1000, 50}, {1200, 50} };
static const Note NOTES_SNAKE_EAT[]   PROGMEM = { {880, 60} };
static const Note NOTES_SNAKE_DEAD[]  PROGMEM = { {392, 100}, {330, 100}, {262, 150} };
static const Note NOTES_PONG_HIT[]    PROGMEM = { {600, 40} };
static const Note NOTES_PONG_WALL[]   PROGMEM = { {400, 40} };
static const Note NOTES_PONG_SCORE[]  PROGMEM = { {523, 80}, {659, 80} };
static const Note NOTES_INV_SHOOT[]   PROGMEM = { {1000, 30} };
static const Note NOTES_INV_HIT[]     PROGMEM = { {700, 40} };
static const Note NOTES_INV_DEAD[]    PROGMEM = { {440, 100}, {330, 100} };
static const Note NOTES_GAMEOVER[]    PROGMEM = { {392, 150}, {330, 150}, {294, 150}, {262, 300} };
static const Note NOTES_LEVELUP[]     PROGMEM = { {523, 80}, {659, 80}, {784, 120} };

static Note queue[QUEUE_CAPACITY];
static uint8_t queueHead = 0;
static uint8_t queueCount = 0;

static bool toneActive = false;
static unsigned long toneStart = 0;
static int16_t toneDuration = 0;

static void queueClear() {
  queueHead = 0;
  queueCount = 0;
}

static void enqueueSingle(int16_t freq, int16_t duration) {
  if (queueCount >= QUEUE_CAPACITY) return; // fila cheia: descarta graciosamente
  uint8_t idx = (queueHead + queueCount) % QUEUE_CAPACITY;
  queue[idx].freq = freq;
  queue[idx].duration = duration;
  queueCount++;
}

static void enqueueFromProgmem(const Note* notes, uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    Note n;
    memcpy_P(&n, &notes[i], sizeof(Note));
    enqueueSingle(n.freq, n.duration);
  }
}

void soundInit() {
  pinMode(BUZZER_PIN, OUTPUT);
  queueClear();
  toneActive = false;
}

void soundResetForTest() {
  queueClear();
  toneActive = false;
  noTone(BUZZER_PIN);
}

void soundEnqueueForTest(int freq, int duration) {
  enqueueSingle((int16_t)freq, (int16_t)duration);
}

uint8_t soundGetQueueLength() { return queueCount; }
bool soundIsToneActive() { return toneActive; }
bool isSoundPlaying() { return toneActive || queueCount > 0; }

void playSound(SoundEffect sfx) {
  // um novo som sempre interrompe o que estiver tocando
  queueClear();
  noTone(BUZZER_PIN);
  toneActive = false;

  switch (sfx) {
    case SFX_MENU_MOVE:   enqueueFromProgmem(NOTES_MENU_MOVE, 1);   break;
    case SFX_MENU_SELECT: enqueueFromProgmem(NOTES_MENU_SELECT, 3); break;
    case SFX_SNAKE_EAT:   enqueueFromProgmem(NOTES_SNAKE_EAT, 1);   break;
    case SFX_SNAKE_DEAD:  enqueueFromProgmem(NOTES_SNAKE_DEAD, 3);  break;
    case SFX_PONG_HIT:    enqueueFromProgmem(NOTES_PONG_HIT, 1);    break;
    case SFX_PONG_WALL:   enqueueFromProgmem(NOTES_PONG_WALL, 1);   break;
    case SFX_PONG_SCORE:  enqueueFromProgmem(NOTES_PONG_SCORE, 2);  break;
    case SFX_INV_SHOOT:   enqueueFromProgmem(NOTES_INV_SHOOT, 1);   break;
    case SFX_INV_HIT:     enqueueFromProgmem(NOTES_INV_HIT, 1);     break;
    case SFX_INV_DEAD:    enqueueFromProgmem(NOTES_INV_DEAD, 2);    break;
    case SFX_GAMEOVER:    enqueueFromProgmem(NOTES_GAMEOVER, 4);    break;
    case SFX_LEVELUP:     enqueueFromProgmem(NOTES_LEVELUP, 3);     break;
  }
}

void updateSound(unsigned long now) {
  if (toneActive) {
    if (now - toneStart >= (unsigned long)toneDuration) {
      noTone(BUZZER_PIN);
      toneActive = false;
    } else {
      return; // nota atual ainda tocando
    }
  }

  if (queueCount > 0) {
    Note n = queue[queueHead];
    queueHead = (queueHead + 1) % QUEUE_CAPACITY;
    queueCount--;

    tone(BUZZER_PIN, n.freq, n.duration);
    toneActive = true;
    toneStart = now;
    toneDuration = n.duration;
  }
}
