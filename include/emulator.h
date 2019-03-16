#ifndef C8_EMULATOR_H
#define C8_EMULATOR_H

#include "config.h"
#include "registers.h"

#define C8_MEMORY_SIZE 0x1000
#define C8_DISPLAY_WIDTH 64
#define C8_DISPLAY_HEIGHT 32
#define C8_DISPLAY_SIZE (C8_DISPLAY_WIDTH * C8_DISPLAY_HEIGHT)
#define C8_STACK_SIZE 24
#define C8_KEY_COUNT 16

typedef struct c8_emulator {
	uint8 memory[C8_MEMORY_SIZE];
	uint8 pixels[C8_DISPLAY_SIZE];
	uint16 stack[C8_STACK_SIZE];
	c8_registers_t registers;

	union {
		struct {
			uint8 k0 : 1;
			uint8 k1 : 1;
			uint8 k2 : 1;
			uint8 k3 : 1;
			uint8 k4 : 1;
			uint8 k5 : 1;
			uint8 k6 : 1;
			uint8 k7 : 1;
			uint8 k8 : 1;
			uint8 k9 : 1;
			uint8 ka : 1;
			uint8 kb : 1;
			uint8 kc : 1;
			uint8 kd : 1;
			uint8 ke : 1;
			uint8 kf : 1;
		};

		uint16 keys;
	};

	uint16 stack_pointer;
	uint16 address_register_i;
	uint16 program_counter;
	uint8 delay_timer;
	uint8 sound_timer;
	uint8 draw_flag;
} c8_emulator_t;

void c8_emulator_init(c8_emulator_t* const emulator);
int32 c8_emulator_load_rom(c8_emulator_t* const emulator, const char* rom);
void c8_emulator_cycle(c8_emulator_t* const emulator);

#endif
