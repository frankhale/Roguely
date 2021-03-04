/*
* SpriteSheet.h
*
* MIT License
*
* Copyright (c) 2021 Frank Hale
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>

#include "Common.h"

const int NUM_SPRITES = 25;

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
		WALL_WITH_GRASS_2 = 23,
		MANTIS = 24
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