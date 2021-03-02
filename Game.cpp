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

		player = std::make_shared<Player>(60, 2);

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

		win_lose_message.str(" ");
		enemy_stats_info.str(" ");
		player_combat_info.str(" ");
		enemy_combat_info.str(" ");

		coins = std::make_shared<std::vector<Entity>>();
		health_gems = std::make_shared<std::vector<Entity>>();
		enemies = std::make_shared<std::vector<Entity>>();
		treasure_chests = std::make_shared<std::vector<Entity>>();
		bonus = std::make_shared<std::vector<Entity>>();

		combat_log = std::make_shared<std::queue<std::shared_ptr<CombatLog>>>();

		SpawnEntities(coins, NUMBER_OF_COINS_ON_MAP, EntityType::Pickup, EntitySubType::Coin);
		SpawnEntities(health_gems, NUMBER_OF_HEALTH_GEMS_ON_MAP, EntityType::Pickup, EntitySubType::Health_Gem);

		int num_enemies_for_each_type = NUMBER_OF_ENEMIES_ON_MAP / 5;

		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, EntitySubType::Spider);
		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, EntitySubType::Lurcher);
		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, EntitySubType::Crab);
		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, EntitySubType::Bug);
		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, EntitySubType::Fire_Walker);

		golden_candle = {
				GenerateRandomPoint(),
				EntityType::Pickup,
				nullptr,
				100
		};

		RB_FOV();
}

bool Game::IsEntityLocationTraversable(int x, int y, std::shared_ptr<std::vector<Entity>> entities)
{
		return !(std::any_of(entities->begin(), entities->end(), [&](const Entity& elem) { return elem.point.x == x && elem.point.y == y; }));
}

auto Game::IsEntityLocationTraversable(int x, int y, std::shared_ptr<std::vector<Entity>> entities, WhoAmI whoAmI, MovementDirection dir)
{
		for (const auto& e : *entities)
		{
				if ((dir == MovementDirection::Up && e.point.y == y - 1 && e.point.x == x) ||
						(dir == MovementDirection::Down && e.point.y == y + 1 && e.point.x == x) ||
						(dir == MovementDirection::Left && e.point.y == y && e.point.x == x - 1) ||
						(dir == MovementDirection::Right && e.point.y == y && e.point.x == x + 1))
				{
						if (e.entityType == EntityType::Enemy || whoAmI == WhoAmI::Enemy)
						{
								TileWalkableInfo twi{
										false,
										{ e.point.x, e.point.y }
								};

								return std::make_shared<TileWalkableInfo>(twi);
						}
				}
		}

		TileWalkableInfo twi{
				true,
				{ x, y }
		};

		return std::make_shared<TileWalkableInfo>(twi);
}

bool Game::IsTilePlayerTile(int x, int y, MovementDirection dir)
{
		return ((dir == MovementDirection::Up && player->Y() == y - 1 && player->X() == x) ||
				(dir == MovementDirection::Down && player->Y() == y + 1 && player->X() == x) ||
				(dir == MovementDirection::Left && player->Y() == y && player->X() == x - 1) ||
				(dir == MovementDirection::Right && player->Y() == y && player->X() == x + 1));
}

bool Game::IsTileOnMapTraversable(int x, int y, MovementDirection dir, int tileId)
{
		return !(dir == MovementDirection::Up && map[y - 1][x] == tileId ||
				dir == MovementDirection::Down && map[y + 1][x] == tileId ||
				dir == MovementDirection::Left && map[y][x - 1] == tileId ||
				dir == MovementDirection::Right && map[y][x + 1] == tileId);
}

bool Game::IsTileWalkable(int x, int y, MovementDirection dir, WhoAmI whoAmI)
{
		auto is_player = player->X() == x && player->Y() == y;

		if (IsTilePlayerTile(x, y, dir)) return false;

		auto enemy = IsEntityLocationTraversable(x, y, enemies, whoAmI, dir);

		if (!enemy->walkable && is_player)
				InitiateAttackSequence(enemy->point.x, enemy->point.y);

		auto coins_traversable = IsEntityLocationTraversable(x, y, coins, whoAmI, dir);
		auto health_gems_traversable = IsEntityLocationTraversable(x, y, health_gems, whoAmI, dir);
		auto enemies_traversable = IsEntityLocationTraversable(x, y, enemies, whoAmI, dir);

		return IsTileOnMapTraversable(x, y, dir, 0 /* Wall */) &&
				coins_traversable->walkable &&
				health_gems_traversable->walkable &&
				enemies_traversable->walkable &&
				enemy->walkable;
}

void Game::MovePlayerLeft()
{
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::Left, WhoAmI::Player)) return;

		player->X(player->X() - 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerRight()
{
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::Right, WhoAmI::Player)) return;

		player->X(player->X() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerDown()
{
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::Down, WhoAmI::Player)) return;

		player->Y(player->Y() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerUp()
{
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::Up, WhoAmI::Player)) return;

		player->Y(player->Y() - 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::UpdatePlayerViewPortPoints(int playerX, int playerY)
{
		view_port_x = playerX - VIEW_PORT_WIDTH;
		view_port_y = playerY - VIEW_PORT_HEIGHT;

		if (view_port_x < 0) view_port_x = std::max(0, view_port_x);
		if (view_port_x > (MAP_WIDTH - (VIEW_PORT_WIDTH * 2))) view_port_x = (MAP_WIDTH - (VIEW_PORT_WIDTH * 2));

		if (view_port_y < 0) view_port_y = std::max(0, view_port_y);
		if (view_port_y > (MAP_HEIGHT - (VIEW_PORT_HEIGHT * 2))) view_port_y = (MAP_HEIGHT - (VIEW_PORT_HEIGHT * 2));

		view_port_width = view_port_x + (VIEW_PORT_WIDTH * 2);
		view_port_height = view_port_y + (VIEW_PORT_HEIGHT * 2);
}

void Game::UpdateCollection(std::shared_ptr<std::vector<Entity>> entities, std::function<void()> fn)
{
		entities->erase(std::remove_if(entities->begin(), entities->end(),
				[&](Entity e) {
						if (e.point.x == player->X() && e.point.y == player->Y())
						{
								fn();
								return true;
						}
						return false;
				}), entities->end());
}

void Game::UpdateAfterPlayerMoved()
{
		UpdateCollection(coins,
				[&]() {
						player->SetScore(player->GetScore() + COIN_VALUE);
				});

		UpdateCollection(health_gems,
				[&]() {
						auto ph = player->GetHealth();
						if (ph < 100)
						{
								auto h = ph + HEALTH_GEM_VALUE;
								if (h > 100) h = 100;
								player->SetHealth(h);
						}
				});

		UpdateCollection(treasure_chests,
				[&]() {
						auto health_recovery_chance = std::rand() % 100 <= 20;
						auto extra_score_change = std::rand() % 100 <= 15;

						if (health_recovery_chance)
						{
								player->SetHealth(player->GetHealth() + 40);
						}

						if (extra_score_change)
						{
								player->SetScore(player->GetScore() + 50);
						}

						player->SetScore(player->GetScore() + 25);
				});

		UpdateCollection(bonus,
				[&]() {
						player->SetAttack(player->GetAttack() + 1);
				});

		if (golden_candle.point.x == player->X() && golden_candle.point.y == player->Y())
		{
				player->SetScore(player->GetScore() + 10000);
				win_lose_message << "YOU WIN!!!";
		}

		MoveEnemies();

		RB_FOV();
}

bool Game::IsXYBlocked(int x, int y)
{
		return (map[y][x] == 0 ||
				player->X() == x ||
				player->Y() == y ||
				(!(IsEntityLocationTraversable(x, y, coins) ||
						IsEntityLocationTraversable(x, y, health_gems) ||
						IsEntityLocationTraversable(x, y, bonus) ||
						IsEntityLocationTraversable(x, y, enemies) ||
						(x == golden_candle.point.x && y == golden_candle.point.y))));
}

Point Game::GenerateRandomPoint()
{
		int c = 0;
		int r = 0;

		do
		{
				c = std::rand() % (MAP_WIDTH - 1);
				r = std::rand() % (MAP_HEIGHT - 1);
		} while (IsXYBlocked(c, r));

		return { c, r };
}

Point Game::GetOpenPointForXY(int x, int y)
{
		int left = x - 1;
		int right = x + 1;
		int up = y - 1;
		int down = y + 1;

		if (!IsXYBlocked(left, y)) return { left, y };
		else if (!IsXYBlocked(right, y)) return { right, y };
		else if (!IsXYBlocked(x, up)) return { x, up };
		else if (!IsXYBlocked(x, down)) return { x, down };

		return GenerateRandomPoint();
}

void Game::SpawnEntity(std::shared_ptr<std::vector<Entity>> entity, EntityType entityType, EntitySubType entitySubType, int x, int y)
{
		auto components = std::make_shared<std::vector<std::shared_ptr<Component>>>();

		if (entityType == EntityType::Enemy)
		{
				int health, attack;

				if (entitySubType == EntitySubType::Spider) { health = 20; attack = 1; }
				else if (entitySubType == EntitySubType::Lurcher) { health = 30; attack = 2; }
				else if (entitySubType == EntitySubType::Crab) { health = 40; attack = 2; }
				else if (entitySubType == EntitySubType::Bug) { health = 50; attack = 2; }
				else if (entitySubType == EntitySubType::Fire_Walker) { health = 75; attack = 4; }

				components->push_back(std::make_shared<StatComponent>(health, attack));
		}

		components->push_back(std::make_shared<EntitySubTypeComponent>(entitySubType));

		entity->push_back(
				{
						{ x, y },
						entityType,
						components,
						static_cast<int>(entitySubType)
				});
}

void Game::SpawnEntities(std::shared_ptr<std::vector<Entity>> entity, int num, EntityType entityType, EntitySubType entitySubType)
{
		for (int i = 0; i < num; i++)
		{
				auto p = GenerateRandomPoint();
				SpawnEntity(entity, entityType, entitySubType, p.x, p.y);
		};
}

void Game::MoveEnemies()
{
		for (auto& enemy : *enemies)
		{
				if (std::rand() % 100 + 1 >= 40) continue;

				int direction = std::rand() % 4;
				Point loc = { enemy.point.x, enemy.point.y };

				if (direction == 0 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::Up, WhoAmI::Enemy))
				{
						loc.y -= 1;
				}
				else if (direction == 1 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::Down, WhoAmI::Enemy))
				{
						loc.y += 1;
				}
				else if (direction == 2 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::Left, WhoAmI::Enemy))
				{
						loc.x -= 1;
				}
				else if (direction == 3 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::Right, WhoAmI::Enemy))
				{
						loc.x += 1;
				}

				enemy.point = { loc.x, loc.y };
		}
}

void Game::ClearInfo()
{
		win_lose_message.str("");
		enemy_stats_info.str("");
		player_combat_info.str("");
		enemy_combat_info.str("");
}

template<typename T>
std::shared_ptr<T> Game::find_component(std::shared_ptr<std::vector<std::shared_ptr<Component>>> components)
{
		auto found_component = std::find_if(components->begin(), components->end(),
				[](std::shared_ptr<Component> c) {
						return std::dynamic_pointer_cast<T>(c) != nullptr;
				});

		std::shared_ptr<T> t_component = nullptr;

		if (*found_component != nullptr)
		{
				t_component = std::dynamic_pointer_cast<T>(*found_component);

				if (t_component != nullptr)
				{
						return t_component;
				}
		}

		return nullptr;
}

void Game::AddCombatLog(WhoAmI who, Point point, AttackType attack_type, CombatMultiplier combat_multiplier, int damage)
{
		std::ostringstream message;

		if (combat_multiplier == CombatMultiplier::Plus)
				message << "+";
		else if (combat_multiplier == CombatMultiplier::Minus)
				message << "-";

		message << damage;

		auto log = std::make_shared<CombatLog>();
		log->who = who;
		log->point = { point.x, point.y };
		log->attack_type = attack_type;
		log->message = message.str();

		combat_log->push(log);
}

void Game::InitiateAttackSequence(int x, int y)
{
		ClearInfo();

		auto enemy = std::find_if(enemies->begin(), enemies->end(),
				[&](Entity e) {
						return e.point.x == x && e.point.y == y;
				});

		auto enemy_stat_component = find_component<StatComponent>(enemy->components);

		if (enemy_stat_component != nullptr)
		{
				auto player_critical_strike = std::rand() % 100 <= 20;
				auto enemy_critical_strike = std::rand() % 100 <= 20;

				if (player_critical_strike)
				{
						player_combat_info.str("");

						auto damage = player->GetAttack() + (std::rand() % 5 + 1) * 2;
						auto enemy_health = enemy_stat_component->GetHealth() - damage;
						enemy_stat_component->SetHealth(enemy_health);
						player_combat_info << "Player CRITICALLY STRIKES for " << damage << " damage!!!";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();

						AddCombatLog(WhoAmI::Player, { enemy->point.x, enemy->point.y }, AttackType::Critical, CombatMultiplier::Plus, damage);
				}
				else
				{
						player_combat_info.str("");

						auto damage = player->GetAttack() + (std::rand() % 5 + 1);
						auto enemy_health = enemy_stat_component->GetHealth() - damage;
						enemy_stat_component->SetHealth(enemy_health);
						player_combat_info << "Player attacks for " << damage << " damage!!!";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();

						AddCombatLog(WhoAmI::Player, { enemy->point.x, enemy->point.y }, AttackType::Normal, CombatMultiplier::Plus, damage);
				}

				if (enemy_critical_strike)
				{
						enemy_stats_info.str("");

						auto damage = enemy_stat_component->GetAttack() + (std::rand() % 5 + 1) * 2;
						player->SetHealth(player->GetHealth() - damage);
						enemy_combat_info << "Enemy CRITICALLY STRIKES for " << damage << " damage!!!";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();

						AddCombatLog(WhoAmI::Enemy, { player->X(), player->Y() }, AttackType::Critical, CombatMultiplier::Minus, damage);
				}
				else
				{
						enemy_stats_info.str("");

						auto damage = enemy_stat_component->GetAttack() + (std::rand() % 5 + 1);
						player->SetHealth(player->GetHealth() - damage);
						enemy_combat_info << "Enemy attacks for " << damage << " damage";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();

						AddCombatLog(WhoAmI::Enemy, { player->X(), player->Y() }, AttackType::Normal, CombatMultiplier::Minus, damage);
				}

				if (enemy_stat_component->GetHealth() <= 0)
				{
						ClearInfo();

						player->SetEnemiesKilled(player->GetEnemiesKilled() + 1);
						player->SetScore(player->GetScore() + 25);

						auto enemy_sub_type_component = find_component<EntitySubTypeComponent>(enemy->components);

						if (enemy_sub_type_component != nullptr && enemy_sub_type_component->GetEntitySubType() == EntitySubType::Fire_Walker)
						{
								auto pos = GetOpenPointForXY(enemy->point.x, enemy->point.y);
								SpawnEntity(bonus, EntityType::Pickup, EntitySubType::Attack_Gem, pos.x, pos.y);
						}

						treasure_chests->push_back({
								{ enemy->point.x, enemy->point.y },
								EntityType::Pickup,
								nullptr,
								20
								});

						enemies->erase(std::remove_if(enemies->begin(), enemies->end(),
								[&](Entity e) {
										return e.point.x == enemy->point.x && e.point.y == enemy->point.y;
								}),
								enemies->end());

						ClearQueue<std::shared_ptr<CombatLog>>(*combat_log);
				}

				if (player->GetHealth() <= 0)
				{
						ClearInfo();

						player->SetHealth(0);
						win_lose_message << "You ded son!";
				}
		}
}

// Taken from http://www.roguebasin.com/index.php?title=Eligloscode
// Modified to fit in my game
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

				for (int j = 0; j < 40; j++)
				{
						light_map[(int)oy][(int)ox] = 2;

						if (map[(int)oy][(int)ox] == 0) // if tile is a wall
								break;

						ox += x;
						oy += y;
				};
		};
}

