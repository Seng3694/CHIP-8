#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "emulator.h"

static uint8 fontset[0x50] =
{
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

static inline uint8 random_uint8(void)
{
	return rand() % 0x100;
}

void c8_emulator_init(c8_emulator_t* const emulator)
{
	memset(emulator->memory, 0, C8_MEMORY_SIZE);
	memcpy(emulator->memory, fontset, 0x50);
	memset(emulator->pixels, 0, C8_DISPLAY_SIZE);
	memset(emulator->stack, 0, C8_STACK_SIZE);
	memset(&emulator->registers, 0, sizeof(c8_registers_t));
	memset(emulator->keys, 0, C8_KEY_COUNT);
	emulator->stack_pointer = 0;
	emulator->address_register_i = 0;
	emulator->program_counter = 0x200;
	emulator->delay_timer = 0;
	emulator->sound_timer = 0;
	emulator->draw_flag = 1;
	srand((uint32)time(NULL));
}

int32 c8_emulator_load_rom(c8_emulator_t* const emulator, const char* rom)
{
	FILE *file = fopen(rom, "rb");

	if (file == NULL)
		return 1;

	fseek(file, 0, SEEK_END);
	const size_t size = (size_t)ftell(file);
	rewind(file);
	fread(&emulator->memory[0x200], 1, size, file);
	fclose(file);

	return 0;
}

void c8_emulator_cycle(c8_emulator_t* const emulator)
{
	const uint16 opcode = (emulator->memory[emulator->program_counter] << 8) | emulator->memory[emulator->program_counter + 1];
	const uint8 x = (uint8)((opcode & 0x0f00) >> 8);
	const uint8 y = (uint8)((opcode & 0x00f0) >> 4);

	switch (opcode & 0xf000)
	{
	case 0x0000:

		if (opcode == 0x00e0) //display clear
		{
			memset(emulator->pixels, 0, C8_DISPLAY_SIZE);
			emulator->draw_flag = 1;
			emulator->program_counter += 2;
		}
		else if (opcode == 0x00ee) //return
		{
			emulator->stack_pointer--;
			emulator->program_counter = emulator->stack[emulator->stack_pointer];
			emulator->program_counter += 2;
		}
		else //0NNN call RCA 1802 program at address NNN
		{
			emulator->program_counter += 2;
		}

		break;

	case 0x1000: //1NNN goto address nnn
		emulator->program_counter = (uint16)(opcode & 0x0fff);
		break;

	case 0x2000: //2NNN call subroutine at nnn
		emulator->stack[emulator->stack_pointer] = emulator->program_counter;
		emulator->stack_pointer++;
		emulator->program_counter = opcode & 0x0fff;
		break;

	case 0x3000: //3XNN skip if VX equal NN
		if (emulator->registers.v[x] == (uint8)(opcode & 0x00ff))
			emulator->program_counter += 4;
		else
			emulator->program_counter += 2;
		break;

	case 0x4000: //4XNN skip if VX unequal NN
		if(emulator->registers.v[x] != (uint8)(opcode & 0x00ff))
			emulator->program_counter += 4;
		else
			emulator->program_counter += 2;
		break;

	case 0x5000: //5XY0 skip if x equal y
		if(emulator->registers.v[x] == emulator->registers.v[y])
			emulator->program_counter += 4;
		else
			emulator->program_counter += 2;
		break;

	case 0x6000: //6XNN set VX to NN
		emulator->registers.v[x] = (uint8)(opcode & 0x00ff);
		emulator->program_counter += 2;
		break;

	case 0x7000: //7XNN add NN to VX (carry flag not changed)
		emulator->registers.v[x] += (uint8)(opcode & 0x00ff);
		emulator->program_counter += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000f)
		{
		case 0x0: //0x8XY0 assign VX = VY
			emulator->registers.v[x] = emulator->registers.v[y];
			emulator->program_counter += 2;
			break;

		case 0x1: //0x8XY1 VX |= VY
			emulator->registers.v[x] |= emulator->registers.v[y];
			emulator->program_counter += 2;
			break;

		case 0x2: //0x8XY2 VX &= VY
			emulator->registers.v[x] &= emulator->registers.v[y];
			emulator->program_counter += 2;
			break;

		case 0x3: //0x8XY3 VX ^= VY
			emulator->registers.v[x] ^= emulator->registers.v[y];
			emulator->program_counter += 2;
			break;

		case 0x4: //0x8XY4 VX += VY (VF => 1 if carry, 0 if not)
			/*i32 = emulator->registers.v[x] + emulator->registers.v[y];
			emulator->registers.vf = (i32 > 0xff);
			emulator->registers.v[x] = (uint8)(i32 % 0xff);*/
			emulator->registers.vf = emulator->registers.v[x] < emulator->registers.v[y];
			emulator->registers.v[x] += emulator->registers.v[y];
			emulator->program_counter += 2;
			break;

		case 0x5: //0x8XY5 VX -= VY (VF => 0 if there's a borrow. 1 if not)
			emulator->registers.vf = emulator->registers.v[x] > emulator->registers.v[y];
			emulator->registers.v[x] -= emulator->registers.v[y];
			emulator->program_counter += 2;
			break;

		case 0x6: //0x8XY6 VX >>= 1  (least significant bit in VF then shift)
			emulator->registers.vf = (emulator->registers.v[x] & 1);
			emulator->registers.v[x] >>= 1;
			emulator->program_counter += 2;
			break;

		case 0x7: //0x8XY7 VX = VY - VX (VF => 0 if borrow. 1 if not)
			emulator->registers.vf = emulator->registers.v[x] < emulator->registers.v[y];
			emulator->registers.v[x] = emulator->registers.v[y] - emulator->registers.v[x];
			emulator->program_counter += 2;
			break;

		case 0xe: //0x8XYE VX <<= 1 (most significant bit of VX in VF then shift)
			emulator->registers.vf = emulator->registers.v[x] >> 7;
			emulator->registers.v[x] <<= 1;
			emulator->program_counter += 2;
			break;
		}

		break;

	case 0x9000: //9XY0 skips next if VX != VY
		if (emulator->registers.v[x] != emulator->registers.v[y])
			emulator->program_counter += 4;
		else
			emulator->program_counter += 2;

		break;

	case 0xa000: //ANNN I = NNN
		emulator->address_register_i = (uint16)(opcode & 0x0fff);
		emulator->program_counter += 2;
		break;

	case 0xb000: //BNNN PC=V0+NNN
		emulator->program_counter = (uint16)(emulator->registers.v0 + (opcode & 0x0fff));
		break;

	case 0xc000: //CXNN VX=rand() & NN
		emulator->registers.v[x] = (uint8)(random_uint8() & (uint8)(opcode & 0x00ff));
		emulator->program_counter += 2;
		break;

	case 0xd000: //DXYN draws sprite at VX VY with width 8 px and height of N px. Each row of 8 px is read as bit-coded starting from mem loc I. I doesn't change. VF is set to 1 if any screen pixels are flipped, 0 if not
	{
		const uint8 height = opcode & 0x000f;
		uint8 pixel_line = 0;
		uint16 pixel_line_address = 0;

		emulator->registers.vf = 0;

		for (uint8 py = 0; py < height; ++py)
		{
			pixel_line = emulator->memory[emulator->address_register_i + (uint16)py];

			for (uint8 px = 0; px < 8; ++px)
			{
				if ((pixel_line & (0x80 >> px)) != 0)
				{
					pixel_line_address = (py + emulator->registers.v[y]) * C8_DISPLAY_WIDTH + px + emulator->registers.v[x];
					if (emulator->pixels[pixel_line_address] == 1)
						emulator->registers.vf = 1;
					emulator->pixels[pixel_line_address] ^= 1;
				}
			}
		}

		emulator->draw_flag = 1;
		emulator->program_counter += 2;
		break;
	}
	case 0xe000:
		switch (opcode & 0x00ff)
		{
		case 0x009e: //EX9E skips next if key stored in VX is pressed
			if (emulator->keys[emulator->registers.v[x]] != 0)
				emulator->program_counter += 4;
			else
				emulator->program_counter += 2;
			break;

		case 0x00a1: //EXA1 skips next if key stored in VX isn't pressed
			if (emulator->keys[emulator->registers.v[x]] == 0)
				emulator->program_counter += 4;
			else
				emulator->program_counter += 2;
			break;
		}

		break;

	case 0xf000:
		switch (opcode & 0x00ff)
		{
		case 0x0007: //FX07 VX = delay_timer
			emulator->registers.v[x] = emulator->delay_timer;
			emulator->program_counter += 2;
			break;

		case 0x000a: //FX0A VX = key()   keypress awaited and then stored to VX
		{
			uint8 key_pressed = 0;
			for (int i = 0; i < C8_KEY_COUNT; ++i)
			{
				if (emulator->keys[i] != 0)
				{
					emulator->registers.v[x] = i;
					key_pressed = 1;
				}
			}

			if (key_pressed)
				emulator->program_counter += 2;
			
			break;
		}
		case 0x0015: //FX15 delay_timer = VX
			emulator->delay_timer = emulator->registers.v[x];
			emulator->program_counter += 2;
			break;

		case 0x0018: //FX18 sound_timer = VX
			emulator->sound_timer = emulator->registers.v[x];
			emulator->program_counter += 2;
			break;

		case 0x001e: //FX1E I += VX
			emulator->address_register_i += emulator->registers.v[x];
			emulator->program_counter += 2;
			break;

		case 0x0029: //FX29 I = sprite_address(VX)
			emulator->address_register_i = emulator->registers.v[x] * 0x5;
			emulator->program_counter += 2;
			break;

		case 0x0033: //FX33 bcd = BCD(VX) => I+0 = hundreds   I+1 = tens  I+2 = Ones
			emulator->memory[emulator->address_register_i] = emulator->registers.v[x] / 100;
			emulator->memory[emulator->address_register_i + 1] = (emulator->registers.v[x] / 10) % 10;
			emulator->memory[emulator->address_register_i + 2] = (emulator->registers.v[x] % 100) % 10;
			emulator->program_counter += 2;
			break;

		case 0x0055: //FX55 from V0 to VX dump to I
			memcpy(&emulator->memory[emulator->address_register_i], &emulator->registers.v0, x + 1);
			emulator->address_register_i += (x + 1);
			emulator->program_counter += 2;
			break;

		case 0x0065: //FX65 fill V0 to VX with values from I
			memcpy(&emulator->registers.v0, &emulator->memory[emulator->address_register_i], x + 1);
			emulator->address_register_i += (x + 1);
			emulator->program_counter += 2;
			break;
		}

		break;

	default:
		LOG("unknown opcode: 0x%x4\n", opcode);
		break;
	}

	if (emulator->delay_timer > 0)
		emulator->delay_timer--;

	if (emulator->sound_timer > 0)
	{
		if (emulator->sound_timer == 1)
		{
			LOG("BEEP\n");
		}

		emulator->sound_timer--;
	}
}
