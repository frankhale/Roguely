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

		SpawnEntities(coins, NUMBER_OF_COINS_ON_MAP, EntityType::Pickup, nullptr, 1);
		SpawnEntities(health_gems, NUMBER_OF_HEALTH_GEMS_ON_MAP, EntityType::Pickup, nullptr, 2);

		int num_enemies_for_each_type = NUMBER_OF_ENEMIES_ON_MAP / 4;

		auto spider_stat_components = std::make_shared<std::vector<std::shared_ptr<Component>>>();
		auto lurcher_stat_components = std::make_shared<std::vector<std::shared_ptr<Component>>>();
		auto crab_stat_components = std::make_shared<std::vector<std::shared_ptr<Component>>>();
		auto bug_stat_components = std::make_shared<std::vector<std::shared_ptr<Component>>>();

		auto spider_stat_component = std::make_shared<StatComponent>(10, 1);
		auto lurcher_stat_component = std::make_shared<StatComponent>(20, 2);
		auto crab_stat_component = std::make_shared<StatComponent>(30, 3);
		auto bug_stat_component = std::make_shared<StatComponent>(40, 4);

		spider_stat_components->emplace_back(spider_stat_component);
		lurcher_stat_components->push_back(lurcher_stat_component);
		crab_stat_components->push_back(crab_stat_component);
		bug_stat_components->push_back(bug_stat_component);

		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, spider_stat_components, Spider);
		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, lurcher_stat_components, Lurcher);
		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, crab_stat_components, Crab);
		SpawnEntities(enemies, num_enemies_for_each_type, EntityType::Enemy, bug_stat_components, Bug);

		golden_candle.entityType = EntityType::Pickup;
		golden_candle.id = 100;
		golden_candle.point = GenerateRandomPoint();

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
				if ((dir == MovementDirection::UP && e.point.y == y - 1 && e.point.x == x) ||
						(dir == MovementDirection::DOWN && e.point.y == y + 1 && e.point.x == x) ||
						(dir == MovementDirection::LEFT && e.point.y == y && e.point.x == x - 1) ||
						(dir == MovementDirection::RIGHT && e.point.y == y && e.point.x == x + 1))
				{
						if (e.entityType == EntityType::Enemy || whoAmI == WhoAmI::Enemy)
						{
								auto twi = std::make_shared<TileWalkableInfo>();
								twi->point = e.point;
								twi->walkable = false;
								return twi;
						}
				}
		}

		auto twi = std::make_shared<TileWalkableInfo>();
		twi->point = { x, y };
		twi->walkable = true;
		return twi;
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

bool Game::IsTileWalkable(int x, int y, MovementDirection dir, WhoAmI whoAmI)
{
		auto is_player = player->X() == x && player->Y() == y;

		if (IsTilePlayerTile(x, y, dir)) return false;

		auto enemy = IsEntityLocationTraversable(x, y, enemies, whoAmI, dir);

		if (!enemy->walkable && is_player)
		{
				InitiateAttackSequence(enemy->point.x, enemy->point.y);
		}
		else
		{

		}

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
		//if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::LEFT, 0)) return;
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::LEFT, WhoAmI::Player)) return;

		player->SetX(player->X() - 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerRight()
{
		//if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::RIGHT, 0)) return;
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::RIGHT, WhoAmI::Player)) return;

		player->SetX(player->X() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerDown()
{
		//if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::DOWN, 0)) return;
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::DOWN, WhoAmI::Player)) return;

		player->SetY(player->Y() + 1);
		UpdatePlayerViewPortPoints(player->X(), player->Y());
		UpdateAfterPlayerMoved();
};

void Game::MovePlayerUp()
{
		//if (!IsTileOnMapTraversable(player->X(), player->Y(), MovementDirection::UP, 0)) return;
		if (!IsTileWalkable(player->X(), player->Y(), MovementDirection::UP, WhoAmI::Player)) return;

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
						player->SetHealth(player->GetHealth() + HEALTH_GEM_VALUE);
				});

		UpdateCollection(treasure_chests,
				[&]() {
						auto health_recovery_chance = rand() % 100 <= 20;
						auto extra_score_change = rand() % 100 <= 15;

						if (health_recovery_chance)
						{
								player->SetHealth(player->GetHealth() + 15);
						}

						if (extra_score_change)
						{
								player->SetScore(player->GetScore() + 25);
						}

						player->SetScore(player->GetScore() + 10);
				});

		MoveEnemies();

		RB_FOV();
}

Point Game::GenerateRandomPoint()
{
		int c = 0;
		int r = 0;

		do
		{
				c = rand() % (MAP_WIDTH - 1);
				r = rand() % (MAP_HEIGHT - 1);
		} while (map[r][c] == 0 ||
				player->X() == c ||
				player->Y() == r ||
				(!(IsEntityLocationTraversable(c, r, coins) ||
						IsEntityLocationTraversable(c, r, health_gems) ||
						IsEntityLocationTraversable(c, r, enemies))));

		/*(std::any_of(coins->begin(), coins->end(), [&](const Entity& elem) { return elem.point.x == c && elem.point.y == r; })) ||
		(std::any_of(health_gems->begin(), health_gems->end(), [&](const Entity& elem) { return elem.point.x == c && elem.point.y == r; })) ||
		(std::any_of(enemies->begin(), enemies->end(), [&](const Entity& elem) { return elem.point.x == c && elem.point.y == r; }))
		);*/

		return { c, r };
}

void Game::SpawnEntities(std::shared_ptr<std::vector<Entity>> entity, int num, EntityType entityType, std::shared_ptr<std::vector<std::shared_ptr<Component>>> components, int id)
{
		for (int i = 0; i < num; i++)
		{
				auto p = GenerateRandomPoint();

				Entity e;
				e.point = { p.x, p.y };
				e.entityType = entityType;
				e.id = id;
				e.components = components;
				entity->push_back(e);
		}
}

void Game::MoveEnemies()
{
		for (auto& enemy : *enemies)
		{
				if (rand() % 100 + 1 >= 30) continue;

				int direction = rand() % 4;
				Point loc = { enemy.point.x, enemy.point.y };

				if (direction == 0 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::UP, WhoAmI::Enemy))
				{
						loc.y -= 1;
				}
				else if (direction == 1 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::DOWN, WhoAmI::Enemy))
				{
						loc.y += 1;
				}
				else if (direction == 2 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::LEFT, WhoAmI::Enemy))
				{
						loc.x -= 1;
				}
				else if (direction == 3 && IsTileWalkable(enemy.point.x, enemy.point.y, MovementDirection::RIGHT, WhoAmI::Enemy))
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

void Game::InitiateAttackSequence(int x, int y)
{
		ClearInfo();

		Entity enemy;		

		for (auto const& e : *enemies)
		{
				if (e.point.x == x && e.point.y)
				{
						enemy = e;
						break;
				}
		}

		auto enemy_stat_component = std::dynamic_pointer_cast<StatComponent>(enemy.components->front());

		if (enemy_stat_component != nullptr)
		{
				//enemy_stats_info << " Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();

				auto player_critical_strike = rand() % 100 <= 20;
				auto enemy_critical_strike = rand() % 100 <= 20;

				if (player_critical_strike)
				{
						player_combat_info.str("");

						auto damage = player->GetAttack() + (rand() % 5 + 1) * 2;
						auto enemy_health = enemy_stat_component->GetHealth() - damage;
						enemy_stat_component->SetHealth(enemy_health);
						player_combat_info << "Player CRITICALLY STRIKES for " << damage << " damage!!!";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();
				}
				else
				{
						player_combat_info.str("");

						auto damage = player->GetAttack() + (rand() % 5 + 1);
						auto enemy_health = enemy_stat_component->GetHealth() - damage;
						enemy_stat_component->SetHealth(enemy_health);
						player_combat_info << "Player attacks for " << damage << " damage!!!";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();
				}

				if (enemy_critical_strike)
				{
						enemy_stats_info.str("");

						auto damage = enemy_stat_component->GetAttack() + (rand() % 5 + 1) * 2;
						player->SetHealth(player->GetHealth() - damage);
						enemy_combat_info << "Enemy CRITICALLY STRIKES for " << damage << " damage!!!";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();
				}
				else
				{
						enemy_stats_info.str("");

						player->SetHealth(player->GetHealth() - enemy_stat_component->GetAttack());
						enemy_combat_info << "Enemy attacks for " << enemy_stat_component->GetAttack() + (rand() % 5 + 1) << " damage";
						enemy_stats_info << "Enemy Health: " << enemy_stat_component->GetHealth() << " | Attack: " << enemy_stat_component->GetAttack();
				}

				if (enemy_stat_component->GetHealth() <= 0)
				{
						ClearInfo();

						player->SetEnemiesKilled(player->GetEnemiesKilled() + 1);

						enemies->erase(std::remove_if(enemies->begin(), enemies->end(),
								[&](Entity e) {
										return e.point.x == enemy.point.x && e.point.y == enemy.point.y;
								}),
								enemies->end());

						Entity t_chest;
						t_chest.id = 20;
						t_chest.point = { enemy.point.x, enemy.point.y };
						t_chest.entityType = EntityType::Pickup;

						treasure_chests->push_back(t_chest);
				}

				if (player->GetHealth() <= 0)
				{
						ClearInfo();

						player->SetHealth(0);
						win_lose_message << "You ded son!";
				}
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
