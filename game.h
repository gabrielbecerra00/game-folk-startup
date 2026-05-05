#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>
#include "oled.h"

// Display dimensions
#define PADDLE_W        20
#define PADDLE_H         2
#define PADDLE_Y        60
#define PADDLE_SPEED     3

#define BALL_SIZE        3
#define BALL_START_X    62
#define BALL_START_Y    32
#define BALL_START_DX    1
#define BALL_START_DY   -1

//#define WINNING_SCORE 7
#define LIVES_START      3

#define P2_PADDLE_W      2
#define P2_PADDLE_H     16
#define P2_LEFT_X        2
#define P2_RIGHT_X     124
#define P2_PADDLE_SPEED  3
#define P2_WIN_SCORE     5

#define PLAT_PLAYER_W    6
#define PLAT_PLAYER_H    8
#define PLAT_GROUND_Y   56
#define PLAT_MOVE_SPEED  2
#define PLAT_JUMP_VY    -8
#define PLAT_MAX_FALL    6
#define PLAT_WORLD_W   360
#define PLAT_GOAL_X    334

// Speedrun timer
// Set this to match the tick rate of TIMA0 (ticks per second).
// e.g. if your timer period = 1/60 s  →  PLAT_TICKS_PER_SEC 60
// If you use the RTC peripheral instead, see platformer_tick() for the
// optional DL_RTC swap-in comment.
#define PLAT_TICKS_PER_SEC  60u

typedef enum {
    APP_STATE_SPLASH,
    APP_STATE_MENU,
    APP_STATE_PONG,
    APP_STATE_2P_PONG,
    APP_STATE_PLATFORMER,
    APP_STATE_TETRIS,
    APP_STATE_BUTTON_TEST
} AppState;

typedef enum {
    GAME_STATE_TITLE,
    GAME_STATE_PLAYING,
    GAME_STATE_DEAD,     // brief pause after losing a life, was previously GAME_STATE_DEAD
    GAME_STATE_GAMEOVER
} GameState;

typedef struct {
    uint8_t selected;      // 0 = Pong, 1 = 2P Pong, 2 = Platformer, 3 = Button Test
    uint8_t blink_timer;
    bool blink_on;
} MenuState;

typedef enum {
    SFX_NONE,
    SFX_PADDLE_HIT,
    SFX_WALL_HIT,
    SFX_SCORE,
    SFX_GAMEOVER
} GameSFX;

typedef struct {
    AppState app_state;
    MenuState menu;
    GameSFX sfx;
    // Ball
    int16_t ball_x, ball_y;
    int16_t ball_dx, ball_dy;

    // Paddle
    int16_t paddle_x;
    int16_t left_paddle_y;
    int16_t right_paddle_y;
    int16_t player_x;
    int16_t player_y;
    int16_t player_vy;
    int16_t camera_x;
    bool player_grounded;
    bool platformer_won;

    // Score / lives
    uint8_t score;
    uint8_t lives;
    uint8_t p1_score;
    uint8_t p2_score;

    // State
    GameState state;

    // Title/gameover blink counter
    uint8_t blink_timer;
    bool    blink_on;

    // Dead state timer (pause before respawn)
    uint8_t dead_timer;

    // Speedrun timer (platformer)
    uint32_t run_ticks;        // counts up every game tick while playing
    uint32_t run_ticks_final;  // frozen at the moment the level is finished
    bool     timer_active;     // true only while the run clock is ticking
} GameData;

void game_init(GameData *g);
void game_tick(GameData *g, bool left, bool right, bool action);
void game_render(GameData *g);

void game2p_init(GameData *g);
void game2p_tick(GameData *g, bool left_up, bool left_down,
                 bool right_up, bool right_down, bool action);
void game2p_render(GameData *g);

void platformer_init(GameData *g);
void platformer_tick(GameData *g, bool left, bool right, bool jump);
void platformer_render(GameData *g);

#endif
