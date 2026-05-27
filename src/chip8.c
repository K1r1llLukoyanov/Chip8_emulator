#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_err(CHIP8_State *state, uint16_t command) {
#ifdef DEBUG
    printf("Unknown command: %x\n", command);
    exit(1);
#else
    state->has_error = true;
    snprintf(state->error_msg, sizeof(state->error_msg),
             "Unknown opcode: 0x%04X", command);
#endif
}

void dispatch_command(CHIP8_State *state, uint16_t command) {
    uint8_t opcode_h = (command & 0xF000) >> 12;
    uint8_t opcode_l = (command & 0x0F00) >> 8;
    uint16_t address = (command & 0x0FFF);
    uint8_t Vx = (command & 0x0F00) >> 8;
    uint8_t Vy = (command & 0x00F0) >> 4;
    uint8_t N = (command & 0x000F);
    uint8_t NN = (command & 0x00FF);
    switch (opcode_h) {
    case 0: {
        if (command == 0x00E0)
            clear_screen(state);
        else if (command == 0x00EE)
            execute_return(state);
        else
            print_err(state, command);
        break;
    }
    case 1: {
        execute_jump(state, address);
        break;
    }
    case 2: {
        execute_call(state, address);
        break;
    }
    case 3: {
        execute_skip_eq_const(state, Vx, NN);
        break;
    }
    case 4: {
        execute_skip_neq_const(state, Vx, NN);
        break;
    }
    case 5: {
        if (!N)
            execute_skip_eq_regs(state, Vx, Vy);
        else
            print_err(state, command);
        break;
    }
    case 6: {
        execute_set_const(state, Vx, NN);
        break;
    }
    case 7: {
        execute_add_const(state, Vx, NN);
        break;
    }
    case 8: {
        switch (N) {
        case 0:
            execute_set_reg(state, Vx, Vy);
            break;
        case 1:
            execute_or(state, Vx, Vy);
            break;
        case 2:
            execute_and(state, Vx, Vy);
            break;
        case 3:
            execute_xor(state, Vx, Vy);
            break;
        case 4:
            execute_add_reg(state, Vx, Vy);
            break;
        case 5:
            execute_sub_reg(state, Vx, Vy);
            break;
        case 6:
            execute_shr(state, Vx);
            break;
        case 7:
            execute_sub_inv(state, Vx, Vy);
            break;
        case 0xE:
            execute_shl(state, Vx);
            break;
        default:
            print_err(state, command);
            break;
        }
        break;
    }
    case 9: {
        execute_skip_neq_regs(state, Vx, Vy);
        break;
    }
    case 0xA: {
        execute_set_addr(state, address);
        break;
    }
    case 0xB: {
        execute_jump_plus(state, address);
        break;
    }
    case 0xC: {
        execute_rand(state, Vx, NN);
        break;
    }
    case 0xD: {
        execute_draw(state, Vx, Vy, N);
        break;
    }
    case 0xE: {
        switch (NN) {
        case 0x9E:
            execute_skip_key_pressed(state, Vx);
            break;
        case 0xA1:
            execute_skip_key_not_pressed(state, Vx);
            break;
        default:
            print_err(state, command);
        }
        break;
    }
    case 0xF: {
        switch (NN) {
        case 0x07: {
            execute_get_delay(state, Vx);
            break;
        }
        case 0x0A: {
            execute_get_key(state, Vx);
            break;
        }
        case 0x15: {
            execute_delay_timer(state, Vx);
            break;
        }
        case 0x18: {
            execute_sound_timer(state, Vx);
            break;
        }
        case 0x1E: {
            execute_add_mem(state, Vx);
            break;
        }
        case 0x29: {
            execute_sprite_addr(state, Vx);
            break;
        }
        case 0x33: {
            execute_set_bcd(state, Vx);
            break;
        }
        case 0x55: {
            execute_reg_dump(state, Vx);
            break;
        }
        case 0x65: {
            execute_reg_load(state, Vx);
            break;
        }
        default:
            print_err(state, command);
        }
        break;
    }
    default: {
        print_err(state, command);
    }
    }
}

void execute_call(CHIP8_State *state, uint16_t address) {
    if (state->SP >= 16) {
#ifdef DEBUG
        printf("Error: Stack is overloaded! (16 subroutine depth max)\n");
        exit(1);
#else
        state->has_error = true;
        snprintf(state->error_msg, sizeof(state->error_msg),
                 "Stack overflow! (max depth 16)");
#endif
    }
    state->stack[state->SP++] = state->PC;
    state->PC = address;
}

void execute_jump(CHIP8_State *state, uint16_t address) { state->PC = address; }

void clear_screen(CHIP8_State *state) {
    memset(state->display, 0, sizeof(state->display));
    fflush(stdout);
}

void execute_return(CHIP8_State *state) {
    if (state->SP < 1) {
#ifdef DEBUG
        printf("Error: stack is empty!\n");
        exit(1);
#else
        state->has_error = true;
        snprintf(state->error_msg, sizeof(state->error_msg),
                 "Error: can't return function! No function had been called!");
#endif
    }
    state->PC = state->stack[--state->SP];
}

void execute_rcall(CHIP8_State *state, uint16_t addr) {
    execute_call(state, addr);
}

void execute_skip_eq_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val) {
    uint8_t reg_val = state->regs[Vx];
    if (reg_val == const_val)
        state->PC += 2;
}

void execute_skip_neq_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val) {
    uint8_t reg_val = state->regs[Vx];
    if (reg_val != const_val)
        state->PC += 2;
}

void execute_skip_eq_regs(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    uint8_t Vx_val = state->regs[Vx];
    uint8_t Vy_val = state->regs[Vy];
    if (Vx_val == Vy_val)
        state->PC += 2;
}

void execute_set_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val) {
    state->regs[Vx] = const_val;
}

void execute_add_const(CHIP8_State *state, uint8_t Vx, uint8_t const_val) {
    state->regs[Vx] += const_val;
}

void execute_set_reg(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    state->regs[Vx] = state->regs[Vy];
}

void execute_or(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    state->regs[Vx] |= state->regs[Vy];
}

void execute_and(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    state->regs[Vx] &= state->regs[Vy];
}

void execute_xor(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    state->regs[Vx] ^= state->regs[Vy];
}

void execute_add_reg(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    uint16_t sum = state->regs[Vx] + state->regs[Vy];
    uint8_t flag = (sum > 255) ? 1 : 0;
    state->regs[Vx] = sum & 0xFF;
    state->regs[0xF] = flag;
}

void execute_sub_reg(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    uint8_t x_val = state->regs[Vx];
    uint8_t y_val = state->regs[Vy];
    uint8_t result = x_val - y_val;
    uint8_t flag = (x_val >= y_val) ? 1 : 0;
    state->regs[Vx] = result;
    state->regs[0xF] = flag;
}

void execute_shr(CHIP8_State *state, uint8_t Vx) {
    uint8_t val = state->regs[Vx];
    uint8_t flag = val & 0x1;
    state->regs[Vx] = val >> 1;
    state->regs[0xF] = flag;
}

void execute_sub_inv(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    uint8_t x_val = state->regs[Vx];
    uint8_t y_val = state->regs[Vy];
    uint8_t result = y_val - x_val;
    uint8_t flag = (y_val >= x_val) ? 1 : 0;
    state->regs[Vx] = result;
    state->regs[0xF] = flag;
}

void execute_shl(CHIP8_State *state, uint8_t Vx) {
    uint8_t val = state->regs[Vx];
    uint8_t flag = (val >> 7) & 0x1;
    state->regs[Vx] = val << 1;
    state->regs[0xF] = flag;
}

void execute_skip_neq_regs(CHIP8_State *state, uint8_t Vx, uint8_t Vy) {
    if (state->regs[Vx] != state->regs[Vy])
        state->PC += 2;
}

void execute_set_addr(CHIP8_State *state, uint16_t addr) {
    state->address_reg = addr;
}

void execute_jump_plus(CHIP8_State *state, uint16_t addr) {
    state->PC = addr + state->regs[0];
}

void execute_rand(CHIP8_State *state, uint8_t Vx, uint8_t const_val) {
    state->regs[Vx] = (rand() % 256) & const_val;
}

void execute_draw(CHIP8_State *state, uint8_t Vx, uint8_t Vy, uint8_t N) {
    uint8_t x = state->regs[Vx] % DISPLAY_WIDTH;
    uint8_t y = state->regs[Vy] % DISPLAY_HEIGHT;
    state->regs[0xF] = 0;

    for (int row = 0; row < N; row++) {
        if (state->address_reg + row >= 4096)
            break;
        uint8_t sprite_byte = state->ram[state->address_reg + row];
        for (int col = 0; col < 8; col++) {
            if ((sprite_byte >> (7 - col)) & 0x1) {
                int px = x + col;
                int py = y + row;

                if (px >= DISPLAY_WIDTH || py >= DISPLAY_HEIGHT)
                    continue;

                int index = py * DISPLAY_WIDTH + px;
                if (state->display[index] == 1) {
                    state->regs[0xF] = 1;
                }
                state->display[index] ^= 1;
            }
        }
    }
}

void execute_skip_key_pressed(CHIP8_State *state, uint8_t Vx) {
    uint8_t key = state->regs[Vx];
    if (key < 16 && state->keypad[key]) {
        state->PC += 2;
    }
}

void execute_skip_key_not_pressed(CHIP8_State *state, uint8_t Vx) {
    uint8_t key = state->regs[Vx];
    if (key >= 16 || !state->keypad[key]) {
        state->PC += 2;
    }
}

void execute_get_key(CHIP8_State *state, uint8_t Vx) {
    for (int i = 0; i < 16; i++) {
        if (state->keypad[i]) {
            state->regs[Vx] = i;
            state->waiting_for_key = false;
            return;
        }
    }
    state->waiting_for_key = true;
    state->key_register = Vx;
}

void execute_get_delay(CHIP8_State *state, uint8_t Vx) {
    state->regs[Vx] = state->DT;
}

void execute_delay_timer(CHIP8_State *state, uint8_t Vx) {
    state->DT = state->regs[Vx];
}

void execute_sound_timer(CHIP8_State *state, uint8_t Vx) {
    state->ST = state->regs[Vx];
}

void execute_add_mem(CHIP8_State *state, uint8_t Vx) {
    state->address_reg = (state->address_reg + state->regs[Vx]) & 0xFFF;
}

void execute_sprite_addr(CHIP8_State *state, uint8_t Vx) {
    state->address_reg = (state->regs[Vx] & 0xF) * 5;
}

void execute_set_bcd(CHIP8_State *state, uint8_t Vx) {
    uint8_t value = state->regs[Vx];
    if (state->address_reg <= 4093) {
        state->ram[state->address_reg] = value / 100;
        state->ram[state->address_reg + 1] = (value / 10) % 10;
        state->ram[state->address_reg + 2] = value % 10;
    }
}

void execute_reg_dump(CHIP8_State *state, uint8_t Vx) {
    uint16_t store_addr = state->address_reg;
    for (int i = 0; i <= Vx; i++) {
        if (store_addr < 4096) {
            state->ram[store_addr++] = state->regs[i];
        }
    }
}

void execute_reg_load(CHIP8_State *state, uint8_t Vx) {
    uint16_t store_addr = state->address_reg;
    for (int i = 0; i <= Vx; i++) {
        if (store_addr < 4096) {
            state->regs[i] = state->ram[store_addr++];
        }
    }
}

void emulate_cycle(CHIP8_State *state) {
    if (state->waiting_for_key) {
        execute_get_key(state, state->key_register);
        return;
    }

    uint8_t command_h = state->ram[state->PC];
    uint8_t command_l = state->ram[state->PC + 1];
    uint16_t command = (command_h << 8) | command_l;
    state->PC += 2;
    dispatch_command(state, command);
}

void init_chip8(CHIP8_State *state) {
    memset(state, 0, sizeof(CHIP8_State));
    memcpy(state->ram, chip8_font_sprite, 80);
    state->PC = 0x200;
}

void render_raylib(CHIP8_State *state) {
    BeginDrawing();
    ClearBackground(BLACK);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (state->display[y * 64 + x]) {
                DrawRectangle(x * 10, y * 10, 10, 10, WHITE);
            }
        }
    }

    EndDrawing();
}

void render(CHIP8_State *state) { render_raylib(state); }
