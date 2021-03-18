/*
* SpriteSheet.cpp
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

#include "SpriteSheet.h"

namespace roguely::sprites
{
		SpriteSheet::SpriteSheet(SDL_Renderer* renderer, std::string n, std::string p, int sw, int sh)
		{
				path = p;
				name = n;
				sprite_width = sw;
				sprite_height = sh;

				auto tileset = IMG_Load(p.c_str());
				auto t_color = SDL_MapRGB(tileset->format, 0, 0, 0);
				SDL_SetColorKey(tileset, SDL_TRUE, t_color);
				spritesheet_texture = SDL_CreateTextureFromSurface(renderer, tileset);
				int total_sprites_on_sheet = tileset->w / sw * tileset->h / sh;
				sprites = std::make_unique<std::vector<std::shared_ptr<SDL_Rect>>>(0);

				for (int y = 0; y < total_sprites_on_sheet / (sh / 2); y++)
				{
						for (int x = 0; x < total_sprites_on_sheet / (sw / 2); x++)
						{
								SDL_Rect rect = { x * sw, y * sh, sw, sh };
								auto r = std::make_shared<SDL_Rect>(rect);
								sprites->emplace_back(r);
						}
				}

				SDL_FreeSurface(tileset);
		}

		void SpriteSheet::draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y)
		{
				draw_sprite(renderer, sprite_id, x, y, 0, 0);
		}

		void SpriteSheet::draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y, int scaled_width, int scaled_height)
		{
				if (sprite_id < 0 || sprite_id > sprites->capacity()) return;

				int width = sprite_width;
				int height = sprite_height;

				if (scaled_width > 0 && scaled_height > 0)
				{
						width = scaled_width;
						height = scaled_height;
				}

				SDL_Rect dest = { x, y, width, height };
				auto sprite_rect = sprites->at(sprite_id);
				SDL_RenderCopy(renderer, spritesheet_texture, &(*sprite_rect), &dest);
		}

		SpriteSheet::~SpriteSheet()
		{
				SDL_DestroyTexture(spritesheet_texture);
		}

		sol::table SpriteSheet::get_sprites_as_lua_table(sol::this_state s)
		{
				sol::state_view lua(s);
				sol::table sprites_table = lua.create_table();

				//std::cout << "getting info for: " << name << std::endl;
				//std::cout << "number of sprites: " << sprites->size() << std::endl;

				for (std::size_t i = 0, sp = sprites->size(); i != sp; ++i)
				{
						auto sprite_rect = sprites->at(i);

						sol::table rect_table = lua.create_table();

						rect_table.set("x", sprite_rect->x);
						rect_table.set("y", sprite_rect->y);
						rect_table.set("w", sprite_rect->w);
						rect_table.set("h", sprite_rect->h);

						sprites_table.set(i, rect_table);

						//std::cout << i << " - " <<
						//		" x = " << sprite_rect->x <<
						//		" y = " << sprite_rect->x <<
						//		" w = " << sprite_rect->w <<
						//		" h = " << sprite_rect->h << std::endl;
				}

				return sprites_table;
		}
}