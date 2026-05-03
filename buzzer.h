#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include <stdbool.h>
#include "ti_msp_dl_config.h"
#include "game.h"

// ── Note frequencies (Hz) ────────────────────────────────────────────────────
#define NOTE_REST     0
#define NOTE_B4     494
#define NOTE_C5     523
#define NOTE_D5     587
#define NOTE_E5     659
#define NOTE_G5     784
#define NOTE_A5     880
#define NOTE_B5     988
#define NOTE_D6    1175
#define NOTE_E6    1319

// ── Public API ───────────────────────────────────────────────────────────────
void buzzer_init(void);
void buzzer_set_music_enabled(bool enabled);
void sfx_play(uint16_t frequency_hz, uint16_t duration_ms);
void jingle_start(void);
void jingle_tick(void);
void dispatch_game_sfx(GameSFX sfx);

#endif