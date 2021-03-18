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

#include "Common.h"

namespace roguely::sprites
{
		class SpriteSheet
		{
		public:
				SpriteSheet(SDL_Renderer* renderer, std::string n, std::string p, int sw, int sh);
				~SpriteSheet();

				void draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y);
				void draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y, int scaled_width, int scaled_height);				

				SDL_Texture* get_spritesheet_texture() const { return spritesheet_texture; }

				auto get_name() const { return name; }

				sol::table get_sprites_as_lua_table(sol::this_state s);

		private:
				std::string name;
				std::string path;
				int sprite_width = 0;
				int sprite_height = 0;
				std::unique_ptr<std::vector<std::shared_ptr<SDL_Rect>>> sprites = nullptr;
				SDL_Texture* spritesheet_texture = nullptr;
		};
}