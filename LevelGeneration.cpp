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

#include "LevelGeneration.hpp"

#include <cstdlib>

int get_neighbor_wall_count(std::shared_ptr<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>> map, int x, int y)
{
		int wall_count = 0;

		for (int row = y - 1; row <= y + 1; row++)
		{
				for (int col = x - 1; col <= x + 1; col++)
				{
						if (row >= 1 && col >= 1 && row < MAP_HEIGHT - 1 && col < MAP_WIDTH - 1)
						{
								if ((*map)[row][col] == 0)
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

void perform_cellular_automaton(std::shared_ptr<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>> map, int passes)
{
		for (int p = 0; p < passes; p++)
		{
				auto temp_map = std::make_shared<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>>() = map;

				for (int rows = 0; rows < MAP_HEIGHT; rows++)
				{
						for (int columns = 0; columns < MAP_WIDTH; columns++)
						{
								auto neighbor_wall_count = get_neighbor_wall_count(temp_map, columns, rows);

								if (neighbor_wall_count > 4)
										(*map)[rows][columns] = 0;
								else
										(*map)[rows][columns] = 9;
						}
				}
		}
}

std::shared_ptr<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>> init_cellular_automata()
{
		auto starting_map = std::make_shared<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>>();

		for (int r = 0; r < MAP_HEIGHT; r++)
		{
				for (int c = 0; c < MAP_WIDTH; c++)
				{
						auto z = std::rand() % 100 + 1;
						if (z > 48)
						{
								(*starting_map)[r][c] = 9;
						}
						else
						{
								(*starting_map)[r][c] = 0;
						}
				}
		}

		return starting_map;
}
