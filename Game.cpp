#include "Game.h"

Game::Game()
{
		srand(time(NULL));

		view_port_x = 0;
		view_port_y = 0;
		view_port_width = VIEW_PORT_WIDTH;
		view_port_height = VIEW_PORT_HEIGHT;

		map[MAP_HEIGHT - 1][MAP_WIDTH - 1] = {};

		player = std::make_shared<Player>(30, 1);

		UpdatePlayerViewPortPoints(player->X(), player->Y());

		for (int height = 0; height < MAP_HEIGHT; height++)
		{
				for (int width = 0; width < MAP_WIDTH; width++)
				{
						if (height == 0 || width == 0 || height == MAP_HEIGHT - 1 || width == MAP_WIDTH - 1)
						{
								map[height][width] = 0;
						}
						else
						{
								if ((rand() % 100) <= 5 && height != player->Y() && width != player->X())
								{
										map[height][width] = 0;
								}
								else
								{
										map[height][width] = 9;
								}
						}
				}
		}
}

bool Game::IsTileOnMapTraversable(int x, int y, MovementDirection dir, int tileId)
{
		return !(dir == MovementDirection::UP && map[y - 1][x] == tileId ||
				dir == MovementDirection::DOWN && map[y + 1][x] == tileId ||
				dir == MovementDirection::LEFT && map[y][x - 1] == tileId ||
				dir == MovementDirection::RIGHT && map[y][x + 1] == tileId);
}

void Game::MovePlayerLeft()
{
		if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::LEFT, 0)) return;

		player->SetX(player->X() - 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
};

void Game::MovePlayerRight()
{
		if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::RIGHT, 0)) return;

		player->SetX(player->X() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
};

void Game::MovePlayerDown()
{
		if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::DOWN, 0)) return;

		player->SetY(player->Y() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
};

void Game::MovePlayerUp()
{
		if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::UP, 0)) return;

		player->SetY(player->Y() - 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
};

void Game::UpdatePlayerViewPortPoints(int playerX, int playerY)
{
		view_port_x = player->X() - VIEW_PORT_WIDTH;
		view_port_y = player->Y() - VIEW_PORT_HEIGHT;

		if (view_port_x < 0) view_port_x = std::max(0, view_port_x);
		if (view_port_x > (MAP_WIDTH - (VIEW_PORT_WIDTH * 2))) view_port_x = (MAP_WIDTH - (VIEW_PORT_WIDTH * 2));

		if (view_port_y < 0) view_port_y = std::max(0, view_port_y);
		if (view_port_y > (MAP_HEIGHT - (VIEW_PORT_HEIGHT * 2))) view_port_y = (MAP_HEIGHT - (VIEW_PORT_HEIGHT * 2));

		view_port_width = view_port_x + (VIEW_PORT_WIDTH * 2);
		view_port_height = view_port_y + (VIEW_PORT_HEIGHT * 2);
}