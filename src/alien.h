#ifndef __ALIEN_H
#define __ALIEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define REGISTER_COUNT 16
#define MAX_CYCLES 1000000
#define DISPLAY_SIZE 64
#define MAX_MEM 1024

typedef struct alien_computer {
    uint64_t registers[REGISTER_COUNT];
    uint8_t memory[MAX_MEM];
    uint32_t cycles;
    uint8_t display[DISPLAY_SIZE][8];
    bool _debug;
} AlienComputer;

#define ALIEN_PROGRAM_COUNTER_REGISTER 0xe
#define ALIEN_FLAGS_REGISTER 0xf

#define AI_FLAGS_CMP_MASK 0x03
#define AI_CMP_GT 0x0
#define AI_CMP_EQ 0x1
#define AI_CMP_LT 0x2

typedef struct alien_instruction {
    uint8_t opcode;
    int (*function)(AlienComputer *);
} AlienInstruction;

typedef enum alien_instruction_e {
    ALIEN_INSTRUCTION_LOAD,
    ALIEN_INSTRUCTION_GET_PAINT,
    ALIEN_INSTRUCTION_PAINT,
    ALIEN_INSTRUCTION_CMP,
    ALIEN_INSTRUCTION_SET,
    ALIEN_INSTRUCTION_JMP_EQ,
    ALIEN_INSTRUCTION_JMP_LT,
    ALIEN_INSTRUCTION_JMP_GT,
    ALIEN_INSTRUCTION_PRINT,
    ALIEN_INSTRUCTION_HALT,
    ALIEN_INSTRUCTION_ADD,
    ALIEN_INSTRUCTION_MULT,
    ALIEN_INSTRUCTION_DIV,
    ALIEN_INSTRUCTION_MOD,
    ALIEN_INSTRUCTION_AND,
    ALIEN_INSTRUCTION_SET_BIT,
    ALIEN_INSTRUCTION_UNSET_BIT,
    ALIEN_INSTRUCTION_COUNT
} AlienInstructionE;

typedef struct octal_u64 {
    uint8_t data[22];
} OctalU64;

extern AlienInstruction ais[ALIEN_INSTRUCTION_COUNT];

int alien_instruction_load(AlienComputer *ac);
int alien_instruction_get_paint(AlienComputer *ac);
int alien_instruction_paint(AlienComputer *ac);
int alien_instruction_cmp(AlienComputer *ac);
int alien_instruction_set(AlienComputer *ac);
int alien_instruction_jmp_eq(AlienComputer *ac);
int alien_instruction_jmp_ne(AlienComputer *ac);
int alien_instruction_jmp_le(AlienComputer *ac);
int alien_instruction_print(AlienComputer *ac);
int alien_instruction_halt(AlienComputer *ac);
int alien_instruction_add(AlienComputer *ac);
int alien_instruction_mult(AlienComputer *ac);
int alien_instruction_div(AlienComputer *ac);
int alien_instruction_mod(AlienComputer *ac);
int alien_instruction_and(AlienComputer *ac);
int alien_instruction_set_bit(AlienComputer *ac);
int alien_instruction_unset_bit(AlienComputer *ac);

void print_display(AlienComputer ac);

#endif
