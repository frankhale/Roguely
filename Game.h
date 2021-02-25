#pragma once

#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <time.h>
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

class Game
{
public:
		Game();

		const int COIN_VALUE = 50;
		const int POTION_VALUE = 25;
		const int NUMBER_OF_COINS_ON_MAP = 50;
		const int NUMBER_OF_POTIONS_ON_MAP = 10;
		const int NUMBER_OF_ENEMIES_ON_MAP = 25;

		auto Map() const { return map; }

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

private:
		std::shared_ptr<Player> player;

		int view_port_x, view_port_y, view_port_width, view_port_height;

		int map[MAP_HEIGHT][MAP_WIDTH];
};


