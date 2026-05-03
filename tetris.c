#include "tetris.h"
#include <string.h>

typedef struct { int8_t r, c; } Cell;

static const Cell pieces[TET_PIECE_COUNT][4][4] = {
    // I
    {{{0,0},{0,1},{0,2},{0,3}},
     {{0,2},{1,2},{2,2},{3,2}},
     {{1,0},{1,1},{1,2},{1,3}},
     {{0,1},{1,1},{2,1},{3,1}}},
    // O
    {{{0,0},{0,1},{1,0},{1,1}},
     {{0,0},{0,1},{1,0},{1,1}},
     {{0,0},{0,1},{1,0},{1,1}},
     {{0,0},{0,1},{1,0},{1,1}}},
    // T
    {{{0,1},{1,0},{1,1},{1,2}},
     {{0,1},{1,1},{2,1},{1,2}},
     {{1,0},{1,1},{1,2},{2,1}},
     {{0,1},{1,0},{1,1},{2,1}}},
    // S
    {{{0,1},{0,2},{1,0},{1,1}},
     {{0,1},{1,1},{1,2},{2,2}},
     {{0,1},{0,2},{1,0},{1,1}},
     {{0,1},{1,1},{1,2},{2,2}}},
    // Z
    {{{0,0},{0,1},{1,1},{1,2}},
     {{0,2},{1,1},{1,2},{2,1}},
     {{0,0},{0,1},{1,1},{1,2}},
     {{0,2},{1,1},{1,2},{2,1}}},
    // J
    {{{0,0},{1,0},{1,1},{1,2}},
     {{0,1},{0,2},{1,1},{2,1}},
     {{1,0},{1,1},{1,2},{2,2}},
     {{0,1},{1,1},{2,0},{2,1}}},
    // L
    {{{0,2},{1,0},{1,1},{1,2}},
     {{0,1},{1,1},{2,1},{2,2}},
     {{1,0},{1,1},{1,2},{2,0}},
     {{0,0},{0,1},{1,1},{2,1}}},
};

static uint16_t rng_state = 12345;

static uint8_t rng_next(void)
{
    rng_state = rng_state * 25173u + 13849u;
    return (uint8_t)(rng_state >> 8);
}

static uint8_t random_piece(void)
{
    return rng_next() % TET_PIECE_COUNT;
}

static bool collides(const TetrisData *t, uint8_t piece, uint8_t rot,
                     int8_t px, int8_t py)
{
    for (uint8_t i = 0; i < 4; i++) {
        int8_t r = py + pieces[piece][rot][i].r;
        int8_t c = px + pieces[piece][rot][i].c;
        if (c < 0 || c >= TET_COLS) return true;
        if (r >= TET_ROWS)          return true;
        if (r < 0)                  continue;
        if (t->board[r] & (1u << c)) return true;
    }
    return false;
}

static void lock_piece(TetrisData *t)
{
    for (uint8_t i = 0; i < 4; i++) {
        int8_t r = t->piece_y + pieces[t->piece][t->rotation][i].r;
        int8_t c = t->piece_x + pieces[t->piece][t->rotation][i].c;
        if (r >= 0 && r < TET_ROWS && c >= 0 && c < TET_COLS)
            t->board[r] |= (1u << c);
    }
}

static uint8_t clear_lines(TetrisData *t)
{
    uint8_t cleared = 0;
    uint16_t full = (1u << TET_COLS) - 1u;

    for (int8_t r = TET_ROWS - 1; r >= 0; r--) {
        if (t->board[r] == full) {
            for (int8_t rr = r; rr > 0; rr--)
                t->board[rr] = t->board[rr - 1];
            t->board[0] = 0;
            cleared++;
            r++;
        }
    }
    return cleared;
}

static void spawn_piece(TetrisData *t)
{
    t->piece      = t->next_piece;
    t->rotation   = 0;
    t->piece_x    = TET_COLS / 2 - 2;
    t->piece_y    = 0;
    t->next_piece = random_piece();

    if (collides(t, t->piece, t->rotation, t->piece_x, t->piece_y)) {
        t->state       = TET_STATE_GAMEOVER;
        t->blink_timer = 0;
        t->blink_on    = true;
    }
}

static void do_lock(TetrisData *t)
{
    lock_piece(t);

    uint16_t full = (1u << TET_COLS) - 1u;
    t->flash_rows = 0;
    for (uint8_t r = 0; r < TET_ROWS; r++)
        if (t->board[r] == full)
            t->flash_rows |= (1u << r);

    if (t->flash_rows) {
        t->flashing    = true;
        t->flash_timer = 8;
    } else {
        spawn_piece(t);
    }
}

// Classic Nintendo scoring
static const uint16_t line_scores[5] = {0, 100, 300, 500, 800};

void tetris_init(TetrisData *t)
{
    memset(t, 0, sizeof(TetrisData));
    t->state      = TET_STATE_TITLE;
    t->gravity    = TET_GRAVITY_START;
    t->blink_on   = true;
    t->next_piece = random_piece();
    spawn_piece(t);
}

void tetris_tick(TetrisData *t, bool left, bool right,
                 bool soft_drop, bool rotate, bool hard_drop)
{
    bool do_left     = left      && !t->prev_left;
    bool do_right    = right     && !t->prev_right;
    bool do_rotate   = rotate    && !t->prev_rotate;
    bool do_harddrop = hard_drop && !t->prev_drop;

    t->prev_left   = left;
    t->prev_right  = right;
    t->prev_rotate = rotate;
    t->prev_drop   = hard_drop;

    switch (t->state) {

    case TET_STATE_TITLE:
        t->blink_timer++;
        if (t->blink_timer >= 30) {
            t->blink_timer = 0;
            t->blink_on = !t->blink_on;
        }
        // Only A starts the game from title
        if (rotate) {
            memset(t->board, 0, sizeof(t->board));
            t->score   = 0;
            t->lines   = 0;
            t->level   = 0;
            t->gravity = TET_GRAVITY_START;
            t->flashing    = false;
            t->flash_rows  = 0;
            t->flash_timer = 0;
            t->next_piece  = random_piece();
            spawn_piece(t);
            t->state = TET_STATE_PLAYING;
        }
        break;

    case TET_STATE_PLAYING:
        // Handle active flash — freeze everything
        if (t->flashing) {
            t->flash_timer--;
            if (t->flash_timer == 0) {
                t->flashing       = false;
                uint8_t cleared   = clear_lines(t);
                t->score         += line_scores[cleared] * (t->level + 1u);
                t->lines         += cleared;
                t->level          = t->lines / TET_LINES_PER_LEVEL;
                int16_t new_grav  = TET_GRAVITY_START - (int16_t)(t->level * 3);
                t->gravity        = (new_grav < TET_GRAVITY_MIN) ?
                                     TET_GRAVITY_MIN : (uint8_t)new_grav;
                spawn_piece(t);
            }
            return;
        }

        // Move left
        if (do_left) {
            if (!collides(t, t->piece, t->rotation,
                          t->piece_x - 1, t->piece_y))
                t->piece_x--;
        }

        // Move right
        if (do_right) {
            if (!collides(t, t->piece, t->rotation,
                          t->piece_x + 1, t->piece_y))
                t->piece_x++;
        }

        // Rotate with wall kick
        if (do_rotate) {
            uint8_t new_rot = (t->rotation + 1u) % 4u;
            if (!collides(t, t->piece, new_rot, t->piece_x, t->piece_y)) {
                t->rotation = new_rot;
            } else if (!collides(t, t->piece, new_rot,
                                 t->piece_x - 1, t->piece_y)) {
                t->piece_x--;
                t->rotation = new_rot;
            } else if (!collides(t, t->piece, new_rot,
                                 t->piece_x + 1, t->piece_y)) {
                t->piece_x++;
                t->rotation = new_rot;
            }
        }

        // Hard drop
        if (do_harddrop) {
            while (!collides(t, t->piece, t->rotation,
                             t->piece_x, t->piece_y + 1))
                t->piece_y++;
            do_lock(t);
            break;
        }

        // Gravity / soft drop
        {
            t->gravity_timer++;
            uint8_t drop_rate = soft_drop ? 2u : t->gravity;
            if (t->gravity_timer >= drop_rate) {
                t->gravity_timer = 0;
                if (!collides(t, t->piece, t->rotation,
                              t->piece_x, t->piece_y + 1)) {
                    t->piece_y++;
                } else {
                    do_lock(t);
                }
            }
        }
        break;

    case TET_STATE_GAMEOVER:
        t->blink_timer++;
        if (t->blink_timer >= 30) {
            t->blink_timer = 0;
            t->blink_on = !t->blink_on;
        }
        // No action here — main.c handles A -> menu transition
        break;
    }
}

static void draw_cell(int8_t row, int8_t col, uint8_t color)
{
    int16_t x = TET_BOARD_X + col * TET_CELL;
    int16_t y = TET_BOARD_Y + row * TET_CELL;
    OLED_FillRect(x, y, TET_CELL - 1, TET_CELL - 1, color);
}

static void draw_board(const TetrisData *t)
{
    for (uint8_t r = 0; r < TET_ROWS; r++) {
        bool flash_this = t->flashing &&
                          (t->flash_rows & (1u << r)) &&
                          (t->flash_timer & 1u);
        for (uint8_t c = 0; c < TET_COLS; c++) {
            if (t->board[r] & (1u << c))
                draw_cell(r, c, flash_this ? 0 : 1);
            else if (flash_this)
                draw_cell(r, c, 1);
        }
    }
}

static void draw_current_piece(const TetrisData *t)
{
    for (uint8_t i = 0; i < 4; i++) {
        int8_t r = t->piece_y + pieces[t->piece][t->rotation][i].r;
        int8_t c = t->piece_x + pieces[t->piece][t->rotation][i].c;
        if (r >= 0) draw_cell(r, c, 1);
    }
}

static void draw_ghost(const TetrisData *t)
{
    int8_t ghost_y = t->piece_y;
    while (!collides(t, t->piece, t->rotation, t->piece_x, ghost_y + 1))
        ghost_y++;

    if (ghost_y == t->piece_y) return;

    for (uint8_t i = 0; i < 4; i++) {
        int8_t r = ghost_y + pieces[t->piece][t->rotation][i].r;
        int8_t c = t->piece_x + pieces[t->piece][t->rotation][i].c;
        if (r >= 0) {
            int16_t x = TET_BOARD_X + c * TET_CELL;
            int16_t y = TET_BOARD_Y + r * TET_CELL;
            OLED_DrawRect(x, y, TET_CELL - 1, TET_CELL - 1, 1);
        }
    }
}

static void draw_next_piece(const TetrisData *t)
{
    OLED_DrawString(TET_PANEL_X, 2, "NXT", 1, 1);
    for (uint8_t i = 0; i < 4; i++) {
        int8_t r = pieces[t->next_piece][0][i].r;
        int8_t c = pieces[t->next_piece][0][i].c;
        int16_t x = TET_PANEL_X + c * (TET_CELL - 1);
        int16_t y = 12 + r * (TET_CELL - 1);
        OLED_FillRect(x, y, TET_CELL - 1, TET_CELL - 1, 1);
    }
}

static void draw_score(const TetrisData *t)
{
    OLED_DrawString(TET_PANEL_X, 30, "SCR", 1, 1);
    char buf[6];
    uint16_t s = t->score;
    buf[0] = '0' + (s / 10000u) % 10u;
    buf[1] = '0' + (s / 1000u)  % 10u;
    buf[2] = '0' + (s / 100u)   % 10u;
    buf[3] = '0' + (s / 10u)    % 10u;
    buf[4] = '0' + (s % 10u);
    buf[5] = '\0';
    OLED_DrawString(TET_PANEL_X, 38, buf, 1, 1);

    OLED_DrawString(TET_PANEL_X, 48, "LNS", 1, 1);
    char lbuf[4];
    lbuf[0] = '0' + (t->lines / 100u) % 10u;
    lbuf[1] = '0' + (t->lines / 10u)  % 10u;
    lbuf[2] = '0' + (t->lines % 10u);
    lbuf[3] = '\0';
    OLED_DrawString(TET_PANEL_X, 56, lbuf, 1, 1);
}

void tetris_render(TetrisData *t)
{
    OLED_Clear();

    switch (t->state) {

    case TET_STATE_TITLE:
        OLED_DrawString(28, 4, "TETRIS", 1, 2);
        OLED_DrawHLine(0, 20, OLED_W, 1);
        OLED_DrawString(4, 26, "L/R:MOVE  A:ROT", 1, 1);
        OLED_DrawString(4, 38, "DOWN:SOFT  B:HARD", 1, 1);
        OLED_DrawHLine(0, 50, OLED_W, 1);
        if (t->blink_on)
            OLED_DrawString(37, 54, "PRESS A", 1, 1);
        break;

    case TET_STATE_PLAYING:
        OLED_DrawRect(TET_BOARD_X - 1,
                      TET_BOARD_Y - 1,
                      TET_COLS * TET_CELL + 2,
                      TET_ROWS * TET_CELL + 2, 1);
        draw_ghost(t);
        draw_board(t);
        draw_current_piece(t);
        draw_next_piece(t);
        draw_score(t);
        break;

    case TET_STATE_GAMEOVER:
        OLED_DrawString(10, 8, "GAME OVER", 1, 2);
        OLED_DrawHLine(0, 28, OLED_W, 1);
        {
            char buf[6];
            uint16_t s = t->score;
            buf[0] = '0' + (s / 10000u) % 10u;
            buf[1] = '0' + (s / 1000u)  % 10u;
            buf[2] = '0' + (s / 100u)   % 10u;
            buf[3] = '0' + (s / 10u)    % 10u;
            buf[4] = '0' + (s % 10u);
            buf[5] = '\0';
            OLED_DrawString(22, 32, "SCORE:", 1, 1);
            OLED_DrawString(40, 42, buf, 1, 1);
        }
        if (t->blink_on)
            OLED_DrawString(28, 54, "A: MAIN MENU", 1, 1);
        break;
    }

    OLED_Flush();
}