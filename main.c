#include "ti_msp_dl_config.h"
#include "oled.h"
#include "input.h"
#include "game.h"
#include "tetris.h"
#include "buzzer.h"

volatile bool g_tick = false;

#define MENU_ITEM_PONG        0
#define MENU_ITEM_2P_PONG     1
#define MENU_ITEM_PLATFORMER  2
#define MENU_ITEM_TETRIS      3
#define MENU_ITEM_BUTTON_TEST 4
#define MENU_ITEM_COUNT       5

typedef enum {
    SPLASH_PHASE_GAMEFOLK,
    SPLASH_PHASE_V1,
    SPLASH_PHASE_WAIT
} SplashPhase;

static SplashPhase splash_phase       = SPLASH_PHASE_GAMEFOLK;
static int16_t     splash_offset      = 128;
static int16_t     v1_offset          = 128;
static uint8_t     splash_blink_timer = 0;
static bool        splash_blink_on    = false;
static bool tetris_failing = false;

static void menu_init(MenuState *menu)
{
    menu->selected    = 0;
    menu->blink_timer = 0;
    menu->blink_on    = true;
}

static void draw_splash(void)
{
    OLED_Clear();
    OLED_DrawString(16 - splash_offset, 18, "GAMEFOLK", 1, 2);
    if (splash_phase != SPLASH_PHASE_GAMEFOLK)
        OLED_DrawString(58 + v1_offset, 38, "V1", 1, 1);
    if (splash_phase == SPLASH_PHASE_WAIT && splash_blink_on)
        OLED_DrawString(37, 54, "PRESS A", 1, 1);
    OLED_Flush();
}

void draw_main_menu(uint8_t selected, bool blink_on)
{
    OLED_Clear();
    OLED_DrawString(34, 4, "MAIN MENU", 1, 1);
    OLED_DrawHLine(0, 14, OLED_W, 1);

    const char *apps[] = {
        "PONG", "2P PONG", "PLATFORMER", "TETRIS", "BUTTON TEST"
    };
    for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
        uint8_t y = 16 + (i * 8);
        if (i == selected && blink_on)
            OLED_DrawString(12, y, ">", 1, 1);
        OLED_DrawString(25, y, apps[i], 1, 1);
    }
    OLED_DrawString(31, 56, "UP/DOWN + A", 1, 1);
    OLED_Flush();
}

static void draw_button_label(uint8_t x, uint8_t y,
                               const char *label, bool pressed)
{
    if (pressed) OLED_DrawString(x, y, ">", 1, 1);
    OLED_DrawString(x + 10, y, label, 1, 1);
}

void draw_button_test(const ButtonState *buttons)
{
    OLED_Clear();
    OLED_DrawString(22, 2, "BUTTON TEST", 1, 1);
    OLED_DrawHLine(0, 12, OLED_W, 1);
    draw_button_label(0,  18, "UP",    buttons->up);
    draw_button_label(0,  28, "DOWN",  buttons->down);
    draw_button_label(0,  38, "LEFT",  buttons->left);
    draw_button_label(0,  48, "RIGHT", buttons->right);
    draw_button_label(70, 18, "A",     buttons->a);
    draw_button_label(70, 28, "B",     buttons->b);
    draw_button_label(70, 38, "C",     buttons->c);
    draw_button_label(70, 48, "D",     buttons->d);
    OLED_DrawString(37, 56, "L+R: MENU", 1, 1);
    OLED_Flush();
}

void TIMA0_IRQHandler(void)
{
    if (DL_TimerA_getPendingInterrupt(TIMER_0_INST) == DL_TIMER_IIDX_ZERO)
        g_tick = true;
}

int main(void)
{
    SYSCFG_DL_init();
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    DL_TimerA_startCounter(TIMER_0_INST);
    OLED_Init();
    buzzer_init();

    GameData   game;
    TetrisData tetris;

    game_init(&game);
    game.app_state = APP_STATE_SPLASH;
    menu_init(&game.menu);
    tetris_init(&tetris);

    ButtonState buttons          = {0};
    ButtonState previous_buttons = {0};

    while (1) {
        buzzer_set_music_enabled(game.app_state == APP_STATE_MENU);

        if (g_tick) {
            g_tick = false;
            jingle_tick();
            input_update(&buttons);

            bool up_pressed   = buttons.up   && !previous_buttons.up;
            bool down_pressed = buttons.down  && !previous_buttons.down;
            bool a_pressed    = buttons.a     && !previous_buttons.a;

            switch (game.app_state) {

            case APP_STATE_SPLASH:
                switch (splash_phase) {
                case SPLASH_PHASE_GAMEFOLK:
                    if (splash_offset > 0) {
                        splash_offset -= 6;
                        if (splash_offset < 0) splash_offset = 0;
                    } else {
                        splash_phase = SPLASH_PHASE_V1;
                        v1_offset    = 128;
                    }
                    break;
                case SPLASH_PHASE_V1:
                    if (v1_offset > 0) {
                        v1_offset -= 6;
                        if (v1_offset < 0) v1_offset = 0;
                    } else {
                        splash_phase = SPLASH_PHASE_WAIT;
                    }
                    break;
                case SPLASH_PHASE_WAIT:
                    splash_blink_timer++;
                    if (splash_blink_timer >= 30) {
                        splash_blink_timer = 0;
                        splash_blink_on    = !splash_blink_on;
                    }
                    if (a_pressed) {
                        game.app_state = APP_STATE_MENU;
                        menu_init(&game.menu);
                    }
                    break;
                }
                draw_splash();
                break;

            case APP_STATE_MENU:
                game.menu.blink_timer++;
                if (game.menu.blink_timer >= 10) {
                    game.menu.blink_timer = 0;
                    game.menu.blink_on    = !game.menu.blink_on;
                }
                if (up_pressed) {
                    game.menu.selected = (game.menu.selected == 0) ?
                        MENU_ITEM_COUNT - 1 : game.menu.selected - 1;
                }
                if (down_pressed) {
                    game.menu.selected++;
                    if (game.menu.selected >= MENU_ITEM_COUNT)
                        game.menu.selected = 0;
                }
                draw_main_menu(game.menu.selected, game.menu.blink_on);
                if (a_pressed) {
                    switch (game.menu.selected) {
                    case MENU_ITEM_PONG:
                        game_init(&game);
                        game.app_state = APP_STATE_PONG;
                        break;
                    case MENU_ITEM_2P_PONG:
                        game2p_init(&game);
                        game.app_state = APP_STATE_2P_PONG;
                        break;
                    case MENU_ITEM_PLATFORMER:
                        platformer_init(&game);
                        game.app_state = APP_STATE_PLATFORMER;
                        break;
                    case MENU_ITEM_TETRIS:
                    tetris_init(&tetris);
                    tetris_failing = false;
                    game.app_state = APP_STATE_TETRIS;
                    break;
                    case MENU_ITEM_BUTTON_TEST:
                        game.app_state = APP_STATE_BUTTON_TEST;
                        break;
                    }
                }
                break;

            case APP_STATE_PONG:
                if (game.state == GAME_STATE_GAMEOVER && a_pressed) {
                    game.app_state = APP_STATE_MENU;
                    menu_init(&game.menu);
                } else {
                    game_tick(&game, buttons.left, buttons.right, a_pressed);
                    dispatch_game_sfx(game.sfx);
                    game.sfx = SFX_NONE;
                    game_render(&game);
                }
                break;

            case APP_STATE_2P_PONG:
                if (game.state == GAME_STATE_GAMEOVER && a_pressed) {
                    game.app_state = APP_STATE_MENU;
                    menu_init(&game.menu);
                } else {
                    game2p_tick(&game, buttons.up, buttons.down,
                                buttons.d, buttons.b, a_pressed);
                    dispatch_game_sfx(game.sfx);
                    game.sfx = SFX_NONE;
                    game2p_render(&game);
                }
                break;

            case APP_STATE_PLATFORMER:
                if (game.state == GAME_STATE_GAMEOVER && a_pressed) {
                    game.app_state = APP_STATE_MENU;
                    menu_init(&game.menu);
                } else {
                    platformer_tick(&game, buttons.left, buttons.right,
                                    a_pressed);
                    dispatch_game_sfx(game.sfx);
                    game.sfx = SFX_NONE;
                    platformer_render(&game);
                }
                break;

            case APP_STATE_BUTTON_TEST:
                draw_button_test(&buttons);
                if (buttons.left && buttons.right)
                    game.app_state = APP_STATE_MENU;
                break;

            case APP_STATE_TETRIS:
                tetris_tick(&tetris,
                            buttons.left, buttons.right,
                            buttons.down,
                            a_pressed,
                            buttons.b);

                // Dispatch SFX
                if (tetris.sfx != TET_SFX_NONE) {
                    dispatch_tetris_sfx(tetris.sfx);
                    if (tetris.sfx == TET_SFX_GAMEOVER)
                        tetris_failing = true;
                    tetris.sfx = TET_SFX_NONE;
                }

                // Hold on playing state until fail jingle finishes
                if (tetris_failing) {
                    if (jingle_finished()) {
                        tetris_failing = false;
                        // Now let gameover screen show
                    } else {
                        // Keep rendering playing state during jingle
                        tetris_render(&tetris);
                        break;
                    }
                }

                tetris_render(&tetris);

                if (tetris.state == TET_STATE_GAMEOVER && a_pressed && !tetris_failing) {
                    game.app_state = APP_STATE_MENU;
                    menu_init(&game.menu);
                    tetris_init(&tetris);
                }
                break;
            }

            previous_buttons = buttons;
        }
    }
}