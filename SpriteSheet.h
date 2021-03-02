#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

#include "Common.h"

const int NUM_SPRITES = 24;

enum Sprites {
		WALL = 0,
		HEALTH_GEM = 1,
		ATTACK_BONUS_GEM = 2,
		PLAYER = 3,
		SPIDER = 4,
		LURCHER = 5,
		GOLDEN_CANDLE = 6,
		DOWN_LADDER = 7,
		UP_LADDER = 8,
		GROUND = 9,
		WATER = 10,
		GRASS = 11,
		CRAB = 12,
		TREASURE_CHEST = 13,
		COIN = 14,
		DOOR = 15,
		WALL_SECRET_DOOR = 16,
		BUG = 17,
		FIRE_WALKER = 18,
		HIDDEN = 19,
		HEART = 20,
		CRIMSON_SHADOW = 21,
		WALL_WITH_GRASS_1 = 22,
		WALL_WITH_GRASS_2 = 23
};

struct MetaSprite
{
		//std::shared_ptr<std::vector<int>> images{};
};

class SpriteSheet
{
public:
		SpriteSheet(SDL_Renderer* renderer, const char* path, int sw, int sh);
		~SpriteSheet();

		void drawSprite(SDL_Renderer* renderer, int sprite_id, int x, int y);
		void drawSprite(SDL_Renderer* renderer, int sprite_id, int x, int y, int scaled_width, int scaled_height);
		int totalSprites() const { return NUM_SPRITES; }

		SDL_Texture* get_spritesheet_texture() const { return spritesheet_texture; }

private:
		int sprite_width, sprite_height;
		SDL_Rect sprites[NUM_SPRITES];
		SDL_Texture* spritesheet_texture;
};