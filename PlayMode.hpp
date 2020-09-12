#pragma once

#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include "load_save_png.hpp"
#include "Load.hpp"

#include <functional>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;

	//player position:
	glm::vec2 player_at = glm::vec2(0.0f);

	//----- drawing handled by PPU466 -----

	PPU466 ppu;


	void load(uint8_t palette, uint8_t tile, std::string f) {
		glm::uvec2 size = glm::ivec2(8,8);
		std::vector< glm::u8vec4 > picdata(8*8);
		load_png(f, &size, &picdata, LowerLeftOrigin);
		printf("Print stuff \n");
		ppu.palette_table[palette] = {
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		};
		uint8_t colors = 0;
		for (int i = 0; i < picdata.size(); i++) { // get colors for palette
			if (picdata[i].a != 0 && colors < 4) {
				bool notin = true;
				for (int j = 0; j < colors; j++) {
					if (picdata[i].r == ppu.palette_table[palette][j].r &&
						picdata[i].g == ppu.palette_table[palette][j].g &&
						picdata[i].b == ppu.palette_table[palette][j].b &&
						picdata[i].a == ppu.palette_table[palette][j].a) {
						notin = false;
					}
				}
				if (notin) {
					ppu.palette_table[palette][colors] = glm::u8vec4(picdata[i].r, picdata[i].g,
															picdata[i].b, picdata[i].a);
					colors += 1;
				}
			}
		}
		printf("colors %d\n", colors);
		for (int i = 0; i < colors; i ++) {
			printf("%d %d %d %d \n", ppu.palette_table[7][i].r,  ppu.palette_table[7][i].g,
			ppu.palette_table[7][i].b, ppu.palette_table[7][i].a);
		}

		for (int i = 0; i < picdata.size() / 8; i++) {
			uint8_t row0 = 0b11111111;
			uint8_t row1 = 0b11111111;
			uint8_t bit = 0;
			for (int j = 0; j < 8; j++) {
				printf("%d %d %d %d \n", picdata[(i * 8) + j].r,  picdata[(i * 8) + j].g,
				picdata[(i * 8) + j].b, picdata[(i * 8) + j].a);
				uint8_t xor = 1 << bit;
				if (picdata[(i * 8) + j].r == ppu.palette_table[palette][0].r &&
					picdata[(i * 8) + j].g == ppu.palette_table[palette][0].g &&
					picdata[(i * 8) + j].b == ppu.palette_table[palette][0].b &&
					picdata[(i * 8) + j].a == ppu.palette_table[palette][0].a) {
					row0 ^= xor;
					row1 ^= xor;
				} else if (picdata[(i * 8) + j].r == ppu.palette_table[palette][1].r &&
					picdata[(i * 8) + j].g == ppu.palette_table[palette][1].g &&
					picdata[(i * 8) + j].b == ppu.palette_table[palette][1].b &&
					picdata[(i * 8) + j].a == ppu.palette_table[palette][1].a) {
					row1 ^= xor;
				} else if (picdata[(i * 8) + j].r == ppu.palette_table[palette][2].r &&
					picdata[(i * 8) + j].g == ppu.palette_table[palette][2].g &&
					picdata[(i * 8) + j].b == ppu.palette_table[palette][2].b &&
					picdata[(i * 8) + j].a == ppu.palette_table[palette][2].a) {
					row0 ^= xor;
				} else if (picdata[(i * 8) + j].r == ppu.palette_table[palette][3].r &&
					picdata[(i * 8) + j].g == ppu.palette_table[palette][3].g &&
					picdata[(i * 8) + j].b == ppu.palette_table[palette][3].b &&
					picdata[(i * 8) + j].a == ppu.palette_table[palette][3].a) {
					// do nothing
				}
				bit++;
			}
			ppu.tile_table[tile].bit0[i] = row0;
			ppu.tile_table[tile].bit1[i] = row1;
		}
	}

	std::function< void() > png_to_tile = [this]() {
		load(7, 32, "assets/player.png");
		load(6, 31, "assets/corona.png");
		load(5, 0, "assets/cloud1.png");
		load(5, 1, "assets/cloud2.png");
	};
};
