#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include <stdbool.h>
#include "ti_msp_dl_config.h"
#include "game.h"
#include "tetris.h"

// Note frequencies (Hz)
#define NOTE_REST     0
#define NOTE_A4     440
#define NOTE_B4     494
#define NOTE_C5     523
#define NOTE_CS5    554
#define NOTE_D5     587
#define NOTE_E5     659
#define NOTE_F5     698
#define NOTE_FS5    740
#define NOTE_G5     784
#define NOTE_GS5    831
#define NOTE_A5     880
#define NOTE_AS5    932
#define NOTE_B5     988
#define NOTE_D6    1175
#define NOTE_E6    1319

// Types
typedef enum {
    JINGLE_GAMEOVER,
    JINGLE_TETRIS_CLEAR,
    JINGLE_TETRIS_FAIL
} JingleType;

// Public API
void buzzer_init(void);
void buzzer_set_music_enabled(bool enabled);
void sfx_play(uint16_t frequency_hz, uint16_t duration_ms);
void dispatch_game_sfx(GameSFX sfx);
void dispatch_tetris_sfx(TetrisSFX sfx);
void jingle_start(JingleType type);
void jingle_tick(void);
bool jingle_finished(void);

#endif