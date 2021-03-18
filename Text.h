/*
* Text.h
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

namespace roguely::common
{
		struct TextExtents
		{
				int width, height;
		};

		class Text
		{
		public:
				Text();

				int load_font(const char* path, int ptsize);
				void draw_text(SDL_Renderer* renderer, int x, int y, const char* text);
				void draw_text(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color);
				TextExtents get_text_extents(const char* text);

		private:
				TTF_Font* font;
				SDL_Texture* text_texture;

				SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };
				SDL_Color text_background_color = { 0x00, 0x00, 0x00, 0xFF };
		};
}