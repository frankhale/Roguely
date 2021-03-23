/*
* Text.cpp
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

#include "Text.h"

namespace roguely::common
{
		Text::Text()
		{
				font = nullptr;
				text_texture = nullptr;
		}

		int Text::load_font(const char* path, int ptsize)
		{
				font = TTF_OpenFont(path, ptsize);

				if (!font) {
						std::cout << "Unable to load font: " << path << std::endl
								<< "SDL2_ttf Error: " << TTF_GetError() << std::endl;
						return -1;
				}

				return 0;
		}

		TextExtents Text::get_text_extents(const char* text)
		{
				int w{}, h{};

				if (TTF_SizeText(font, text, &w, &h) == 0) {
						return { w, h };
				}

				return {};
		}

		void Text::draw_text(SDL_Renderer* renderer, int x, int y, const char* text)
		{
				draw_text(renderer, x, y, text, text_color);
		}

		void Text::draw_text(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color)
		{
				if (strlen(text) <= 0) return;

				text_texture = nullptr;

				SDL_Rect text_rect;
				SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, color);
				text_rect.x = x;
				text_rect.y = y;
				text_rect.w = text_surface->w;
				text_rect.h = text_surface->h;

				text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
				SDL_FreeSurface(text_surface);

				SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
				SDL_DestroyTexture(text_texture);
		}
}