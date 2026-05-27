#include "chip8.h"
#include "roms.h"
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define TIMER_HZ 60.0
#define TIMER_INTERVAL (1.0 / TIMER_HZ) // ≈ 0.01667 секунды

typedef struct {
    CHIP8_State state;
    Sound beep;
    Image icon;
    bool game_running;
    bool game_running_background;
    int selected_game_index;
    int cycles_per_frame;
    int games_count;
    int game_select_window_start;
    int game_select_window_end;
} EmulatorContext;

int max_val(int a, int b) {
    if (a > b)
        return a;
    return b;
}

double get_time_in_seconds() { return GetTime(); }

double last_timer_tick;

void update_timers(CHIP8_State *state, Sound *beep) {
    static bool first_call = true;
    double now = get_time_in_seconds();

    if (first_call) {
        last_timer_tick = now;
        first_call = false;
        return;
    }

    while (last_timer_tick + TIMER_INTERVAL <= now) {
        if (state->DT > 0)
            state->DT--;
        if (state->ST > 0)
            state->ST--;
        last_timer_tick += TIMER_INTERVAL;
    }

    if (state->ST > 0) {
        PlaySound(*beep);
    }
}

void load_rom_from_memory(CHIP8_State *state, const unsigned char *rom_data,
                          unsigned int rom_size) {
    init_chip8(state);
    memcpy(&state->ram[0x200], rom_data, rom_size);
}

void update_draw_frame(void *arg) {
    EmulatorContext *ctx = (EmulatorContext *)arg;

    if (ctx->game_running) {

        if (IsKeyPressed(KEY_ESCAPE)) {
            ctx->game_running = false;
            ctx->game_running_background = true;
        }

        update_timers(&ctx->state, &ctx->beep);

        memset(ctx->state.keypad, 0, sizeof(ctx->state.keypad));

        if (IsKeyDown(KEY_ONE))
            ctx->state.keypad[0x1] = true;
        if (IsKeyDown(KEY_TWO))
            ctx->state.keypad[0x2] = true;
        if (IsKeyDown(KEY_THREE))
            ctx->state.keypad[0x3] = true;
        if (IsKeyDown(KEY_FOUR))
            ctx->state.keypad[0xC] = true;
        if (IsKeyDown(KEY_Q))
            ctx->state.keypad[0x4] = true;
        if (IsKeyDown(KEY_W))
            ctx->state.keypad[0x5] = true;
        if (IsKeyDown(KEY_E))
            ctx->state.keypad[0x6] = true;
        if (IsKeyDown(KEY_R))
            ctx->state.keypad[0xD] = true;
        if (IsKeyDown(KEY_A))
            ctx->state.keypad[0x7] = true;
        if (IsKeyDown(KEY_S))
            ctx->state.keypad[0x8] = true;
        if (IsKeyDown(KEY_D))
            ctx->state.keypad[0x9] = true;
        if (IsKeyDown(KEY_F))
            ctx->state.keypad[0xE] = true;
        if (IsKeyDown(KEY_Z))
            ctx->state.keypad[0xA] = true;
        if (IsKeyDown(KEY_X))
            ctx->state.keypad[0x0] = true;
        if (IsKeyDown(KEY_C))
            ctx->state.keypad[0xB] = true;
        if (IsKeyDown(KEY_V))
            ctx->state.keypad[0xF] = true;

        for (int i = 0; i < ctx->cycles_per_frame; i++) {
            emulate_cycle(&ctx->state);
        }

        if (ctx->state.has_error) {
            ctx->game_running_background = false;
            ctx->game_running = false;
        }

        render(&ctx->state);
    } else {
        if (ctx->state.has_error) {
            if (IsKeyPressed(KEY_ENTER)) {
                ctx->state.has_error = false;
            }

            BeginDrawing();
            ClearBackground(BLACK);

            int error_width = MeasureText(ctx->state.error_msg, 20);
            DrawText(ctx->state.error_msg, 320 - error_width / 2, 120, 20, RED);

            const char *sub_msg = "Press ENTER to return to menu";
            int sub_width = MeasureText(sub_msg, 16);
            DrawText(sub_msg, 320 - sub_width / 2, 180, 16, LIGHTGRAY);

            EndDrawing();
            return;
        }
        if (IsKeyPressed(KEY_UP)) {
            ctx->selected_game_index =
                (ctx->selected_game_index - 1 + ctx->games_count) % ctx->games_count;
            if (ctx->selected_game_index < ctx->game_select_window_start) {
                ctx->game_select_window_start = ctx->selected_game_index;
                ctx->game_select_window_end = ctx->selected_game_index + 4;
            } else if (ctx->selected_game_index >= ctx->game_select_window_end) {
                ctx->game_select_window_start =
                    max_val(ctx->selected_game_index - 3, 0);
                ctx->game_select_window_end = ctx->selected_game_index + 1;
            }
        }
        if (IsKeyPressed(KEY_DOWN)) {
            ctx->selected_game_index = (ctx->selected_game_index + 1) % ctx->games_count;
            if (ctx->selected_game_index >= ctx->game_select_window_end) {
                ctx->game_select_window_start =
                    max_val(ctx->selected_game_index - 3, 0);
                ctx->game_select_window_end = ctx->selected_game_index + 1;
            } else if (ctx->selected_game_index < ctx->game_select_window_start) {
                ctx->game_select_window_start = ctx->selected_game_index;
                ctx->game_select_window_end = ctx->selected_game_index + 4;
            }
        }
        if (IsKeyPressed(KEY_LEFT)) {
            if (ctx->cycles_per_frame > 1)
                ctx->cycles_per_frame--;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            if (ctx->cycles_per_frame < 100)
                ctx->cycles_per_frame++;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            load_rom_from_memory(&ctx->state, g_roms[ctx->selected_game_index].data,
                                 g_roms[ctx->selected_game_index].size);
            ctx->game_running = true;
            last_timer_tick = get_time_in_seconds();
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (ctx->game_running_background) {
                ctx->game_running = true;
                last_timer_tick = get_time_in_seconds();
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("CHIP-8 Emulator", 50, 30, 24, RAYWHITE);
        DrawText("Select game (UP/DOWN) & Press ENTER", 50, 70, 16,
                 LIGHTGRAY);
        for (int i = 0; i < ctx->games_count; i++) {
            if (i >= ctx->game_select_window_start &&
                i < ctx->game_select_window_end) {
                if (i == ctx->selected_game_index) {
                    DrawText(TextFormat("> %s <", g_roms[i].name), 70,
                             110 + (i - ctx->game_select_window_start) * 25, 18,
                             YELLOW);
                } else {
                    DrawText(g_roms[i].name, 70,
                             110 + (i - ctx->game_select_window_start) * 25, 18,
                             RAYWHITE);
                }
            }
        }

        DrawText("Emulator Speed (LEFT/RIGHT)", 50, 240, 16, LIGHTGRAY);
        DrawText(TextFormat("Cycles per frame: %d (~%d Hz)",
                            ctx->cycles_per_frame, ctx->cycles_per_frame * 60),
                 70, 265, 18, GREEN);
        EndDrawing();
    }
}

int main() {
    static EmulatorContext ctx;
    srand(GetTime());

    InitWindow(640, 320, "CHIP-8 Emulator");
    SetExitKey(KEY_NULL);
    InitAudioDevice();

    ctx.beep = LoadSound("assets/sounds/beep.wav");
    ctx.icon = LoadImage("assets/icon/eight.png");
    SetWindowIcon(ctx.icon);

    init_chip8(&ctx.state);

    SetTargetFPS(60);

    ctx.game_running = false;
    ctx.game_running_background = false;

    last_timer_tick = get_time_in_seconds();

    ctx.selected_game_index = 0;
    ctx.cycles_per_frame = 8;
    ctx.games_count = sizeof(g_roms) / sizeof(g_roms[0]);

    ctx.game_select_window_start = 0;
    ctx.game_select_window_end = 4;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(update_draw_frame, &ctx, 0, 1);
#else
    while (!WindowShouldClose()) {
        update_draw_frame(&ctx);
    }
#endif

    UnloadSound(ctx.beep);
    UnloadImage(ctx.icon);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
