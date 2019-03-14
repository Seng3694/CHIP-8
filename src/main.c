#include <SDL.h>

#include "emulator.h"

#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xffffff
#define WINDOW_SCALE 16

c8_emulator_t emulator;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;

void update_graphics(void);

int main(int argc, char** argv)
{
	c8_emulator_init(&emulator);
	if (c8_emulator_load_rom(&emulator, "roms/pong2.c8") == 1)
		return 1;

	if (SDL_Init(SDL_INIT_EVERYTHING))
		return 1;

	window = SDL_CreateWindow(
		"CHIP-8",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		C8_DISPLAY_WIDTH * WINDOW_SCALE,
		C8_DISPLAY_HEIGHT * WINDOW_SCALE,
		SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_SOFTWARE
	);

	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGB888,
		SDL_TEXTUREACCESS_STREAMING,
		C8_DISPLAY_WIDTH,
		C8_DISPLAY_HEIGHT
	);

	uint8 running = 1;
	Uint64 last = 0;
	Uint64 now = SDL_GetPerformanceCounter();
	float delta = 60;

	while (running)
	{
		last = now;
		now = SDL_GetPerformanceCounter();
		delta += (float)((now - last) / (double)SDL_GetPerformanceFrequency());

		if (delta >= 0.0040)
		{
			delta = 0;

			c8_emulator_cycle(&emulator);

			if (emulator.draw_flag == 1)
			{
				update_graphics();
				SDL_RenderCopy(renderer, texture, NULL, NULL);
				SDL_RenderPresent(renderer);
			}

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				switch (e.type)
				{
				case SDL_QUIT:
					running = 0;
					break;

				case SDL_KEYDOWN:
				case SDL_KEYUP:
				{
					const uint8 key_down = e.key.type == SDL_KEYDOWN ? 1 : 0;
					switch (e.key.keysym.sym)
					{
					case SDLK_1: emulator.keys[0] = key_down; break;
					case SDLK_2: emulator.keys[1] = key_down; break;
					case SDLK_3: emulator.keys[2] = key_down; break;
					case SDLK_4: emulator.keys[3] = key_down; break;

					case SDLK_q: emulator.keys[4] = key_down; break;
					case SDLK_w: emulator.keys[5] = key_down; break;
					case SDLK_e: emulator.keys[6] = key_down; break;
					case SDLK_r: emulator.keys[7] = key_down; break;

					case SDLK_a: emulator.keys[8] = key_down; break;
					case SDLK_s: emulator.keys[9] = key_down; break;
					case SDLK_d: emulator.keys[10] = key_down; break;
					case SDLK_f: emulator.keys[11] = key_down; break;

					case SDLK_y: emulator.keys[12] = key_down; break;
					case SDLK_x: emulator.keys[13] = key_down; break;
					case SDLK_c: emulator.keys[14] = key_down; break;
					case SDLK_v: emulator.keys[15] = key_down; break;
					}

					break;
				}
				}
			}
		}
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}

void update_graphics(void)
{
	uint32* pixels = NULL;
	int pitch = 0;

	SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

	for (int y = 0; y < C8_DISPLAY_HEIGHT; ++y)
	{
		for (int x = 0; x < C8_DISPLAY_WIDTH; ++x)
		{
			const uint8 pixel = emulator.pixels[y * C8_DISPLAY_WIDTH + x];
			pixels[y * (pitch / sizeof(uint32)) + x] = pixel == 0 ? COLOR_BLACK : COLOR_WHITE;
		}
	}

	SDL_UnlockTexture(texture);
}