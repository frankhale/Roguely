#pragma once

#include <algorithm>
#include <iostream>
#include <vector>
#include <cstdlib> 
#include <ctime> 

#include "Player.h"

const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 100;

const int VIEW_PORT_WIDTH = 22;
const int VIEW_PORT_HEIGHT = 13;

enum class MovementDirection {
		LEFT,
		RIGHT,
		UP,
		DOWN
};

struct Point
{
		int x, y;
};

struct Entity
{
		Point point;
};

class Game
{
public:
		Game();

		auto Map() const { return map; }
		auto LightMap() const { return light_map; }

		void MovePlayerLeft();
		void MovePlayerRight();
		void MovePlayerDown();
		void MovePlayerUp();

		int GetPlayerHealth() const { return player->GetHealth(); }
		int GetPlayerScore() const { return player->GetScore(); }
		int GetPlayerX() const { return player->X(); }
		int GetPlayerY() const { return player->Y(); }
		int GetViewPortX() const { return view_port_x; }
		int GetViewPortY() const { return view_port_y; }
		int GetViewPortWidth() const { return view_port_width; }
		int GetViewPortHeight() const { return view_port_height; }

		bool IsTileOnMapTraversable(int x, int y, MovementDirection dir, int tileId);
		void UpdatePlayerViewPortPoints(int playerX, int playerY);
		void UpdateAfterPlayerMoved();

		Point GenerateRandomPoint();

		std::vector<Entity> GetCoins() const { return coins; }
		std::vector<Entity> GetHealthGems() const { return health_gems; }
		
		void SpawnEntities(std::vector<Entity> &entity, int num);

		void RB_FOV();		

private:
		std::shared_ptr<Player> player;

		int view_port_x, view_port_y, view_port_width, view_port_height;
		int map[MAP_HEIGHT][MAP_WIDTH];
		int light_map[MAP_HEIGHT][MAP_WIDTH];

		const int COIN_VALUE = 50;
		const int HEALTH_GEM_VALUE = 25;
		const int NUMBER_OF_COINS_ON_MAP = 150;
		const int NUMBER_OF_HEALTH_GEMS_ON_MAP = 50;
		const int NUMBER_OF_ENEMIES_ON_MAP = 50;

		std::vector<Entity> coins;
		std::vector<Entity> health_gems;
};


