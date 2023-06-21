#include "engine.h"

int main(int argc, char *argv[])
{
  auto engine = std::make_unique<roguely::engine::Engine>();
  engine->game_loop();

  // boost::numeric::ublas::matrix<int> grid(5, 5);

  // int values[5][5] = {
  //     // 0, 1, 2, 3, 4
  //     {0, 0, 0, 0, 0},  // 0
  //     {0, 1, 1, 1, 1},  // 1
  //     {0, 0, 0, 0, 0},  // 2
  //     {0, 0, 1, 1, 0},  // 3
  //     {0, 0, 1, 0, 0}}; // 4

  // for (size_t i = 0; i < 5; ++i)
  // {
  //   for (size_t j = 0; j < 5; ++j)
  //   {
  //     grid(i, j) = values[i][j];
  //   }
  // }

  // // Define the start and goal coordinates
  // int start_col = 2;
  // int start_row = 2;
  // int goal_col = 4;
  // int goal_row = 4;

  // roguely::map::AStar astar;

  // // Find the shortest path using A*
  // std::vector<std::pair<int, int>> path = astar.FindPath(grid, start_row, start_col, goal_row, goal_col);

  // if (path.size() == 0)
  // {
  //   std::cout << "No path found" << std::endl;
  //   return 0;
  // }

  // // Print the path
  // std::cout << "Shortest path from (" << start_row << ", " << start_col << ") to (" << goal_col << ", " << goal_row << "):\n";
  // for (const auto &point : path)
  // {
  //   std::cout << "(" << point.first << ", " << point.second << ") ";
  // }
  // std::cout << std::endl;
  // fmt::print("Path size = {}", path.size());

  return 0;
}
