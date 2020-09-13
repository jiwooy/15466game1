#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include "load_save_png.hpp"

#include <random>
#include <bitset>
using namespace std;
PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	//Also, *don't* use these tiles in your game:

	// { //use tiles 0-16 as some weird dot pattern thing:
	// 	std::array< uint8_t, 8*8 > distance;
	// 	for (uint32_t y = 0; y < 8; ++y) {
	// 		for (uint32_t x = 0; x < 8; ++x) {
	// 			float d = glm::length(glm::vec2((x + 0.5f) - 4.0f, (y + 0.5f) - 4.0f));
	// 			d /= glm::length(glm::vec2(4.0f, 4.0f));
	// 			distance[x+8*y] = std::max(0,std::min(255,int32_t( 255.0f * d )));
	// 		}
	// 	}
	// 	for (uint32_t index = 0; index < 16; ++index) {
	// 		PPU466::Tile tile;
	// 		uint8_t t = (255 * index) / 16;
	// 		for (uint32_t y = 0; y < 8; ++y) {
	// 			uint8_t bit0 = 0;
	// 			uint8_t bit1 = 0;
	// 			for (uint32_t x = 0; x < 8; ++x) {
	// 				uint8_t d = distance[x+8*y];
	// 				if (d > t) {
	// 					bit0 |= (1 << x);
	// 				} else {
	// 					bit1 |= (1 << x);
	// 				}
	// 			}
	// 			tile.bit0[y] = bit0;
	// 			tile.bit1[y] = bit1;
	// 		}
	// 		ppu.tile_table[index] = tile;
	// 	}
	// }
	
	printf("adding ");
	add_load_function(LoadTagDefault, png_to_tile);
	printf("done adding\n");
	printf("call load");
	call_load_functions();
	printf("done load\n");
	//starting platform
	uint8_t attribute = 0b00000011;
	ppu.sprites[1].x = 120;
	ppu.sprites[1].y = 20;
	ppu.sprites[1].index = 2;
	ppu.sprites[1].attributes = attribute;
	ppu.sprites[2].x = 128;
	ppu.sprites[2].y = 20;
	ppu.sprites[2].index = 2;
	ppu.sprites[2].attributes = attribute;

	int r = 20;
	std::default_random_engine gen;
	std::uniform_int_distribution<int> xcoord(0, 240);
	for (uint8_t i = 0; i < numPlatforms - 1; i++) { //3 - 18
		r += (int) (220 / numPlatforms);
		std::uniform_int_distribution<int> ycoord(r, r + 10);
		int x = xcoord(gen); 
		int y = ycoord(gen);
		uint8_t c1 = (i * 2) + 3;
		uint8_t c2 = c1 + 1;
		ppu.sprites[c1].x = x;
		ppu.sprites[c1].y = y;
		ppu.sprites[c1].index = 2;
		ppu.sprites[c1].attributes = attribute;
		ppu.sprites[c2].x = x + 8;
		ppu.sprites[c2].y = y;
		ppu.sprites[c2].index = 2;
		ppu.sprites[c2].attributes = attribute;
	}
	for (uint8_t i = 19; i < 63; i++) {
		ppu.sprites[i].y = 250;
	}

	//blank sprite
	ppu.palette_table[4] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};
	ppu.tile_table[4].bit0 = {
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
	};
	ppu.tile_table[4].bit1 = {
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
		0b11111111,
	};

	// makes the center of tiles 0-16 solid:
	// ppu.palette_table[1] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	//used for the player:
	// ppu.palette_table[7] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0xff, 0xff, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// };

	//used for the misc other sprites:
	// ppu.palette_table[6] = {
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// 	glm::u8vec4(0x88, 0x88, 0xff, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0xff),
	// 	glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	// };
	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	uint16_t cloud1 = 0b0000010100000000;
	uint16_t cloud2 = 0b0000010100000001;
	uint16_t blank = 0b0000010000000100;
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*y] = blank;
		}
	}
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth / 2; ++x) {
			//TODO: make weird plasma thing
			if (y == 0 || y == 2 || y == 12 || y == 14 || y == 24 || y == 26 ||
				y == 36 || y == 38 || y == 48 || y == 50) {
				if ((x + y) % 4 == 0) {
					int t = (x*2)+PPU466::BackgroundWidth*y;
					ppu.background[t] = cloud1;
					ppu.background[t+1] = cloud2;
				}
			}
		}
	}

}

PlayMode::~PlayMode() {
}

uint8_t PlayMode::platform() {
	float px = player_at.x;
	float py = player_at.y;
	for (uint8_t i = 0; i < numPlatforms; i++) {
		uint8_t c1 = (i * 2) + 1;
		uint8_t c2 = c1 + 1;
		float begin = (float)ppu.sprites[c1].x;
		float end = (float)ppu.sprites[c2].x + 8;
		float ypos = (float)ppu.sprites[c1].y + 8;
		if (std::max(px,begin) <= std::min(px + 8,end)) {
			//printf("in range %f %f ", py, ypos);
			if (py > (ypos - 4.0) && py <= ypos) {
				//printf("on platform");
				player_at.y = ypos;
				yvel = 0.0;
				return c1;
			}

			if ((py + 8) > (ypos - 8) && (py + 8) <= (ypos - 5)) {
				player_at.y = ypos - 16;
				yvel = 0.0;
				return 0;
			}
		}
	}
	return 0;
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update_platforms() {
	int prev1 = 0;
	int prev2 = 240;
	std::default_random_engine gen;
	for (uint8_t i = 0; i < numPlatforms; i++) {
		uint8_t c1 = (i * 2) + 1;
		uint8_t c2 = c1 + 1;
		gen.seed(std::random_device{}());
		std::uniform_int_distribution<int> ycoord(240, 245);
		std::uniform_int_distribution<int> xcoord(prev1, prev2);
		int x = xcoord(gen); 
		int y = ycoord(gen);
		if (ppu.sprites[c1].y <= 0) {
			prev1 = std::max(0, x - 130);
			prev2 = std::min(x + 130, 240);
			ppu.sprites[c1].y = y;
			ppu.sprites[c1].x = x;
			ppu.sprites[c2].y = y;
			ppu.sprites[c2].x = x + 8;
		}
	}
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	uint8_t p = platform();
	if (left.pressed) player_at.x -= PlayerSpeed * elapsed;
	if (right.pressed) player_at.x += PlayerSpeed * elapsed;
	if (down.pressed) player_at.y -= PlayerSpeed * elapsed;
	if (up.pressed && p != 0) {
		yvel = jump;
	}
	if (p == 0) {
		yvel -= 3.0f;
		if (yvel < gravity) yvel = gravity;
	}
	player_at.y += yvel * elapsed;
	for (uint8_t i = 0; i < numPlatforms; i++) {
		uint8_t c1 = (i * 2) + 1;
		uint8_t c2 = c1 + 1;
		if (yvel * elapsed > 0) {
			ppu.sprites[c1].y -= (int)(yvel * elapsed);
			ppu.sprites[c2].y -= (int)(yvel * elapsed);
		}
	}
	if (player_at.y > 180) {
		float diff = player_at.y - 180.0f;
		player_at.y = 180;
		for (uint8_t i = 0; i < numPlatforms; i++) {
			uint8_t c1 = (i * 2) + 1;
			uint8_t c2 = c1 + 1;
			ppu.sprites[c1].y -= (int)(1.3 * diff);
			ppu.sprites[c2].y -= (int)(1.3 * diff);
		}
	}
	update_platforms();
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	ppu.background_color = glm::u8vec4(
		116,
		157,
		255,
		0xff
	);

	//background scroll:
	ppu.background_position.y = int32_t(-0.5f * player_at.y);

	//player sprite:
	ppu.sprites[0].x = int32_t(player_at.x);
	ppu.sprites[0].y = int32_t(player_at.y);
	ppu.sprites[0].index = 32;
	ppu.sprites[0].attributes = 7;

	//some other misc sprites:
	for (uint32_t i = 19; i < 63; ++i) {
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int32_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y = int32_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player_at.y) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].index = 31;
		ppu.sprites[i].attributes = 6;
		if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}
