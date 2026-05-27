#include "chip8.h"
#include "roms.h"
#include <raylib.h>
#include <stdlib.h>

#define TIMER_HZ 60.0
#define TIMER_INTERVAL (1.0 / TIMER_HZ) // ≈ 0.01667 секунды

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

int main() {
    CHIP8_State state;
    srand(GetTime());

    bool running = true;
    InitWindow(640, 320, "CHIP-8 Emulator");
    SetExitKey(KEY_NULL);
    InitAudioDevice();

    Sound beep = LoadSound("assets/sounds/beep.wav");
    init_chip8(&state);

    SetTargetFPS(60);

    bool game_running = false;
    bool game_running_background = false;

    last_timer_tick = get_time_in_seconds();

    int selected_game_index = 0;
    int cycles_per_frame = 8;
    int games_count = sizeof(g_roms) / sizeof(g_roms[0]);

    int game_select_window_start = 0;
    int game_select_window_end = 4;

    while (!WindowShouldClose()) {
        if (game_running) {

            if (IsKeyPressed(KEY_ESCAPE)) {
                game_running = false;
                game_running_background = true;
            }

            update_timers(&state, &beep);

            memset(state.keypad, 0, sizeof(state.keypad));

            if (IsKeyDown(KEY_ONE))
                state.keypad[0x1] = true;
            if (IsKeyDown(KEY_TWO))
                state.keypad[0x2] = true;
            if (IsKeyDown(KEY_THREE))
                state.keypad[0x3] = true;
            if (IsKeyDown(KEY_FOUR))
                state.keypad[0xC] = true;
            if (IsKeyDown(KEY_Q))
                state.keypad[0x4] = true;
            if (IsKeyDown(KEY_W))
                state.keypad[0x5] = true;
            if (IsKeyDown(KEY_E))
                state.keypad[0x6] = true;
            if (IsKeyDown(KEY_R))
                state.keypad[0xD] = true;
            if (IsKeyDown(KEY_A))
                state.keypad[0x7] = true;
            if (IsKeyDown(KEY_S))
                state.keypad[0x8] = true;
            if (IsKeyDown(KEY_D))
                state.keypad[0x9] = true;
            if (IsKeyDown(KEY_F))
                state.keypad[0xE] = true;
            if (IsKeyDown(KEY_Z))
                state.keypad[0xA] = true;
            if (IsKeyDown(KEY_X))
                state.keypad[0x0] = true;
            if (IsKeyDown(KEY_C))
                state.keypad[0xB] = true;
            if (IsKeyDown(KEY_V))
                state.keypad[0xF] = true;

            for (int i = 0; i < cycles_per_frame; i++) {
                emulate_cycle(&state);
            }

            if (state.has_error) {
                game_running_background = false;
                game_running = false;
            }

            render(&state);
        } else {
            if (state.has_error) {
                if (IsKeyPressed(KEY_ENTER)) {
                    state.has_error = false;
                }

                BeginDrawing();
                ClearBackground(BLACK);

                int error_width = MeasureText(state.error_msg, 20);
                DrawText(state.error_msg, 320 - error_width / 2, 120, 20, RED);

                const char *sub_msg = "Press ENTER to return to menu";
                int sub_width = MeasureText(sub_msg, 16);
                DrawText(sub_msg, 320 - sub_width / 2, 180, 16, LIGHTGRAY);

                EndDrawing();
                continue;
            }
            if (IsKeyPressed(KEY_UP)) {
                selected_game_index =
                    (selected_game_index - 1 + games_count) % games_count;
                if (selected_game_index < game_select_window_start) {
                    game_select_window_start = selected_game_index;
                    game_select_window_end = selected_game_index + 4;
                } else if (selected_game_index >= game_select_window_end) {
                    game_select_window_start = max_val(selected_game_index - 3, 0);
                    game_select_window_end = selected_game_index + 1;
                }
            }
            if (IsKeyPressed(KEY_DOWN)) {
                selected_game_index = (selected_game_index + 1) % games_count;
                if (selected_game_index >= game_select_window_end) {
                    game_select_window_start = max_val(selected_game_index - 3, 0);
                    game_select_window_end = selected_game_index + 1;
                } else if (selected_game_index < game_select_window_start) {
                    game_select_window_start = selected_game_index;
                    game_select_window_end = selected_game_index + 4;
                }
            }
            if (IsKeyPressed(KEY_LEFT)) {
                if (cycles_per_frame > 1)
                    cycles_per_frame--;
            }
            if (IsKeyPressed(KEY_RIGHT)) {
                if (cycles_per_frame < 100)
                    cycles_per_frame++;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                load_rom_from_memory(&state, g_roms[selected_game_index].data,
                                     g_roms[selected_game_index].size);
                game_running = true;
                last_timer_tick = get_time_in_seconds();
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                if (game_running_background) {
                    game_running = true;
                    last_timer_tick = get_time_in_seconds();
                }
            }

            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("CHIP-8 Emulator", 50, 30, 24, RAYWHITE);
            DrawText("Select game (UP/DOWN) & Press ENTER", 50, 70, 16,
                     LIGHTGRAY);
            for (int i = 0; i < games_count; i++) {
                if (i >= game_select_window_start &&
                    i < game_select_window_end) {
                    if (i == selected_game_index) {
                        DrawText(TextFormat("> %s <", g_roms[i].name), 70,
                                 110 + (i - game_select_window_start) * 25, 18,
                                 YELLOW);
                    } else {
                        DrawText(g_roms[i].name, 70,
                                 110 + (i - game_select_window_start) * 25, 18,
                                 RAYWHITE);
                    }
                }
            }

            DrawText("Emulator Speed (LEFT/RIGHT)", 50, 240, 16, LIGHTGRAY);
            DrawText(TextFormat("Cycles per frame: %d (~%d Hz)",
                                cycles_per_frame, cycles_per_frame * 60),
                     70, 265, 18, GREEN);
            EndDrawing();
        }
    }

    UnloadSound(beep);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
