/*
* LevelGeneration.cpp
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

#include "LevelGeneration.h"
#include <cstdlib>

namespace roguely::level_generation
{
		int get_random_ground_tile()
		{
				auto y = std::rand() % 100 + 1;

				int ground_tile = 0;

				if (y <= 60) {
						ground_tile = 0;
				}
				else if (y >= 61 && y <= 80)
				{
						ground_tile = 35;
				}
				else if (y >= 81 && y <= 100)
				{
						ground_tile = 36;
				}

				return ground_tile;
		}

		int get_neighbor_wall_count(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int x, int y)
		{
				int wall_count = 0;

				for (int row = y - 1; row <= y + 1; row++)
				{
						for (int col = x - 1; col <= x + 1; col++)
						{
								if (row >= 1 && col >= 1 && row < map_height - 1 && col < map_width - 1)
								{
										if ((*map)(row,col) == 0 ||
												(*map)(row, col) == 35 ||
												(*map)(row, col) == 36)
												wall_count++;
								}
								else
								{
										wall_count++;
								}
						}
				}

				return wall_count;
		}

		void perform_cellular_automaton(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int passes)
		{
				for (int p = 0; p < passes; p++)
				{
						auto temp_map = std::make_shared<boost::numeric::ublas::matrix<int>>() = map;

						for (int rows = 0; rows < map_height; rows++)
						{
								for (int columns = 0; columns < map_width; columns++)
								{
										auto neighbor_wall_count = get_neighbor_wall_count(temp_map, map_width, map_height, columns, rows);

										if (neighbor_wall_count > 4)
												(*map)(rows, columns) = get_random_ground_tile();
										else
												(*map)(rows,columns) = 9;
								}
						}
				}
		}

		std::shared_ptr<boost::numeric::ublas::matrix<int>> init_cellular_automata(int map_width, int map_height)
		{
				auto map = std::make_shared<boost::numeric::ublas::matrix<int>>(map_height, map_width);

				for (int r = 0; r < map_height; ++r)
				{
						for (int c = 0; c < map_width; ++c)
						{								
								auto z = std::rand() % 100 + 1;
								if (z > 48)
										(*map)(r, c) = 9;
								else
										(*map)(r, c) = get_random_ground_tile();
						}
				}

				return map;
		}
}