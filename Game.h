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

#include "Common.h"
#include "Player.h"

const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 100;

const int VIEW_PORT_WIDTH = 22;
const int VIEW_PORT_HEIGHT = 13;

enum class MovementDirection
{
		Left,
		Right,
		Up,
		Down
};

enum class EntityType
{
		Enemy,
		Pickup
};

enum class EntitySubType
{
		Coin = 1,
		Health_Gem = 2,
		Attack_Gem = 3,
		Spider = 50,
		Lurcher = 51,
		Crab = 52,
		Bug = 53,
		Fire_Walker = 54
};

enum class WhoAmI
{
		Player,
		Enemy
};

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

class Component {
public:
		virtual ~Component() {};
};
class StatComponent : public Component
{
public:
		StatComponent(int h, int a) { starting_health = h; health = h; attack = a; }

		int GetStartingHealth() const { return starting_health; }
		int GetHealth() const { return health; }
		int GetAttack() const { return attack; }
		void SetHealth(int h) { health = h; }
		void SetAttack(int a) { attack = a; }

private:
		int starting_health, health, attack;
};

class EntitySubTypeComponent : public Component
{
public:
		EntitySubTypeComponent(EntitySubType est) { entitySubType = est; };
		auto GetEntitySubType() const { return entitySubType; }

private:
		EntitySubType entitySubType;
};

struct Entity
{
		Point point{};
		EntityType entityType{};
		std::shared_ptr<std::vector<std::shared_ptr<Component>>> components = nullptr;
		int id = 0;
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
		int GetPlayerEnemiesKilled() const { return player->GetEnemiesKilled(); }
		int GetPlayerStartingHealth() const { return player->GetPlayerStartingHealth(); }
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
		Point GetOpenPointForXY(int x, int y);
		bool IsXYBlocked(int x, int y);

		auto GetCoins() const { return &coins; }
		auto GetHealthGems() const { return &health_gems; }
		auto GetEnemies() const { return &enemies; }
		auto GetTreasureChests() const { return &treasure_chests; }
		auto GetBonus() const { return &bonus; }
		Entity GetGoldenCandle() const { return golden_candle; }
		auto GetCombatLog() const { return combat_log; }

		void SpawnEntities(std::shared_ptr<std::vector<Entity>> entity, int num, EntityType entityType, EntitySubType entitySubType);
		void SpawnEntity(std::shared_ptr<std::vector<Entity>> entity, EntityType entityType, EntitySubType entitySubType, int x, int y);
		void MoveEnemies();
		void InitiateAttackSequence(int x, int y);
		void RB_FOV();

		template<typename T>
		std::shared_ptr<T> find_component(std::shared_ptr<std::vector<std::shared_ptr<Component>>> components);

		void AddCombatLog(WhoAmI who, Point point, AttackType attack_type, CombatMultiplier combat_multiplier, int damage);

private:
		void ClearInfo();
		void UpdateCollection(std::shared_ptr<std::vector<Entity>> entities, std::function<void()>);

		std::shared_ptr<Player> player;

		int view_port_x, view_port_y, view_port_width, view_port_height;
		int map[MAP_HEIGHT][MAP_WIDTH];
		int light_map[MAP_HEIGHT][MAP_WIDTH];

		const int COIN_VALUE = 50;
		const int HEALTH_GEM_VALUE = 25;
		const int NUMBER_OF_COINS_ON_MAP = 150;
		const int NUMBER_OF_HEALTH_GEMS_ON_MAP = 50;
		const int NUMBER_OF_ENEMIES_ON_MAP = 75;

		std::shared_ptr<std::vector<Entity>> coins;
		std::shared_ptr<std::vector<Entity>> health_gems;
		std::shared_ptr<std::vector<Entity>> enemies;
		std::shared_ptr<std::vector<Entity>> treasure_chests;
		std::shared_ptr<std::vector<Entity>> bonus;

		std::shared_ptr<std::queue<std::shared_ptr<CombatLog>>> combat_log;

		Entity golden_candle;

		std::ostringstream win_lose_message;
		std::ostringstream enemy_stats_info;
		std::ostringstream enemy_combat_info;
		std::ostringstream player_combat_info;
};