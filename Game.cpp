#include "Game.h"

Game::Game()
{
		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		view_port_x = 0;
		view_port_y = 0;
		view_port_width = VIEW_PORT_WIDTH;
		view_port_height = VIEW_PORT_HEIGHT;

		map[MAP_HEIGHT - 1][MAP_WIDTH - 1] = {};
		light_map[MAP_HEIGHT - 1][MAP_WIDTH - 1] = {};

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
								if ((std::rand() % 100) <= 5 && height != player->Y() && width != player->X())
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

		coins = std::make_shared<std::vector<Entity>>();
		health_gems = std::make_shared<std::vector<Entity>>();

		SpawnEntities(coins, NUMBER_OF_COINS_ON_MAP, EntityType::Pickup, 1);
		SpawnEntities(health_gems, NUMBER_OF_HEALTH_GEMS_ON_MAP, EntityType::Pickup, 2);

		golden_candle.entityType = EntityType::Pickup;
		golden_candle.id = 100;
		golden_candle.point = GenerateRandomPoint();

		RB_FOV();
}

bool Game::IsEntityLocationTraversable(int x, int y, std::vector<Entity>& entities, WhoAmI whoAmI, MovementDirection dir)
{
		for (auto& e : entities)
		{
				if ((dir == MovementDirection::UP && e.point.y == y - 1 && e.point.x == x) ||
						(dir == MovementDirection::DOWN && e.point.y == y + 1 && e.point.x == x) ||
						(dir == MovementDirection::LEFT && e.point.y == y && e.point.x == x - 1) ||
						(dir == MovementDirection::RIGHT && e.point.y == y && e.point.x == x + 1))
				{
						return true;
				}
		}

		return false;
}

bool Game::IsTilePlayerTile(int x, int y, MovementDirection dir)
{
		return ((dir == MovementDirection::UP && player->Y() == y - 1 && player->X() == x) ||
				(dir == MovementDirection::DOWN && player->Y() == y + 1 && player->X() == x) ||
				(dir == MovementDirection::LEFT && player->Y() == y && player->X() == x - 1) ||
				(dir == MovementDirection::RIGHT && player->Y() == y && player->X() == x + 1));
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
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerRight()
{
		if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::RIGHT, 0)) return;

		player->SetX(player->X() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerDown()
{
		if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::DOWN, 0)) return;

		player->SetY(player->Y() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerUp()
{
		if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::UP, 0)) return;

		player->SetY(player->Y() - 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
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

void Game::UpdateAfterPlayerMoved()
{
		coins->erase(std::remove_if(coins->begin(), coins->end(),
				[&](Entity e) {
						if (e.point.x == player->X() && e.point.y == player->Y())
						{
								player->SetScore(player->GetScore() + COIN_VALUE);
								return true;
						}
						return false;
				}), coins->end());

		health_gems->erase(std::remove_if(health_gems->begin(), health_gems->end(),
				[&](Entity e) {
						if (e.point.x == player->X() && e.point.y == player->Y())
						{
								player->SetHealth(player->GetHealth() + HEALTH_GEM_VALUE);
								return true;
						}
						return false;
				}), health_gems->end());

		RB_FOV();
}

Point Game::GenerateRandomPoint()
{
		int r = 0;
		int c = 0;

		do
		{
				r = rand() % (MAP_HEIGHT - 1);
				c = rand() % (MAP_WIDTH - 1);
		} 		while (map[r][c] == 0 || player->X() == c && player->Y() == r);

		return { c, r };
}

void Game::SpawnEntities(std::shared_ptr<std::vector<Entity>> entity, int num, EntityType entityType, int id)
{
		for (int i = 0; i < num; i++)
		{
				Entity e;
				e.point = GenerateRandomPoint();
				e.entityType = entityType;
				e.id = id;

				entity->push_back(e);
		}
}

void Game::RB_FOV()
{
		float x = 0, y = 0;

		for (int r = 0; r < MAP_HEIGHT; r++)
		{
				for (int c = 0; c < MAP_WIDTH; c++)
				{
						light_map[r][c] = 0;
				}
		}

		for (int i = 0; i < 360; i++)
		{
				x = (float)std::cos(i * 0.01745f);
				y = (float)std::sin(i * 0.01745f);

				float ox = (float)player->X() + 0.5f;
				float oy = (float)player->Y() + 0.5f;

				for (int j = 0; j < 20; j++)
				{
						light_map[(int)oy][(int)ox] = 2;

						if (map[(int)oy][(int)ox] == 0) // if tile is a wall
								break;

						ox += x;
						oy += y;
				};
		};
}