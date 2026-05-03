#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>
#include <stdbool.h>
#include "oled.h"

// Board dimensions
#define TET_COLS        10
#define TET_ROWS        20
#define TET_CELL        3       // pixels per cell

// Board position on screen
#define TET_BOARD_X     30
#define TET_BOARD_Y     2

// Panel (score + next piece) position
#define TET_PANEL_X     (TET_BOARD_X + TET_COLS * TET_CELL + 4)

// Gravity: ticks per automatic drop
#define TET_GRAVITY_START   30
#define TET_GRAVITY_MIN      5
#define TET_LINES_PER_LEVEL  5

// 7 tetrominoes
#define TET_PIECE_COUNT  7

typedef enum {
    TET_STATE_TITLE,
    TET_STATE_PLAYING,
    TET_STATE_GAMEOVER
} TetrisState;

typedef struct {
    TetrisState state;

    // Board: each row is a bitmask of TET_COLS bits
    uint16_t board[TET_ROWS];

    // Current piece
    uint8_t  piece;        // 0–6
    uint8_t  rotation;     // 0–3
    int8_t   piece_x;      // board col of piece origin
    int8_t   piece_y;      // board row of piece origin

    // Next piece
    uint8_t  next_piece;

    // Timing
    uint8_t  gravity;      // ticks per drop (decreases with level)
    uint8_t  gravity_timer;

    // Score
    uint16_t score;
    uint8_t  lines;
    uint8_t  level;

    // Blink for title/gameover
    uint8_t  blink_timer;
    bool     blink_on;

    // Input edge detection
    bool     prev_rotate;
    bool     prev_left;
    bool     prev_right;
    bool     prev_drop;

    uint8_t  flash_timer;
    uint32_t flash_rows;   // needs 20 bits (TET_ROWS=20), uint16_t was truncating rows 16-19
    bool     flashing;
} TetrisData;

void tetris_init(TetrisData *t);
void tetris_tick(TetrisData *t, bool left, bool right,
                 bool soft_drop, bool rotate, bool hard_drop);
void tetris_render(TetrisData *t);

#endif