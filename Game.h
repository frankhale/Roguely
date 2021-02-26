#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <typeinfo>
#include <vector>
#include <cstdlib> 
#include <ctime> 

#include "Player.h"

const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 100;

const int VIEW_PORT_WIDTH = 22;
const int VIEW_PORT_HEIGHT = 13;

enum class MovementDirection
{
		LEFT,
		RIGHT,
		UP,
		DOWN
};

enum class EntityType
{
		Enemy,
		Pickup
};

enum EnemyType
{
		Spider = 50,
		Lurcher = 51,
		Crab = 52,
		Bug = 53
};

enum class WhoAmI
{
		Player,
		Enemy
};

struct Point
{
		int x, y;
};

class Component {
public:
		virtual ~Component() {};
};
class StatComponent : public Component
{
public:
		StatComponent(int h, int a) { health = h; attack = a; }
		~StatComponent() {};

		int GetHealth() const { return health; }
		int GetAttack() const { return attack; }
		void SetHealth(int h) { health = h; }
		void SetAttack(int a) { attack = a; }

private:
		int health, attack;
};

struct Entity
{
		Point point;
		EntityType entityType;
		std::shared_ptr<std::vector<std::shared_ptr<Component>>> components;
		int id;
};

struct TileWalkableInfo
{
		bool walkable;
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

		auto GetPlayerCombatInfo() const { return player_combat_info.str(); }
		auto GetWinLoseMessage() const { return win_lose_message.str(); }
		auto GetEnemyStatInfo() const { return enemy_stats_info.str(); }
		auto GetEnemyCombatInfo() const { return enemy_combat_info.str(); }		
		int GetPlayerEnemiesKilled() const { return player->GetEnemiesKilled(); }
		int GetPlayerHealth() const { return player->GetHealth(); }
		int GetPlayerScore() const { return player->GetScore(); }
		int GetPlayerX() const { return player->X(); }
		int GetPlayerY() const { return player->Y(); }
		int GetViewPortX() const { return view_port_x; }
		int GetViewPortY() const { return view_port_y; }
		int GetViewPortWidth() const { return view_port_width; }
		int GetViewPortHeight() const { return view_port_height; }

		bool IsEntityLocationTraversable(int x, int y, std::shared_ptr<std::vector<Entity>> entities);
		auto IsEntityLocationTraversable(int x, int y, std::shared_ptr<std::vector<Entity>> entities, WhoAmI whoAmI, MovementDirection dir);
		bool IsTilePlayerTile(int x, int y, MovementDirection dir);
		bool IsTileWalkable(int x, int y, MovementDirection dir, WhoAmI whoAmI);
		bool IsTileOnMapTraversable(int x, int y, MovementDirection dir, int tileId);
		void UpdatePlayerViewPortPoints(int playerX, int playerY);
		void UpdateAfterPlayerMoved();

		Point GenerateRandomPoint();

		auto GetCoins() const { return &coins; }
		auto GetHealthGems() const { return &health_gems; }
		auto GetEnemies() const { return &enemies; }
		auto GetTreasureChests() const { return &treasure_chests; }
		Entity GetGoldenCandle() const { return golden_candle; }

		void SpawnEntities(std::shared_ptr<std::vector<Entity>> entity, int num, EntityType entityType, std::shared_ptr<std::vector<std::shared_ptr<Component>>> components, int id);

		void MoveEnemies();
		void InitiateAttackSequence(int x, int y);

		void RB_FOV();

private:
		void ClearInfo();
		void UpdateCollection(std::shared_ptr < std::vector<Entity>> entities, std::function<void()>);

		std::shared_ptr<Player> player;

		int view_port_x, view_port_y, view_port_width, view_port_height;
		int map[MAP_HEIGHT][MAP_WIDTH];
		int light_map[MAP_HEIGHT][MAP_WIDTH];

		const int COIN_VALUE = 50;
		const int HEALTH_GEM_VALUE = 25;
		const int NUMBER_OF_COINS_ON_MAP = 150;
		const int NUMBER_OF_HEALTH_GEMS_ON_MAP = 50;
		const int NUMBER_OF_ENEMIES_ON_MAP = 50;

		std::shared_ptr<std::vector<Entity>> coins;
		std::shared_ptr<std::vector<Entity>> health_gems;
		std::shared_ptr<std::vector<Entity>> enemies;
		std::shared_ptr<std::vector<Entity>> treasure_chests;
		Entity golden_candle;

		std::ostringstream win_lose_message;
		std::ostringstream enemy_stats_info;
		std::ostringstream enemy_combat_info;		
		std::ostringstream player_combat_info;
};