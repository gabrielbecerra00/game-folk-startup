#include "game.h"
#include <string.h>

// ── Helpers ──────────────────────────────────────────────────────────────────
static void reset_ball(GameData *g)
{
    g->ball_x  = BALL_START_X;
    g->ball_y  = BALL_START_Y;
    g->ball_dx = BALL_START_DX;
    g->ball_dy = BALL_START_DY;
}

static void reset_2p_ball(GameData *g, int16_t dx)
{
    g->ball_x = (OLED_W / 2) - (BALL_SIZE / 2);
    g->ball_y = (OLED_H / 2) - (BALL_SIZE / 2);
    g->ball_dx = dx;
    g->ball_dy = ((g->p1_score + g->p2_score) & 1) ? -1 : 1;
}

static void clamp_2p_paddle(int16_t *paddle_y)
{
    if (*paddle_y < 0) {
        *paddle_y = 0;
    }
    if (*paddle_y + P2_PADDLE_H > OLED_H) {
        *paddle_y = OLED_H - P2_PADDLE_H;
    }
}

static void adjust_2p_ball_y(GameData *g, int16_t paddle_y)
{
    int16_t hit = (g->ball_y + BALL_SIZE / 2) - (paddle_y + P2_PADDLE_H / 2);

    if (hit < -4) {
        g->ball_dy = -2;
    } else if (hit > 4) {
        g->ball_dy = 2;
    } else {
        g->ball_dy = (g->ball_dy >= 0) ? 1 : -1;
    }
}

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} PlatformerRect;

typedef struct {
    int16_t start_x;
    int16_t end_x;
} PlatformerPit;

static const PlatformerPit platformer_pits[] = {
    {74, 100},
    {184, 206},
    {252, 274},
};

static const PlatformerRect platformer_platforms[] = {
    {35, 43, 28, 3},
    {110, 38, 25, 3},
    {150, 47, 26, 3},
    {216, 39, 28, 3},
    {286, 45, 24, 3},
};

static const PlatformerRect platformer_spikes[] = {
    {64, 52, 8, 4},
    {170, 52, 8, 4},
    {314, 52, 8, 4},
};

static const PlatformerRect platformer_enemies[] = {
    {118, 50, 7, 6},
    {226, 33, 7, 6},
    {300, 50, 7, 6},
};

static bool rects_overlap(int16_t ax, int16_t ay, int16_t aw, int16_t ah,
                          int16_t bx, int16_t by, int16_t bw, int16_t bh)
{
    return (ax < bx + bw) && (ax + aw > bx) &&
           (ay < by + bh) && (ay + ah > by);
}

static bool platformer_over_pit(int16_t x, int16_t w)
{
    for (uint8_t i = 0;
         i < (uint8_t)(sizeof(platformer_pits) / sizeof(platformer_pits[0]));
         i++)
    {
        if (x + w > platformer_pits[i].start_x &&
            x < platformer_pits[i].end_x)
        {
            return true;
        }
    }

    return false;
}

static bool platformer_hits_rect(GameData *g, const PlatformerRect *rect)
{
    return rects_overlap(g->player_x, g->player_y,
                         PLAT_PLAYER_W, PLAT_PLAYER_H,
                         rect->x, rect->y, rect->w, rect->h);
}

static void platformer_die(GameData *g)
{
    g->timer_active  = false;   // stop the clock on death
    g->state = GAME_STATE_GAMEOVER;
    g->platformer_won = false;
    g->blink_timer = 0;
    g->blink_on = true;
}

static void platformer_win(GameData *g)
{
    g->timer_active    = false;          // stop the clock
    g->run_ticks_final = g->run_ticks;   // freeze the final time
    g->state = GAME_STATE_GAMEOVER;
    g->platformer_won = true;
    g->blink_timer = 0;
    g->blink_on = true;
}

static void draw_world_rect(GameData *g, int16_t x, int16_t y,
                            int16_t w, int16_t h, uint8_t color)
{
    int16_t screen_x = x - g->camera_x;

    if (screen_x >= OLED_W || screen_x + w <= 0) {
        return;
    }
    if (screen_x < 0) {
        w += screen_x;
        screen_x = 0;
    }
    if (screen_x + w > OLED_W) {
        w = OLED_W - screen_x;
    }
    if (w > 0) {
        OLED_FillRect(screen_x, y, w, h, color);
    }
}

// ── Init ─────────────────────────────────────────────────────────────────────
void game_init(GameData *g)
{
    memset(g, 0, sizeof(GameData));
    g->paddle_x   = (OLED_W / 2) - (PADDLE_W / 2);
    g->lives      = LIVES_START;
    g->state      = GAME_STATE_TITLE;
    g->blink_timer = 0;
    g->blink_on    = true;
    reset_ball(g);
}

void game2p_init(GameData *g)
{
    memset(g, 0, sizeof(GameData));
    g->left_paddle_y = (OLED_H / 2) - (P2_PADDLE_H / 2);
    g->right_paddle_y = (OLED_H / 2) - (P2_PADDLE_H / 2);
    g->state = GAME_STATE_TITLE;
    g->blink_timer = 0;
    g->blink_on = true;
    reset_2p_ball(g, 1);
}

// ── Tick (called every 16ms from timer ISR flag) ──────────────────────────────
void game_tick(GameData *g, bool left, bool right, bool action)
{
    g->sfx = SFX_NONE;
    switch (g->state) {

    // ── Title screen ─────────────────────────────────────────────────────────
    case GAME_STATE_TITLE:
        g->blink_timer++;
        if (g->blink_timer >= 30) {   // toggle every ~0.5s
            g->blink_timer = 0;
            g->blink_on = !g->blink_on;
        }
        if (action) {
            g->state = GAME_STATE_PLAYING;
        }
        break;

    // ── Playing ──────────────────────────────────────────────────────────────
    case GAME_STATE_PLAYING:
        // Move paddle
        if (left) {
            g->paddle_x -= PADDLE_SPEED;
            if (g->paddle_x < 0) g->paddle_x = 0;
        }
        if (right) {
            g->paddle_x += PADDLE_SPEED;
            if (g->paddle_x + PADDLE_W > OLED_W)
                g->paddle_x = OLED_W - PADDLE_W;
        }

        // Move ball
        g->ball_x += g->ball_dx;
        g->ball_y += g->ball_dy;

        // Wall collisions (left/right)
        if (g->ball_x <= 0) {
            g->ball_x  = 0;
            g->ball_dx = -g->ball_dx;
            g->sfx = SFX_WALL_HIT; 
        }
        if (g->ball_x + BALL_SIZE >= OLED_W) {
            g->ball_x  = OLED_W - BALL_SIZE;
            g->ball_dx = -g->ball_dx;
            g->sfx = SFX_WALL_HIT; 
        }

        // Top wall
        if (g->ball_y <= 0) {
            g->ball_y  = 0;
            g->ball_dy = -g->ball_dy;
            g->sfx = SFX_WALL_HIT; 
        }

        // Paddle collision
        if (g->ball_y + BALL_SIZE >= PADDLE_Y &&
            g->ball_y + BALL_SIZE <= PADDLE_Y + PADDLE_H + 2 &&
            g->ball_x + BALL_SIZE >= g->paddle_x &&
            g->ball_x <= g->paddle_x + PADDLE_W)
        {
            g->ball_dy = -g->ball_dy;
            g->ball_y  = PADDLE_Y - BALL_SIZE;
            g->sfx = SFX_PADDLE_HIT;

            // Adjust dx based on where ball hits paddle
            // Left third → go left, right third → go right, middle → straight
            int16_t hit = (g->ball_x + BALL_SIZE/2) - 
                          (g->paddle_x + PADDLE_W/2);
            if      (hit < -4) g->ball_dx = -2;
            else if (hit >  4) g->ball_dx =  2;
            else               g->ball_dx =  (g->ball_dx > 0) ? 1 : -1;

            g->score++;
        }

        // Bottom — lose a life
        if (g->ball_y > OLED_H) {
            g->lives--;
            if (g->lives == 0) {
                g->state = GAME_STATE_GAMEOVER;
                g->sfx = SFX_GAMEOVER;
                g->blink_timer = 0;
                g->blink_on = true;
            } else {
                g->state      = GAME_STATE_DEAD;
                g->dead_timer = 60;   // ~1 second pause
                reset_ball(g);
            }
        }
        break;

    // ── Dead — brief pause before resuming ───────────────────────────────────
    case GAME_STATE_DEAD:
        g->dead_timer--;
        if (g->dead_timer == 0)
            g->state = GAME_STATE_PLAYING;
        break;

    // ── Game over ────────────────────────────────────────────────────────────
    case GAME_STATE_GAMEOVER:
        g->blink_timer++;
        if (g->blink_timer >= 30) {
            g->blink_timer = 0;
            g->blink_on = !g->blink_on;
        }
        if (action) {
            game_init(g);
            g->state = GAME_STATE_PLAYING;
        }
        break;
    }
}

// ── Render ───────────────────────────────────────────────────────────────────
void game_render(GameData *g)
{
    OLED_Clear();

    switch (g->state) {

    case GAME_STATE_TITLE:
        OLED_DrawString(32, 10, "PONG", 1, 3);
        if (g->blink_on)
            OLED_DrawString(43, 48, "PRESS A", 1, 1);
        break;

    case GAME_STATE_PLAYING:
    case GAME_STATE_DEAD:
        // Paddle
        OLED_FillRect(g->paddle_x, PADDLE_Y, PADDLE_W, PADDLE_H, 1);

        // Ball
        OLED_FillRect(g->ball_x, g->ball_y, BALL_SIZE, BALL_SIZE, 1);

        // Score (top right)
        {
            char buf[8];
            uint8_t s = g->score;
            buf[0] = '0' + (s / 10);
            buf[1] = '0' + (s % 10);
            buf[2] = '\0';
            OLED_DrawString(100, 2, buf, 1, 1);
        }

        // Lives (top left) — draw filled squares
        for (uint8_t i = 0; i < g->lives; i++)
            OLED_FillRect(2 + i * 6, 2, 4, 4, 1);

        // Divider line
        OLED_DrawHLine(0, 10, OLED_W, 1);

        if (g->state == GAME_STATE_DEAD && g->blink_on)
            OLED_DrawString(40, 28, "OOPS!", 1, 1);
        break;

    case GAME_STATE_GAMEOVER:
        OLED_DrawString(16, 10, "GAME OVER", 1, 2);
        {
            char buf[16];
            uint8_t s = g->score;
            buf[0]  = 'S'; buf[1]  = 'C'; buf[2]  = 'O';
            buf[3]  = 'R'; buf[4]  = 'E'; buf[5]  = ':';
            buf[6]  = ' ';
            buf[7]  = '0' + (s / 10);
            buf[8]  = '0' + (s % 10);
            buf[9]  = '\0';
            OLED_DrawString(28, 36, buf, 1, 1);
        }
        if (g->blink_on)
            OLED_DrawString(43, 52, "PRESS A", 1, 1);
        break;
    }

    OLED_Flush();
}

void game2p_tick(GameData *g, bool left_up, bool left_down,
                 bool right_up, bool right_down, bool action)
{
    g->sfx = SFX_NONE;
    switch (g->state) {

    case GAME_STATE_TITLE:
        g->blink_timer++;
        if (g->blink_timer >= 30) {
            g->blink_timer = 0;
            g->blink_on = !g->blink_on;
        }
        if (action) {
            g->state = GAME_STATE_PLAYING;
        }
        break;

    case GAME_STATE_PLAYING:
        if (left_up) {
            g->left_paddle_y -= P2_PADDLE_SPEED;
        }
        if (left_down) {
            g->left_paddle_y += P2_PADDLE_SPEED;
        }
        if (right_up) {
            g->right_paddle_y -= P2_PADDLE_SPEED;
        }
        if (right_down) {
            g->right_paddle_y += P2_PADDLE_SPEED;
        }
        clamp_2p_paddle(&g->left_paddle_y);
        clamp_2p_paddle(&g->right_paddle_y);

        g->ball_x += g->ball_dx;
        g->ball_y += g->ball_dy;

        if (g->ball_y <= 0) {
            g->ball_y = 0;
            g->ball_dy = -g->ball_dy;
            g->sfx = SFX_WALL_HIT; 
        }
        if (g->ball_y + BALL_SIZE >= OLED_H) {
            g->ball_y = OLED_H - BALL_SIZE;
            g->ball_dy = -g->ball_dy;
            g->sfx = SFX_WALL_HIT; 
        }

        if (g->ball_dx < 0 &&
            g->ball_x <= P2_LEFT_X + P2_PADDLE_W &&
            g->ball_x + BALL_SIZE >= P2_LEFT_X &&
            g->ball_y + BALL_SIZE >= g->left_paddle_y &&
            g->ball_y <= g->left_paddle_y + P2_PADDLE_H)
        {
            g->ball_x = P2_LEFT_X + P2_PADDLE_W;
            g->ball_dx = 2;
            adjust_2p_ball_y(g, g->left_paddle_y);
            g->sfx = SFX_PADDLE_HIT;
        }

        if (g->ball_dx > 0 &&
            g->ball_x + BALL_SIZE >= P2_RIGHT_X &&
            g->ball_x <= P2_RIGHT_X + P2_PADDLE_W &&
            g->ball_y + BALL_SIZE >= g->right_paddle_y &&
            g->ball_y <= g->right_paddle_y + P2_PADDLE_H)
        {
            g->ball_x = P2_RIGHT_X - BALL_SIZE;
            g->ball_dx = -2;
            adjust_2p_ball_y(g, g->right_paddle_y);
            g->sfx = SFX_PADDLE_HIT;
        }

        if (g->ball_x + BALL_SIZE < 0) {
            g->p2_score++;
            if (g->p2_score >= P2_WIN_SCORE) {
                g->state = GAME_STATE_GAMEOVER;
                g->sfx = SFX_GAMEOVER;
                g->blink_timer = 0;
                g->blink_on = true;
            } else {
                reset_2p_ball(g, -1);
                g->sfx = SFX_SCORE;
            }
        }

        if (g->ball_x > OLED_W) {
            g->p1_score++;
            if (g->p1_score >= P2_WIN_SCORE) {
                g->state = GAME_STATE_GAMEOVER;
                g->sfx = SFX_GAMEOVER;
                g->blink_timer = 0;
                g->blink_on = true;
            } else {
                reset_2p_ball(g, 1);
                g->sfx = SFX_SCORE;
            }
        }
        break;

    case GAME_STATE_DEAD:
        g->state = GAME_STATE_PLAYING;
        break;

    case GAME_STATE_GAMEOVER:
        g->blink_timer++;
        if (g->blink_timer >= 30) {
            g->blink_timer = 0;
            g->blink_on = !g->blink_on;
        }
        if (action) {
            game2p_init(g);
            g->state = GAME_STATE_PLAYING;
        }
        break;
    }
}

void game2p_render(GameData *g)
{
    OLED_Clear();

    switch (g->state) {

    case GAME_STATE_TITLE:
        OLED_DrawString(22, 10, "2P PONG", 1, 2);
        OLED_DrawString(22, 34, "L:UP/DN R:D/B", 1, 1);
        if (g->blink_on) {
            OLED_DrawString(43, 50, "PRESS A", 1, 1);
        }
        break;

    case GAME_STATE_PLAYING:
    case GAME_STATE_DEAD:
        OLED_FillRect(P2_LEFT_X, g->left_paddle_y,
                      P2_PADDLE_W, P2_PADDLE_H, 1);
        OLED_FillRect(P2_RIGHT_X, g->right_paddle_y,
                      P2_PADDLE_W, P2_PADDLE_H, 1);
        OLED_FillRect(g->ball_x, g->ball_y, BALL_SIZE, BALL_SIZE, 1);

        for (uint8_t y = 0; y < OLED_H; y += 8) {
            OLED_DrawVLine(OLED_W / 2, y, 4, 1);
        }

        {
            char score[4];
            score[0] = '0' + g->p1_score;
            score[1] = '-';
            score[2] = '0' + g->p2_score;
            score[3] = '\0';
            OLED_DrawString(55, 2, score, 1, 1);
        }
        break;

    case GAME_STATE_GAMEOVER:
        if (g->p1_score >= P2_WIN_SCORE) {
            OLED_DrawString(22, 12, "P1 WINS", 1, 2);
        } else {
            OLED_DrawString(22, 12, "P2 WINS", 1, 2);
        }
        {
            char score[4];
            score[0] = '0' + g->p1_score;
            score[1] = '-';
            score[2] = '0' + g->p2_score;
            score[3] = '\0';
            OLED_DrawString(55, 38, score, 1, 1);
        }
        if (g->blink_on) {
            OLED_DrawString(43, 52, "A: MENU", 1, 1);
        }
        break;
    }

    OLED_Flush();
}

void platformer_init(GameData *g)
{
    memset(g, 0, sizeof(GameData));
    g->player_x = 6;
    g->player_y = PLAT_GROUND_Y - PLAT_PLAYER_H;
    g->player_vy = 0;
    g->camera_x = 0;
    g->player_grounded = true;
    g->platformer_won = false;
    g->state = GAME_STATE_TITLE;
    g->blink_timer = 0;
    g->blink_on = true;

    // Speedrun timer — cleared until the run actually starts
    g->run_ticks       = 0;
    g->run_ticks_final = 0;
    g->timer_active    = false;
}

void platformer_tick(GameData *g, bool left, bool right, bool jump)
{
    switch (g->state) {

    case GAME_STATE_TITLE:
        g->blink_timer++;
        if (g->blink_timer >= 30) {
            g->blink_timer = 0;
            g->blink_on = !g->blink_on;
        }
        if (jump) {
            // Start the speedrun clock the moment the run begins
            g->run_ticks    = 0;
            g->timer_active = true;
            g->state = GAME_STATE_PLAYING;
        }
        break;

    case GAME_STATE_PLAYING:
    {
        // ── Speedrun clock ────────────────────────────────────────────────────
        // Increment every game tick. To use the RTC peripheral instead, replace
        // this block with an elapsed-time read:
        //
        //   uint32_t now_sec = DL_RTC_getCalendarSeconds(RTC);
        //   uint32_t now_sub = DL_RTC_getCalendarSubseconds(RTC); // 1/256 s
        //   g->run_ticks = (now_sec - g->rtc_start_sec) * PLAT_TICKS_PER_SEC
        //                + (now_sub * PLAT_TICKS_PER_SEC) / 256u;
        //
        // For the tick-counter approach (no extra hardware needed):
        if (g->timer_active) {
            g->run_ticks++;
        }

        int16_t old_y = g->player_y;
        int16_t prev_bottom;
        int16_t new_bottom;

        if (left) {
            g->player_x -= PLAT_MOVE_SPEED;
        }
        if (right) {
            g->player_x += PLAT_MOVE_SPEED;
        }
        if (g->player_x < 0) {
            g->player_x = 0;
        }
        if (g->player_x > PLAT_WORLD_W - PLAT_PLAYER_W) {
            g->player_x = PLAT_WORLD_W - PLAT_PLAYER_W;
        }

        if (jump && g->player_grounded) {
            g->player_vy = PLAT_JUMP_VY;
            g->player_grounded = false;
        }

        if (g->player_vy < PLAT_MAX_FALL) {
            g->player_vy++;
        }
        g->player_y += g->player_vy;

        prev_bottom = old_y + PLAT_PLAYER_H;
        new_bottom = g->player_y + PLAT_PLAYER_H;
        g->player_grounded = false;

        if (g->player_vy >= 0) {
            for (uint8_t i = 0;
                 i < (uint8_t)(sizeof(platformer_platforms) /
                                sizeof(platformer_platforms[0]));
                 i++)
            {
                const PlatformerRect *platform = &platformer_platforms[i];

                if (prev_bottom <= platform->y &&
                    new_bottom >= platform->y &&
                    g->player_x + PLAT_PLAYER_W > platform->x &&
                    g->player_x < platform->x + platform->w)
                {
                    g->player_y = platform->y - PLAT_PLAYER_H;
                    g->player_vy = 0;
                    g->player_grounded = true;
                    break;
                }
            }

            if (!g->player_grounded &&
                !platformer_over_pit(g->player_x, PLAT_PLAYER_W) &&
                new_bottom >= PLAT_GROUND_Y)
            {
                g->player_y = PLAT_GROUND_Y - PLAT_PLAYER_H;
                g->player_vy = 0;
                g->player_grounded = true;
            }
        }

        for (uint8_t i = 0;
             i < (uint8_t)(sizeof(platformer_spikes) /
                            sizeof(platformer_spikes[0]));
             i++)
        {
            if (platformer_hits_rect(g, &platformer_spikes[i])) {
                platformer_die(g);
                break;
            }
        }

        if (g->state == GAME_STATE_PLAYING) {
            for (uint8_t i = 0;
                 i < (uint8_t)(sizeof(platformer_enemies) /
                                sizeof(platformer_enemies[0]));
                 i++)
            {
                if (platformer_hits_rect(g, &platformer_enemies[i])) {
                    platformer_die(g);
                    break;
                }
            }
        }

        if (g->state == GAME_STATE_PLAYING && g->player_y > OLED_H) {
            platformer_die(g);
        }
        if (g->state == GAME_STATE_PLAYING && g->player_x >= PLAT_GOAL_X) {
            platformer_win(g);
        }

        g->camera_x = g->player_x - 42;
        if (g->camera_x < 0) {
            g->camera_x = 0;
        }
        if (g->camera_x > PLAT_WORLD_W - OLED_W) {
            g->camera_x = PLAT_WORLD_W - OLED_W;
        }
        break;
    }

    case GAME_STATE_DEAD:
        platformer_die(g);
        break;

    case GAME_STATE_GAMEOVER:
        g->blink_timer++;
        if (g->blink_timer >= 30) {
            g->blink_timer = 0;
            g->blink_on = !g->blink_on;
        }
        break;
    }
}

static void platformer_draw_ground(GameData *g)
{
    int16_t start_x = 0;

    for (uint8_t i = 0;
         i < (uint8_t)(sizeof(platformer_pits) / sizeof(platformer_pits[0]));
         i++)
    {
        draw_world_rect(g, start_x, PLAT_GROUND_Y,
                        platformer_pits[i].start_x - start_x,
                        OLED_H - PLAT_GROUND_Y, 1);
        start_x = platformer_pits[i].end_x;
    }

    draw_world_rect(g, start_x, PLAT_GROUND_Y,
                    PLAT_WORLD_W - start_x, OLED_H - PLAT_GROUND_Y, 1);
}

static void platformer_draw_level(GameData *g)
{
    platformer_draw_ground(g);

    for (uint8_t i = 0;
         i < (uint8_t)(sizeof(platformer_platforms) /
                        sizeof(platformer_platforms[0]));
         i++)
    {
        draw_world_rect(g, platformer_platforms[i].x,
                        platformer_platforms[i].y,
                        platformer_platforms[i].w,
                        platformer_platforms[i].h, 1);
    }

    for (uint8_t i = 0;
         i < (uint8_t)(sizeof(platformer_spikes) /
                        sizeof(platformer_spikes[0]));
         i++)
    {
        draw_world_rect(g, platformer_spikes[i].x,
                        platformer_spikes[i].y,
                        platformer_spikes[i].w,
                        platformer_spikes[i].h, 1);
    }

    for (uint8_t i = 0;
         i < (uint8_t)(sizeof(platformer_enemies) /
                        sizeof(platformer_enemies[0]));
         i++)
    {
        int16_t screen_x = platformer_enemies[i].x - g->camera_x;

        draw_world_rect(g, platformer_enemies[i].x,
                        platformer_enemies[i].y,
                        platformer_enemies[i].w,
                        platformer_enemies[i].h, 1);
        if (screen_x >= -6 && screen_x < OLED_W) {
            OLED_DrawPixel(screen_x + 1, platformer_enemies[i].y + 1, 0);
            OLED_DrawPixel(screen_x + 5, platformer_enemies[i].y + 1, 0);
        }
    }

    draw_world_rect(g, PLAT_GOAL_X, 30, 2, 26, 1);
    draw_world_rect(g, PLAT_GOAL_X + 2, 30, 12, 7, 1);
}

// ── Speedrun timer formatter ──────────────────────────────────────────────────
// Produces "M:SS.tt" (minutes, seconds, centiseconds) into buf[9].
// Uses PLAT_TICKS_PER_SEC to convert raw ticks to real time.
static void format_run_time(char *buf, uint32_t ticks)
{
    // Convert ticks → centiseconds
    uint32_t cs   = (ticks * 100u) / PLAT_TICKS_PER_SEC;
    uint32_t mins = cs / 6000u;
    uint32_t secs = (cs % 6000u) / 100u;
    uint32_t cent = cs % 100u;

    buf[0] = '0' + (char)(mins % 10u);
    buf[1] = ':';
    buf[2] = '0' + (char)(secs / 10u);
    buf[3] = '0' + (char)(secs % 10u);
    buf[4] = '.';
    buf[5] = '0' + (char)(cent / 10u);
    buf[6] = '0' + (char)(cent % 10u);
    buf[7] = '\0';
}

void platformer_render(GameData *g)
{
    OLED_Clear();

    switch (g->state) {

    case GAME_STATE_TITLE:
        OLED_DrawString(31, 8, "PLATFORM", 1, 1);
        OLED_DrawString(16, 24, "L/R RUN  A JUMP", 1, 1);
        OLED_DrawString(19, 38, "DODGE BAD STUFF", 1, 1);
        if (g->blink_on) {
            OLED_DrawString(43, 52, "PRESS A", 1, 1);
        }
        break;

    case GAME_STATE_PLAYING:
    case GAME_STATE_DEAD:
    {
        int16_t screen_x;

        platformer_draw_level(g);

        screen_x = g->player_x - g->camera_x;
        OLED_FillRect(screen_x, g->player_y,
                      PLAT_PLAYER_W, PLAT_PLAYER_H, 1);
        OLED_DrawPixel(screen_x + 4, g->player_y + 2, 0);

        // Live speedrun timer — top-right corner, format "M:SS.tt"
        {
            char tbuf[8];
            format_run_time(tbuf, g->run_ticks);
            // 7 chars × 6px = 42px wide; right-align at x=86 so it ends at 128
            OLED_DrawString(86, 0, tbuf, 1, 1);
        }
        break;
    }

    case GAME_STATE_GAMEOVER:
        if (g->platformer_won) {
            OLED_DrawString(16, 4, "YOU WIN!", 1, 2);

            // Show the final speedrun time prominently
            {
                char tbuf[8];
                format_run_time(tbuf, g->run_ticks_final);
                OLED_DrawString(19, 26, "TIME:", 1, 1);
                // Centre the 7-char time string (42px) → x = (128-42)/2 = 43
                OLED_DrawString(43, 36, tbuf, 1, 1);
            }
        } else {
            OLED_DrawString(12, 10, "YOU DIED", 1, 2);
        }
        OLED_DrawString(16, 48, "A RETURNS MENU", 1, 1);
        if (g->blink_on) {
            OLED_DrawString(43, 56, "PRESS A", 1, 1);
        }
        break;
    }

    OLED_Flush();
}
