/*
* Game.hpp
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

#include <algorithm>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <cstdlib> 
#include <ctime>

#include "Common.hpp"
#include "Player.hpp"
#include "LevelGeneration.hpp"
#include "Entity.hpp"

enum class AttackType
{
		Critical,
		Normal
};

enum class CombatMultiplier
{
		Plus,
		Minus
};

struct CombatLog
{
		Point point{};
		WhoAmI who{};
		AttackType attack_type{};
		std::string message{};
};

struct TileWalkableInfo
{
		bool walkable = false;
		Point point{};
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

		auto GetPlayerCombatInfo() const { return player_combat_info.str(); }
		auto GetWinLoseMessage() const { return win_lose_message.str(); }
		auto GetEnemyStatInfo() const { return enemy_stats_info.str(); }
		auto GetEnemyCombatInfo() const { return enemy_combat_info.str(); }
		auto GetPlayerEnemiesKilled() const { return player->GetEnemiesKilled(); }
		auto GetPlayerStartingHealth() const { return player->GetStartingHealth(); }
		auto GetPlayerHealth() const { return player->GetHealth(); }
		void SetPlayerHealth(int health) const { player->SetHealth(health); }
		void AddPlayerHealth(int health) { player->SetHealth(player->GetHealth() + health); }
		auto GetPlayerScore() const { return player->GetScore(); }
		auto GetPlayerX() const { return player->X(); }
		auto GetPlayerY() const { return player->Y(); }
		Point GetPlayerPoint() const { return { player->X(), player->Y() }; };
		auto GetViewPortX() const { return view_port_x; }
		auto GetViewPortY() const { return view_port_y; }
		auto GetViewPortWidth() const { return view_port_width; }
		auto GetViewPortHeight() const { return view_port_height; }

		bool IsEntityLocationTraversable(int x, int y, std::shared_ptr<std::vector<std::shared_ptr<Entity>>> entities);
		auto IsEntityLocationTraversable(int x, int y, std::shared_ptr<std::vector<std::shared_ptr<Entity>>> entities, WhoAmI whoAmI, MovementDirection dir);
		bool IsTilePlayerTile(int x, int y, MovementDirection dir);
		bool IsTileWalkable(int x, int y, MovementDirection dir, WhoAmI whoAmI);
		bool IsTileOnMapTraversable(int x, int y, MovementDirection dir, int tileId);
		void UpdatePlayerViewPortPoints(int playerX, int playerY);
		void UpdateAfterPlayerMoved();

		Point GenerateRandomPoint();
		Point GetOpenPointForXY(int x, int y);
		bool IsXYBlocked(int x, int y);

		auto GetCoins() const { return &coins; }
		auto GetHealthGems() const { return &health_gems; }
		auto GetEnemies() const { return &enemies; }
		auto GetTreasureChests() const { return &treasure_chests; }
		auto GetBonus() const { return &bonus; }
		auto GetGoldenCandle() const { return golden_candle; }
		auto GetCombatLog() const { return combat_log; }
		auto GetMap() const { return map; }

		void SpawnEntities(std::shared_ptr<std::vector<std::shared_ptr<Entity>>> entity, int num, EntityType entityType, EntitySubType entitySubType);
		void SpawnEntity(std::shared_ptr<std::vector<std::shared_ptr<Entity>>> entity, EntityType entityType, EntitySubType entitySubType, int x, int y);
		void MoveEnemies();
		void InitiateAttackSequence(int x, int y);
		void RB_FOV();

		void AddCombatLog(WhoAmI who, Point point, AttackType attack_type, CombatMultiplier combat_multiplier, int damage);
		void HandleLogicTimer(double delta_time);

private:
		template<typename T>
		void ClearQueue(std::queue<T>& q)
		{
				std::queue<T> empty;
				std::swap(q, empty);
		}

		void ClearInfo();
		void UpdateCollection(std::shared_ptr<std::vector<std::shared_ptr<Entity>>> entities, std::function<void()>);

		std::shared_ptr<Player> player;

		int view_port_x, view_port_y, view_port_width, view_port_height;

		std::shared_ptr<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>> map;
		std::shared_ptr<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>> light_map;

		const int COIN_VALUE = 50;
		const int HEALTH_GEM_VALUE = 25;
		const int NUMBER_OF_COINS_ON_MAP = 150;
		const int NUMBER_OF_HEALTH_GEMS_ON_MAP = 50;
		const int NUMBER_OF_ENEMIES_ON_MAP = 75;

		std::shared_ptr<std::vector<std::shared_ptr<Entity>>> coins;
		std::shared_ptr<std::vector<std::shared_ptr<Entity>>> health_gems;
		std::shared_ptr<std::vector<std::shared_ptr<Entity>>> enemies;
		std::shared_ptr<std::vector<std::shared_ptr<Entity>>> treasure_chests;
		std::shared_ptr<std::vector<std::shared_ptr<Entity>>> bonus;

		std::shared_ptr<std::queue<std::shared_ptr<CombatLog>>> combat_log;

		std::shared_ptr<Entity> golden_candle;

		std::ostringstream win_lose_message;
		std::ostringstream enemy_stats_info;
		std::ostringstream enemy_combat_info;
		std::ostringstream player_combat_info;
};