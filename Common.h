/*
* Common.h
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

#define SOL_ALL_SAFETIES_ON 1

#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>
#include <mpg123.h>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <sol/sol.hpp>
#include <vector>

namespace roguely::common
{
		enum class MovementDirection
		{
				Left,
				Right,
				Up,
				Down
		};

		enum class WhoAmI
		{
				Player,
				Enemy
		};

		struct Point
		{
				int x = 0;
				int y = 0;
		};

		struct Sound
		{
				std::string name;
				Mix_Chunk* sound;

		public:
				void play() { Mix_PlayChannel(-1, sound, 0); }
		};

		struct Map
		{
		public:
				Map(std::string n, int w, int h, std::shared_ptr<boost::numeric::ublas::matrix<int>> m)
				{
						name = n;
						map = m;
						width = w;
						height = h;
						light_map = std::make_shared<boost::numeric::ublas::matrix<int>>(h, w);
				}

				std::string name{};
				int width = 0;
				int height = 0;
				std::shared_ptr<boost::numeric::ublas::matrix<int>> map{};
				std::shared_ptr<boost::numeric::ublas::matrix<int>> light_map{};
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
}