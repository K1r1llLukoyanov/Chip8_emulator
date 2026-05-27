#ifndef __CHIP8_H
#define __CHIP8_H

#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define SCALE 10

typedef struct CHIP8_State {
    uint16_t stack[16];
    uint8_t regs[16];
    uint8_t ram[4096];
    uint8_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    char error_msg[64];
    bool keypad[16];
    uint16_t PC;
    uint16_t address_reg;
    uint8_t DT;
    uint8_t ST;
    uint8_t SP;
    uint8_t key_register;
    bool waiting_for_key;
    bool key_was_pressed;
    bool has_error;
} CHIP8_State;

static const uint8_t chip8_font_sprite[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void execute_call(CHIP8_State *state, uint16_t address);
void execute_jump(CHIP8_State *state, uint16_t address);
void clear_screen(CHIP8_State *state);
void execute_return(CHIP8_State *state);
void execute_rcall(CHIP8_State *state, uint16_t addr);
void execute_skip_eq_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val);
void execute_skip_neq_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val);
void execute_skip_eq_regs(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_set_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val);
void execute_add_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val);
void execute_set_reg(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_or(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_and(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_xor(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_add_reg(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_sub_reg(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_shr(CHIP8_State *state, uint8_t Vx);
void execute_sub_inv(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_shl(CHIP8_State *state, uint8_t Vx);
void execute_skip_neq_regs(CHIP8_State *state, uint8_t Vx, uint8_t Vy);
void execute_set_addr(CHIP8_State *state, uint16_t addr);
void execute_jump_plus(CHIP8_State *state, uint16_t addr);
void execute_rand(CHIP8_State *state, uint8_t Vx, uint8_t const_val);
void execute_draw(CHIP8_State *state, uint8_t Vx, uint8_t Vy, uint8_t N);
void execute_skip_key_pressed(CHIP8_State *state, uint8_t Vx);
void execute_skip_key_not_pressed(CHIP8_State *state, uint8_t Vx);
void execute_get_key(CHIP8_State *state, uint8_t Vx);
void execute_get_delay(CHIP8_State *state, uint8_t Vx);
void execute_delay_timer(CHIP8_State *state, uint8_t Vx);
void execute_sound_timer(CHIP8_State *state, uint8_t Vx);
void execute_add_mem(CHIP8_State *state, uint8_t Vx);
void execute_sprite_addr(CHIP8_State *state, uint8_t Vx);
void execute_set_bcd(CHIP8_State *state, uint8_t Vx);
void execute_reg_dump(CHIP8_State *state, uint8_t Vx);
void execute_reg_load(CHIP8_State *state, uint8_t Vx);

void load_rom(CHIP8_State *state, const char *filepath);
void init_chip8(CHIP8_State *state);
void emulate_cycle(CHIP8_State *state);

void play_beep();
void render(CHIP8_State *state);

#endif
