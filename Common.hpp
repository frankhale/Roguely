/*
* Common.hpp
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

#include <SDL.h>

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 768;
const int MAP_WIDTH = 125;
const int MAP_HEIGHT = 125;
const int VIEW_PORT_WIDTH = 20;
const int VIEW_PORT_HEIGHT = 12;
const int SPRITE_WIDTH = 32;
const int SPRITE_HEIGHT = 32;
const bool MUSIC = false;

struct Point
{
		int x = 0;
		int y = 0;
};

// ref: https://gamedev.stackexchange.com/a/163508/18014
struct Timer
{
		Uint64 previous_ticks{};
		float elapsed_seconds{};

		void tick()
		{
				const Uint64 current_ticks{ SDL_GetPerformanceCounter() };
				const Uint64 delta{ current_ticks - previous_ticks };
				previous_ticks = current_ticks;
				static const Uint64 TICKS_PER_SECOND{ SDL_GetPerformanceFrequency() };
				elapsed_seconds = delta / static_cast<float>(TICKS_PER_SECOND);
		}
};
