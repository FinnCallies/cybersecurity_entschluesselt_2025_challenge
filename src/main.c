#include "alien.h"

// Usage Helper
void usage(char *argv0)
{
    printf("\nUsage: %s <program.alienbin> [OPTIONS]\n\n", argv0);
    printf("Options:\n");
    printf("\t-d\t\t\tEnable debug mode\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    AlienComputer ac = {
        .registers = { 0 },
        .memory = { 0 },
        .cycles = 0,
        ._debug = false
    };
    char *filename = NULL;
    bool halt = false;
    FILE *fp;

    // Init Display
    for (size_t r = 0; r < DISPLAY_SIZE * 8; r++)
        for (size_t c = 0; c < 8; c++)
            ac.display[r][c] = 0;

    // Check CLI
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    // Parse CLI
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && !ac._debug) {
            ac._debug = true;
        } else if (!filename) {
            filename = argv[i];
        } else {
            usage(argv[0]);
            return 1;
        }
    }
    if (!filename) {
        usage(argv[0]);
        return 1;
    }

    // load program
    fp = fopen(filename, "r");
    if (!fp) {
        printf("Unable to open file: %s\n", filename);
        return 1;
    }
    if (fread(ac.memory, sizeof(ac.memory[0]), sizeof(ac.memory), fp) == 0) {
        printf("Unable to read from %s\n", filename);
        return 1;
    }
    fclose(fp);

    // Instruction Cycle
    while (!halt && ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER] < sizeof(ac.memory) && ac.cycles <= MAX_CYCLES) {
        int (*current_instruction)(AlienComputer *) = NULL;
        uint16_t old_pc = ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER];
        uint8_t current_opcode;

        // Check Instruction
        for (size_t i = 0; i < ALIEN_INSTRUCTION_COUNT && !current_instruction; i++)
            if (ais[i].opcode == ac.memory[ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER]])
                current_instruction = ais[i].function;
        if (!current_instruction) {
            printf("Invalid instruction 0x%02x at 0x%3lx after %d cycles\n", ac.memory[ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER]], ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER], ac.cycles);
            return 1;
        } else if (ac.memory[ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER]] == ais[ALIEN_INSTRUCTION_HALT].opcode) {
            halt = true;
        }

        // Fetch Instruction
        current_opcode = ac.memory[ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER]];

        // Progress PC
        ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER]++;

        // Decode AND Execute is handled by the respective instruction implementation
        if (current_instruction(&ac) != 0) {
            printf("Internal error during instruction 0x%02x at 0x%03x\n", current_opcode, old_pc);
            return 1;
        }

        // Progress Cycles
        ac.cycles++;
    }

    // Check for Errors
    if (ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER] >= sizeof(ac.memory)) {
        printf("Invalid memory access: 0x%03lx out of bounds 0x%03lx\n", ac.registers[ALIEN_PROGRAM_COUNTER_REGISTER], sizeof(ac.memory));
        return 1;
    } else if (ac.cycles > MAX_CYCLES) {
        printf("Exceeded max cycles %d\n", MAX_CYCLES);
        return 1;
    }

    // Otherwise Success
    printf("\nProgram finished successfully after %d cycles\n\n", ac.cycles);

    // Print Display
    print_display(ac);

    return 0;
}
