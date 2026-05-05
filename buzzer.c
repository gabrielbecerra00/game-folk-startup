#include "buzzer.h"

#define BUZZER_TICK_HZ       8000
#define BUZZER_TICKS_PER_MS  (BUZZER_TICK_HZ / 1000)

// Menu music 
typedef struct {
    uint16_t frequency_hz;
    uint16_t duration_ms;
} BuzzerNote;

static const BuzzerNote menu_song[] = {
    {NOTE_E5,   120}, {NOTE_REST, 40},
    {NOTE_G5,   120}, {NOTE_REST, 40},
    {NOTE_B5,   120}, {NOTE_REST, 40},
    {NOTE_G5,   120}, {NOTE_REST, 80},
    {NOTE_E6,   160},
    {NOTE_D6,   160},
    {NOTE_B5,   160},
    {NOTE_G5,   160},
    {NOTE_A5,   120}, {NOTE_REST, 40},
    {NOTE_B5,   240}, {NOTE_REST, 160},
};
#define MENU_SONG_LENGTH  (sizeof(menu_song) / sizeof(menu_song[0]))

// Game over jingle 
static const BuzzerNote gameover_jingle[] = {
    {NOTE_B5,   100},
    {NOTE_REST,  40},
    {NOTE_B5,   100},
    {NOTE_REST,  40},
    {NOTE_B4,   400},
};
#define GAMEOVER_JINGLE_LENGTH  (sizeof(gameover_jingle) / sizeof(gameover_jingle[0]))

static const BuzzerNote tetris_clear_jingle[] = {
    {NOTE_GS5, 80},
    {NOTE_REST, 20},
    {NOTE_AS5, 120},
};
#define TETRIS_CLEAR_LENGTH (sizeof(tetris_clear_jingle) / sizeof(tetris_clear_jingle[0]))

static const BuzzerNote tetris_fail_jingle[] = {
    {NOTE_F5,   80},
    {NOTE_REST, 30},
    {NOTE_F5,   80},
    {NOTE_REST, 30},
    {NOTE_F5,   80},
    {NOTE_REST, 30},
};
#define TETRIS_FAIL_LENGTH (sizeof(tetris_fail_jingle) / sizeof(tetris_fail_jingle[0]))

static JingleType  jingle_type    = JINGLE_GAMEOVER;
static bool        jingle_done    = false;



// Music state
static volatile bool     buzzer_music_enabled       = false;
static volatile uint8_t  buzzer_note_index          = 0;
static volatile uint16_t buzzer_note_ticks_remaining = 0;
static volatile uint16_t buzzer_half_period_ticks   = 0;
static volatile uint16_t buzzer_half_period_counter = 0;
static volatile bool     buzzer_pin_on              = false;

// SFX state
static volatile uint16_t sfx_half_period_ticks   = 0;
static volatile uint16_t sfx_half_period_counter = 0;
static volatile uint16_t sfx_ticks_remaining     = 0;
static volatile bool     sfx_pin_on              = false;

// Jingle state
static uint8_t jingle_index   = 0;
static bool    jingle_playing = false;

// Internal helpers
static void buzzer_pin_clear(void)
{
    DL_GPIO_clearPins(BUZZER_PORT, BUZZER_PIN_PIN);
}

static void buzzer_pin_set(void)
{
    DL_GPIO_setPins(BUZZER_PORT, BUZZER_PIN_PIN);
}

static uint16_t half_period_from_freq(uint16_t freq)
{
    if (freq == NOTE_REST) return 0;
    uint32_t hp = BUZZER_TICK_HZ / ((uint32_t)freq * 2u);
    return (uint16_t)(hp < 1 ? 1 : hp);
}

static void buzzer_load_note(uint8_t index)
{
    uint32_t dt = (uint32_t)menu_song[index].duration_ms * BUZZER_TICKS_PER_MS;
    buzzer_note_ticks_remaining = (dt > 0) ? (uint16_t)dt : 1;
    buzzer_half_period_ticks    = half_period_from_freq(
                                      menu_song[index].frequency_hz);
    buzzer_half_period_counter  = 0;
    buzzer_pin_clear();
    buzzer_pin_on = false;
}

// SysTick ISR
void SysTick_Handler(void)
{
    if (buzzer_music_enabled) {
        // Menu music
        if (buzzer_note_ticks_remaining == 0)
            buzzer_load_note(buzzer_note_index);

        if (buzzer_half_period_ticks == 0) {
            buzzer_pin_clear();
            buzzer_pin_on = false;
        } else {
            buzzer_half_period_counter++;
            if (buzzer_half_period_counter >= buzzer_half_period_ticks) {
                buzzer_half_period_counter = 0;
                if (buzzer_pin_on) {
                    buzzer_pin_clear();
                    buzzer_pin_on = false;
                } else {
                    buzzer_pin_set();
                    buzzer_pin_on = true;
                }
            }
        }
        buzzer_note_ticks_remaining--;
        if (buzzer_note_ticks_remaining == 0) {
            buzzer_note_index++;
            if (buzzer_note_index >= MENU_SONG_LENGTH)
                buzzer_note_index = 0;
        }

    } else {
        // SFX one-shot
        if (sfx_ticks_remaining == 0) {
            buzzer_pin_clear();
            sfx_pin_on = false;
            return;
        }
        if (sfx_half_period_ticks == 0) {
            buzzer_pin_clear();
            sfx_pin_on = false;
        } else {
            sfx_half_period_counter++;
            if (sfx_half_period_counter >= sfx_half_period_ticks) {
                sfx_half_period_counter = 0;
                if (sfx_pin_on) {
                    buzzer_pin_clear();
                    sfx_pin_on = false;
                } else {
                    buzzer_pin_set();
                    sfx_pin_on = true;
                }
            }
        }
        sfx_ticks_remaining--;
    }
}

// Public API
void buzzer_init(void)
{
    buzzer_pin_clear();
    SysTick_Config(CPUCLK_FREQ / BUZZER_TICK_HZ);
}

void buzzer_set_music_enabled(bool enabled)
{
    if (enabled && !buzzer_music_enabled) {
        buzzer_note_index           = 0;
        buzzer_note_ticks_remaining = 0;
        buzzer_half_period_ticks    = 0;
        buzzer_half_period_counter  = 0;
    } else if (!enabled && buzzer_music_enabled) {
        buzzer_pin_clear();
        buzzer_pin_on = false;
    }
    buzzer_music_enabled = enabled;
}

void sfx_play(uint16_t frequency_hz, uint16_t duration_ms)
{
    sfx_half_period_ticks   = half_period_from_freq(frequency_hz);
    sfx_half_period_counter = 0;
    sfx_ticks_remaining     = (uint16_t)((uint32_t)duration_ms *
                               BUZZER_TICKS_PER_MS);
    sfx_pin_on              = false;
}

void dispatch_game_sfx(GameSFX sfx)
{
    switch (sfx) {
    case SFX_PADDLE_HIT:  sfx_play(NOTE_B5, 40);              break;
    case SFX_WALL_HIT:    sfx_play(NOTE_B5, 40);              break;
    case SFX_SCORE:       sfx_play(NOTE_B4, 120);             break;
    case SFX_GAMEOVER:    jingle_start(JINGLE_GAMEOVER);      break;
    default:              break;
    }
}

void dispatch_tetris_sfx(TetrisSFX sfx)
{
    switch (sfx) {
    case TET_SFX_MOVE:
        sfx_play(NOTE_A4, 30);
        break;
    case TET_SFX_ROTATE:
        sfx_play(NOTE_D5, 30);
        break;
    case TET_SFX_LOCK:
        sfx_play(NOTE_CS5, 60);
        break;
    case TET_SFX_CLEAR:
        jingle_start(JINGLE_TETRIS_CLEAR);
        break;
    case TET_SFX_GAMEOVER:
        jingle_start(JINGLE_TETRIS_FAIL);
        break;
    default:
        break;
    }
}

void jingle_start(JingleType type)
{
    jingle_type    = type;
    jingle_index   = 0;
    jingle_playing = true;
    jingle_done    = false;
    const BuzzerNote *song = (type == JINGLE_GAMEOVER)    ? gameover_jingle :
                             (type == JINGLE_TETRIS_CLEAR) ? tetris_clear_jingle :
                                                             tetris_fail_jingle;
    sfx_play(song[0].frequency_hz, song[0].duration_ms);
}

bool jingle_finished(void)
{
    return jingle_done;
}

void jingle_tick(void)
{
    if (!jingle_playing) return;
    if (sfx_ticks_remaining > 0) return;

    const BuzzerNote *song;
    uint8_t length;

    if (jingle_type == JINGLE_GAMEOVER) {
        song   = gameover_jingle;
        length = GAMEOVER_JINGLE_LENGTH;
    } else if (jingle_type == JINGLE_TETRIS_CLEAR) {
        song   = tetris_clear_jingle;
        length = TETRIS_CLEAR_LENGTH;
    } else {
        song   = tetris_fail_jingle;
        length = TETRIS_FAIL_LENGTH;
    }

    jingle_index++;
    if (jingle_index >= length) {
        jingle_playing = false;
        jingle_done    = true;
        return;
    }
    sfx_play(song[jingle_index].frequency_hz,
              song[jingle_index].duration_ms);
}