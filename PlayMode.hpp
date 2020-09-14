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
	virtual uint8_t platform();
	virtual void update_platforms(float diff, float elapsed);
	virtual void update_corona(float diff, float elapsed);
	virtual bool check_damage();
	void update_health(float move, bool damaged);

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;

	float yvel = 0.0f;
	float gravity = -100.0f;
	float jump = 110.0f;
	float PlayerSpeed = 100.0f;

	float total_elapsed = 0.0f;
	float print_timer = 5.0f;

	uint8_t numPlatforms = 14;
	uint8_t otherSprites = 1 + (2 * numPlatforms);

	float health = 100.0f;
	float altitude = 0.0f;

	int prev1 = 0;
	int prev2 = 240;

	//player position:
	glm::vec2 player_at = glm::vec2(120.0f, 30.0f);

	//----- drawing handled by PPU466 -----

	PPU466 ppu;

	void load(uint8_t palette, uint8_t tile, std::string f);

	std::function< void() > png_to_tile = [this]() {
		load(7, 32, "assets\\player.png");
		load(6, 31, "assets\\corona.png");
		load(5, 0, "assets\\cloud1.png");
		load(5, 1, "assets\\cloud2.png");
		load(3, 2, "assets\\ground.png");
	};
};
