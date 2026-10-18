#include "alien.h"

/**
 * INSTRUCTION DEFINITIONS
 */

AlienInstruction ais[ALIEN_INSTRUCTION_COUNT] = {
    { .opcode = 0xab, .function = &alien_instruction_load },
    { .opcode = 0xac, .function = &alien_instruction_get_paint },
    { .opcode = 0xad, .function = &alien_instruction_paint },
    { .opcode = 0xb0, .function = &alien_instruction_cmp },
    { .opcode = 0xb1, .function = &alien_instruction_set },
    { .opcode = 0xb3, .function = &alien_instruction_jmp_eq },
    { .opcode = 0xb4, .function = &alien_instruction_jmp_ne },
    { .opcode = 0xb6, .function = &alien_instruction_jmp_le },
    { .opcode = 0xba, .function = &alien_instruction_print },
    { .opcode = 0xbc, .function = &alien_instruction_halt },
    { .opcode = 0xbd, .function = &alien_instruction_add },
    { .opcode = 0xbf, .function = &alien_instruction_mult },
    { .opcode = 0xc0, .function = &alien_instruction_div },
    { .opcode = 0xc1, .function = &alien_instruction_mod },
    { .opcode = 0xc2, .function = &alien_instruction_and },
    { .opcode = 0xc6, .function = &alien_instruction_set_bit },
    { .opcode = 0xc7, .function = &alien_instruction_unset_bit }
};

char *alien_alphabet[8] = {
    "ɕ", "ʕ", "Φ", "Җ", "Г", "ͼ", "ɍ", "ȩ"
};

/**
 * HELPER FUNCTIONS
 */

uint64_t get_register(AlienComputer *ac, uint8_t r)
{
    if (r < REGISTER_COUNT)
        return ac->registers[r];

    printf("Invalid memory access: register 0x%02x is out of bounds 0x%02x\n", r, REGISTER_COUNT);

    // Non ideal Error handling, but this will likely result in another error, trade off with complexity is fine
    return UINT64_MAX;
}

void set_register(AlienComputer *ac, uint8_t r, uint64_t v)
{
    if (r < REGISTER_COUNT) {
        ac->registers[r] = v;
        return;
    }

    printf("Invalid memory access: register 0x%02x is out of bounds 0x%02x\n", r, REGISTER_COUNT);

    // Non ideal Error handling, but this will likely result in another error, trade off with complexity is fine
}

OctalU64 u64_to_o64(uint64_t value)
{
    OctalU64 ret = { .data = { 0 } };

    for (size_t o = 0; o < sizeof(ret); o++) {
        uint64_t temp = value;
        for (size_t i = 0; i < o; i++)
            temp = (temp / 8);
        ret.data[o] = (uint8_t) (temp % 8);
        if (temp == 0)
            o = sizeof(ret);
    }

    return ret;
}

void print_display(AlienComputer ac)
{
    for (size_t r = 0; r < DISPLAY_SIZE; r++) {
        for (size_t c = 0; c < 8; c++) {
            uint8_t byte = ac.display[r][c];
            for (size_t j = 0; j < 8; j++) {
                uint8_t bit = (byte >> j) & 1;
                printf("%c", bit ? '#' : '.');
            }
            printf(" ");
        }
        printf("\n");
    }
}

/**
 * INSTRUCTION IMPLEMENTATIONS
 */

int alien_instruction_load(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r;
        uint64_t v;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 1 byte target register and 8 bytes value
    a = (struct args *) (&ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = 0x%016lx \n", a->r, a->v);

    // r = v
    set_register(ac, a->r, a->v);

    return 0;
}

int alien_instruction_get_paint(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t unknown[8];
        uint8_t r2;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 1 byte input offset r2 and 1 byte output register r1
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = DISPLAY[%ld][%ld]", a->r1, get_register(ac, a->r2) / 8, get_register(ac, a->r2) % 8);

    // r1 = display[r2 / 8][r2 % 8]
    uint64_t offset = get_register(ac, a->r2);
    set_register(ac, a->r1, 0);
    for (size_t i = 0; i < 8; i++) {
        uint8_t row = (offset + i) / 8;
        uint8_t col = (offset + i) % 8;
        uint64_t paint = ((uint64_t) ac->display[row][col]) << (8 * i);
        set_register(ac, a->r1, get_register(ac, a->r1) + paint);
    }

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}

int alien_instruction_paint(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t unknown[8];
        uint8_t r2;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 1 byte input register r1 and 1 byte offset register r2
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("DISPLAY[%ld][%ld] = r%1X(0x%016lx)\n", get_register(ac, a->r2) / 8, get_register(ac, a->r2) % 8, a->r1, get_register(ac, a->r1));

    // display[r2 / 8][r2 % 8] = r1
    uint64_t color_palette = get_register(ac, a->r1);
    uint64_t offset = get_register(ac, a->r2);
    for (size_t i = 0; i < 8; i++) {
        uint8_t row = (offset + i) / 8;
        uint8_t col = (offset + i) % 8;
        uint8_t *color = (uint8_t *) &color_palette;
        uint8_t paint = *(color + i);
        ac->display[row][col] = paint;
    }

    return 0;
}

int alien_instruction_cmp(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 2 bytes input registers
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("CMP( r%1X(0x%016lx), r%1X(0x%016lx) )", a->r1, get_register(ac, a->r1), a->r2, get_register(ac, a->r2));

    // rF = cmp(r1, r2)
    ac->registers[ALIEN_FLAGS_REGISTER] &= ~AI_FLAGS_CMP_MASK;
    if (get_register(ac, a->r1) == get_register(ac, a->r2))
        ac->registers[ALIEN_FLAGS_REGISTER] |= AI_CMP_EQ;
    else if (get_register(ac, a->r1) < get_register(ac, a->r2))
        ac->registers[ALIEN_FLAGS_REGISTER] |= AI_CMP_LT;
    else
        ac->registers[ALIEN_FLAGS_REGISTER] |= AI_CMP_GT;

    if (ac->_debug)
        printf(" = r%1X(0x%016lx)\n", ALIEN_FLAGS_REGISTER, get_register(ac, ALIEN_FLAGS_REGISTER) & AI_FLAGS_CMP_MASK);

    return 0;
}

int alien_instruction_set(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 1 byte target register and 1 byte input register
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = r%1X(0x%016lx)\n", a->r1, a->r2, get_register(ac, a->r2));

    // r1 = r2
    set_register(ac, a->r1, get_register(ac, a->r2));

    return 0;
}

int alien_instruction_jmp_eq(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint64_t addr;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read target address
    a = (struct args *) (&ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = 0x%016lx IF ( r%1X(0x%016lx) == 0x%02x )\n", ALIEN_PROGRAM_COUNTER_REGISTER, a->addr, ALIEN_FLAGS_REGISTER, get_register(ac, ALIEN_FLAGS_REGISTER), AI_CMP_EQ);

    // set pointer position to address
    if ((ac->registers[ALIEN_FLAGS_REGISTER] & AI_FLAGS_CMP_MASK) == AI_CMP_EQ)
        ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] = a->addr;

    return 0;
}

int alien_instruction_jmp_ne(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint64_t addr;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read target address
    a = (struct args *) (&ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = 0x%016lx IF ( r%1X(0x%016lx) != 0x%02x )\n", ALIEN_PROGRAM_COUNTER_REGISTER, a->addr, ALIEN_FLAGS_REGISTER, get_register(ac, ALIEN_FLAGS_REGISTER), AI_CMP_EQ);

    // set pointer position to address
    if ((ac->registers[ALIEN_FLAGS_REGISTER] & AI_FLAGS_CMP_MASK) != AI_CMP_EQ)
        ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] = a->addr;

    return 0;
}

int alien_instruction_jmp_le(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint64_t addr;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read target address
    a = (struct args *) (&ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = 0x%016lx IF ( r%1X(0x%016lx) != 0x%02x )\n", ALIEN_PROGRAM_COUNTER_REGISTER, a->addr, ALIEN_FLAGS_REGISTER, get_register(ac, ALIEN_FLAGS_REGISTER), AI_CMP_GT);

    // set pointer position to address
    if ((ac->registers[ALIEN_FLAGS_REGISTER] & AI_FLAGS_CMP_MASK) != AI_CMP_GT)
        ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] = a->addr;

    return 0;
}

int alien_instruction_print(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 1 byte target register
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X(0x%016lx) = ", a->r, get_register(ac, a->r));

    // print r
    OctalU64 oct = u64_to_o64(get_register(ac, a->r));
    size_t last;
    for (last = sizeof(oct); oct.data[last - 1] == 0 && last > 1; last--)
        ;
    for (size_t o = 0; o < last; o++) {
        printf("%s ", alien_alphabet[oct.data[o] % 8]);
    }
    printf("\n");

    return 0;
}

int alien_instruction_halt(AlienComputer *ac)
{
    if (ac == NULL)
        return -1;

    if (ac->_debug)
        printf("HALT\n");

    return 0;
}

int alien_instruction_add(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
        uint8_t r3;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 1 byte target register and 2 bytes input registers
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = r%1X(0x%016lx) + r%1X(0x%016lx)", a->r1, a->r2, get_register(ac, a->r2), a->r3, get_register(ac, a->r3));

    // r1 = r2 + r3
    set_register(ac, a->r1, get_register(ac, a->r2) + get_register(ac, a->r3));

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}

int alien_instruction_mult(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
        uint8_t r3;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 2 input registers r2, r3 and one output register r1
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = r%1X(0x%016lx) * r%1X(0x%016lx)", a->r1, a->r2, get_register(ac, a->r2), a->r3, get_register(ac, a->r3));

    // r1 = r2 * r3
    set_register(ac, a->r1, get_register(ac, a->r2) * get_register(ac, a->r3));

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}

int alien_instruction_div(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
        uint8_t r3;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 2 input registers r2, r3 and one output register r1
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = r%1X(0x%016lx) / r%1X(0x%016lx)", a->r1, a->r2, get_register(ac, a->r2), a->r3, get_register(ac, a->r3));

    // r1 = r2 / r3
    set_register(ac, a->r1, get_register(ac, a->r2) / get_register(ac, a->r3));

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}

int alien_instruction_mod(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
        uint8_t r3;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 2 input registers r2, r3 and one output register r1
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = r%1X(0x%016lx) %% r%1X(0x%016lx)", a->r1, a->r2, get_register(ac, a->r2), a->r3, get_register(ac, a->r3));

    // r1 = r2 % r3
    set_register(ac, a->r1, get_register(ac, a->r2) % get_register(ac, a->r3));

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}

int alien_instruction_and(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
        uint8_t r3;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 2 input registers r2, r3 and one output register r1
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X = r%1X(0x%016lx) & r%1X(0x%016lx)", a->r1, a->r2, get_register(ac, a->r2), a->r3, get_register(ac, a->r3));

    // r1 = r2 & r3
    set_register(ac, a->r1, get_register(ac, a->r2) & get_register(ac, a->r3));

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}

int alien_instruction_set_bit(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 2 input registers r1, r2
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X(0x%016lx) |= 1 << r%1X(0x%016lx)", a->r1, get_register(ac, a->r1), a->r2, get_register(ac, a->r2));

    // r1 |= (1 << r2)
    set_register(ac, a->r1, get_register(ac, a->r1) | ((uint64_t) 1 << get_register(ac, a->r2)));

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}

int alien_instruction_unset_bit(AlienComputer *ac)
{
    struct __attribute__((packed)) args {
        uint8_t r1;
        uint8_t r2;
    };

    struct args *a;

    if (ac == NULL)
        return -1;

    // read 2 input registers r1, r2
    a = (struct args *) &(ac->memory[ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER]]);
    ac->registers[ALIEN_PROGRAM_COUNTER_REGISTER] += sizeof(*a);

    if (ac->_debug)
        printf("r%1X(0x%016lx) &= ~(1 << r%1X(0x%016lx))", a->r1, get_register(ac, a->r1), a->r2, get_register(ac, a->r2));

    // r1 &= ~(1 << r2)
    set_register(ac, a->r1, get_register(ac, a->r1) & ~((uint64_t) 1 << get_register(ac, a->r2)));

    if (ac->_debug)
        printf(" = 0x%016lx\n", get_register(ac, a->r1));

    return 0;
}
