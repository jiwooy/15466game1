#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>
#include "load_save_png.hpp"
#include "data_path.hpp"

#include <random>
#include <bitset>
#include <thread>        
using namespace std;
PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.


	add_load_function(LoadTagDefault, png_to_tile);
	call_load_functions();
	//starting platform
	uint8_t attribute = 0b00000011;
	ppu.sprites[1].x = 120;
	ppu.sprites[1].y = 10;
	ppu.sprites[1].index = 2;
	ppu.sprites[1].attributes = attribute;
	ppu.sprites[2].x = 128;
	ppu.sprites[2].y = 10;
	ppu.sprites[2].index = 2;
	ppu.sprites[2].attributes = attribute;

	int r = 10;
	std::default_random_engine gen;
	std::uniform_int_distribution<int> xcoord(0, 240);
	for (uint8_t i = 0; i < numPlatforms - 1; i++) { //3 - 18
		r += (int) (230 / numPlatforms);
		std::uniform_int_distribution<int> ycoord(r, r + 2);
		gen.seed(std::random_device{}());
		std::uniform_int_distribution<int> xcoord(prev1, prev2);
		int x = xcoord(gen);
		int y = ycoord(gen);
		prevx = x;
		uint8_t c1 = (i * 2) + 3;
		uint8_t c2 = c1 + 1;
		prev1 = std::max(0, x - 60);
		prev2 = std::min(x + 60, 240);
		ppu.sprites[c1].x = x;
		ppu.sprites[c1].y = r;
		ppu.sprites[c1].index = 2;
		ppu.sprites[c1].attributes = attribute;
		ppu.sprites[c2].x = x + 8;
		ppu.sprites[c2].y = r;
		ppu.sprites[c2].index = 2;
		ppu.sprites[c2].attributes = attribute;
	}
	for (uint8_t i = otherSprites; i < 63; i++) {
		ppu.sprites[i].y = 250;
	}

	for (uint32_t i = otherSprites; i < 63; ++i) {
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int32_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y = int32_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f * M_PI * amt * 3.0f + 0.01f * player_at.y) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].index = 31;
		ppu.sprites[i].attributes = 6;
		if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
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

void PlayMode::update_platforms(float diff, float elapsed) {
	for (uint8_t i = 0; i < numPlatforms; i++) {
		uint8_t c1 = (i * 2) + 1;
		uint8_t c2 = c1 + 1;
		ppu.sprites[c1].y -= (int)(yvel * elapsed);
		ppu.sprites[c2].y -= (int)(yvel * elapsed);
		ppu.sprites[c1].y -= (int)(1.3 * diff);
		ppu.sprites[c2].y -= (int)(1.3 * diff);
	}

	std::default_random_engine gen;
	uint8_t update_count = 0;
	for (uint8_t i = 0; i < numPlatforms; i++) {
		uint8_t c1 = (i * 2) + 1;
		uint8_t c2 = c1 + 1;
		gen.seed(std::random_device{}());
		std::uniform_int_distribution<int> ycoord(240, 242);
		std::uniform_int_distribution<int> xcoord(prev1, prev2);
		int x = xcoord(gen);
		int y = ycoord(gen);
		if (ppu.sprites[c1].y <= 0) {
			//printf("%d %d %d\n", prev1, prev2, x);
			prev1 = std::max(0, x - 30);
			prev2 = std::min(x + 30, 240);
			ppu.sprites[c1].y = y + update_count * 20;
			ppu.sprites[c1].x = x;
			ppu.sprites[c2].y = y + update_count * 20;
			ppu.sprites[c2].x = x + 8;
			update_count++;
		}
	}
}

void PlayMode::update_corona(float diff, float elapsed) {
	std::default_random_engine gen;
	std::uniform_int_distribution<int> movey(-1, 1);
	for (uint32_t i = otherSprites; i < 63; ++i) {
		gen.seed(std::random_device{}());
		float amt = (i + 2.0f * background_fade) / 62.0f;
		ppu.sprites[i].x = int32_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f * M_PI * amt * 5.0f) * 0.4f * PPU466::ScreenWidth);
		ppu.sprites[i].y += movey(gen);
	}

	for (uint8_t i = otherSprites; i < 63; i++) {
		if (yvel * elapsed > 0) {
			ppu.sprites[i].y -= (int)(yvel * elapsed);
		}
		ppu.sprites[i].y -= (int)(1.3 * diff);
	}
	for (uint8_t i = otherSprites; i < 63; i++) {
		gen.seed(std::random_device{}());
		std::uniform_int_distribution<int> xcoord(0, 240);
		int x = xcoord(gen);
		if (ppu.sprites[i].y <= 0) {
			ppu.sprites[i].y = 240;
			ppu.sprites[i].x = x;
		}
	}
}

bool PlayMode::check_damage() {
	uint8_t px = (uint8_t)player_at.x;
	uint8_t py = (uint8_t)player_at.y;
	for (uint8_t i = otherSprites; i < 63; i++) {
		uint8_t coronax = ppu.sprites[i].x;
		uint8_t coronay = ppu.sprites[i].y;
		if (std::max(px, coronax) <= std::min(px + 8, coronax + 8) &&
			std::max(py, coronay) <= std::min(py + 8, coronay + 8)) {
				return true;
		}
	}
	return false;
}

void PlayMode::update_health(float move, bool damaged) {
	if (move > 0.0) {
		float cur_health = health;
		health = std::min(100.0f, (float)(cur_health + (move * 0.5)));
	}
	if (damaged) {
		health -= 1.0f;
		if (health <= 0) {
			printf("You caught the virus! Final altitude: %f\nGame Over!\n", altitude);
			std::this_thread::sleep_for(std::chrono::seconds(200));
		}
	}
	if (total_elapsed > print_timer) {
		print_timer += print_timer;
		printf("Virus resistance at %f!\n", health);
	}
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	// (will be used to set background color)
	background_fade += elapsed / 10.0f;
	background_fade -= std::floor(background_fade);

	total_elapsed += elapsed;

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
	float diff = 0;
	float move = yvel * elapsed;
	player_at.y += move;
	altitude = std::max(altitude + move, 0.0f);
	if (player_at.y <= -8.0f) {
		printf("You fell! Final altitude: %f\nGame Over!\n", altitude);
		std::this_thread::sleep_for(std::chrono::seconds(200));
	}

	if (player_at.y > 180) {
		diff = player_at.y - 180.0f;
		player_at.y = 180;
	}
	bool damaged = check_damage();
	update_health(move, damaged);

	update_platforms(diff, elapsed);
	update_corona(diff, elapsed);
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	//background color will be some hsv-like fade:
	//printf("%d\n", std::min(40, (int)(116 - (altitude * 0.01))));
	ppu.background_color = glm::u8vec4(
		std::max(40, (int)(116 - (altitude * 0.1))),
		std::max(24, (int)(157 - (altitude * 0.1))),
		std::max(91, (int)(255 - (altitude * 0.1))),
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
	//--- actually draw ---
	ppu.draw(drawable_size);
}

void PlayMode::load(uint8_t palette, uint8_t tile, std::string f) {
	std::string full_string = data_path("");
	full_string.erase(full_string.length() - 5);
	full_string += f;
	glm::uvec2 size = glm::ivec2(8,8);
	std::vector< glm::u8vec4 > picdata(8*8);
	load_png(full_string, &size, &picdata, LowerLeftOrigin);
	ppu.palette_table[palette] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};
	uint8_t colors = 0;
	for (uint8_t i = 0; i < picdata.size(); i++) { // get colors for palette
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

	for (uint8_t i = 0; i < picdata.size() / 8; i++) {
		uint8_t row0 = 0b11111111;
		uint8_t row1 = 0b11111111;
		uint8_t bit = 0;
		for (int j = 0; j < 8; j++) {
			//printf("%d %d %d %d \n", picdata[(i * 8) + j].r,  picdata[(i * 8) + j].g,
			//picdata[(i * 8) + j].b, picdata[(i * 8) + j].a);
			uint8_t oper = 1 << bit;
			if (picdata[(i * 8) + j].r == ppu.palette_table[palette][0].r &&
				picdata[(i * 8) + j].g == ppu.palette_table[palette][0].g &&
				picdata[(i * 8) + j].b == ppu.palette_table[palette][0].b &&
				picdata[(i * 8) + j].a == ppu.palette_table[palette][0].a) {
				row0 ^= oper;
				row1 ^= oper;
			} else if (picdata[(i * 8) + j].r == ppu.palette_table[palette][1].r &&
				picdata[(i * 8) + j].g == ppu.palette_table[palette][1].g &&
				picdata[(i * 8) + j].b == ppu.palette_table[palette][1].b &&
				picdata[(i * 8) + j].a == ppu.palette_table[palette][1].a) {
				row1 ^= oper;
			} else if (picdata[(i * 8) + j].r == ppu.palette_table[palette][2].r &&
				picdata[(i * 8) + j].g == ppu.palette_table[palette][2].g &&
				picdata[(i * 8) + j].b == ppu.palette_table[palette][2].b &&
				picdata[(i * 8) + j].a == ppu.palette_table[palette][2].a) {
				row0 ^= oper;
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
