/*
* Game.h
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

#include "Game.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace roguely::game
{
		Game::Game(sol::table gc)
		{
				game_config = gc;

				reset(false);

				sprite_sheets = std::make_shared<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>>();
				sounds = std::make_shared<std::vector<std::shared_ptr<roguely::common::Sound>>>();

				Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
				Mix_Volume(-1, 2);
				Mix_VolumeMusic(2);

				view_port_width = game_config["view_port_width"];
				view_port_height = game_config["view_port_height"];
				VIEW_PORT_WIDTH = view_port_width;
				VIEW_PORT_HEIGHT = view_port_height;

				if (game_config["sounds"].valid() && game_config["sounds"].get_type() == sol::type::table)
				{
						sol::table sound_table = game_config["sounds"];

						for (auto& sound : sound_table)
						{
								if (sound.first.get_type() == sol::type::string &&
										sound.second.get_type() == sol::type::string)
								{
										roguely::common::Sound s{
											sound.first.as<std::string>(),
											Mix_LoadWAV(sound.second.as<std::string>().c_str())
										};

										auto ss = std::make_shared<roguely::common::Sound>(s);

										sounds->emplace_back(ss);
								}
						}
				}

				if ((bool)game_config["music"])
				{
						std::string soundtrack_path = game_config["soundtrack_path"];

						soundtrack = Mix_LoadMUS(soundtrack_path.c_str());
						Mix_PlayMusic(soundtrack, 1);
				}

				std::string font_path = game_config["font_path"];
				text_medium = std::make_shared<roguely::common::Text>();
				text_medium->load_font(font_path.c_str(), 40);
				text_large = std::make_shared<roguely::common::Text>();
				text_large->load_font(font_path.c_str(), 63);
				text_small = std::make_shared<roguely::common::Text>();
				text_small->load_font(font_path.c_str(), 26);
		}

		void Game::tear_down_sdl()
		{
				if (soundtrack != nullptr)
				{
						Mix_FreeMusic(soundtrack);
				}

				for (auto& s : *sounds)
				{
						Mix_FreeChunk(s->sound);
				}

				sprite_sheets.reset();
		}

		void Game::reset(bool reset_ptr) {
				if (reset_ptr) {
						maps.reset();
						entity_groups.reset();
				}

				maps = std::make_shared<std::vector<std::shared_ptr<roguely::common::Map>>>();
				entity_groups = std::make_shared<std::vector<std::shared_ptr<roguely::ecs::EntityGroup>>>();
		}

		void Game::play_sound(std::string name)
		{
				if (!(name.length() > 0)) return;

				auto sound = std::find_if(sounds->begin(), sounds->end(),
						[&](const auto& s) {
								return s->name == name;
						});

				if (sound != sounds->end())
				{
						(*sound)->play();
				}
		}

		void Game::add_spritesheet(SDL_Renderer* renderer, std::string name, std::string path, int sw, int sh)
		{
				auto ss = std::make_shared<roguely::sprites::SpriteSheet>(renderer, name, path, sw, sh);
				sprite_sheets->emplace_back(ss);
		}

		void Game::draw_sprite(SDL_Renderer* renderer, std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height)
		{
				if (!(name.length() > 0)) return;

				auto sheet = std::find_if(sprite_sheets->begin(), sprite_sheets->end(),
						[&](const std::shared_ptr<roguely::sprites::SpriteSheet>& ss) {
								return ss->get_name() == name;
						});

				if (sheet != sprite_sheets->end())
				{
						(*sheet)->draw_sprite(renderer, sprite_id, x, y, scaled_width, scaled_height);
				}
		}

		void Game::draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y)
		{
				draw_text(renderer, t, size, x, y, 255, 255, 255, 255);
		}

		void Game::draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y, int r, int g, int b, int a)
		{
				if (!(t.length() > 0)) return;

				std::shared_ptr<roguely::common::Text> text = text_small;

				if (size == "small")
						text = text_small;
				else if (size == "medium")
						text = text_medium;
				else if (size == "large")
						text = text_large;

				SDL_Color text_color = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a };
				text->draw_text(renderer, x, y, t.c_str(), text_color);
		}


		roguely::common::TextExtents Game::get_text_extents(std::string t, std::string size)
		{
				std::shared_ptr<roguely::common::Text> text = text_small;

				if (size == "small")
						text = text_small;
				else if (size == "medium")
						text = text_medium;
				else if (size == "large")
						text = text_large;

				return text->get_text_extents(t.c_str());
		}

		void Game::generate_map_for_testing()
		{
				auto map = roguely::level_generation::init_cellular_automata(100, 40);
				roguely::level_generation::perform_cellular_automaton(map, 100, 40, 10);

				for (int row = 0; row < 40; row++)
				{
						for (int column = 0; column < 100; column++)
						{
								if ((*map)(row, column) == 0) {
										std::cout << "#";
								}
								else if ((*map)(row, column) == 9) {
										std::cout << ".";
								}
						}

						std::cout << std::endl;
				}
		}

		void Game::switch_map(std::string name)
		{
				auto map = std::find_if(maps->begin(), maps->end(),
						[&](const std::shared_ptr<roguely::common::Map>& m) {
								return m->name == name;
						});

				if (map != maps->end()) {
						current_map = *map;
				}
		}

		std::shared_ptr<roguely::common::Map> Game::get_map(std::string name)
		{
				auto map = std::find_if(maps->begin(), maps->end(),
						[&](const std::shared_ptr<roguely::common::Map>& m) {
								return m->name == name;
						});

				if (map != maps->end()) {
						return *map;
				}

				return nullptr;
		}

		std::shared_ptr<roguely::ecs::EntityGroup> Game::get_entity_group(std::string name)
		{
				auto group = std::find_if(entity_groups->begin(), entity_groups->end(),
						[&](const std::shared_ptr<roguely::ecs::EntityGroup>& eg) {
								return eg->name == name;
						});

				if (group != entity_groups->end()) {
						return *group;
				}

				return nullptr;
		}

		std::shared_ptr<roguely::ecs::Entity> Game::get_entity(std::shared_ptr<roguely::ecs::EntityGroup> entity_group, std::string entity_id)
		{
				auto entity = std::find_if(entity_group->entities->begin(), entity_group->entities->end(),
						[&](const std::shared_ptr<roguely::ecs::Entity>& m) {
								return m->get_id() == entity_id;
						});

				if (entity != entity_group->entities->end())
				{
						return *entity;
				}

				return nullptr;
		}

		std::shared_ptr<roguely::ecs::EntityGroup> Game::create_entity_group(std::string name)
		{
				auto entityGroup = std::make_shared<roguely::ecs::EntityGroup>();
				entityGroup->name = name;
				entityGroup->entities = std::make_shared<std::vector<std::shared_ptr<roguely::ecs::Entity>>>();
				entity_groups->emplace_back(entityGroup);
				return entityGroup;
		}

		std::shared_ptr<roguely::ecs::Entity> Game::add_entity_to_group(std::shared_ptr<roguely::ecs::EntityGroup> entityGroup, roguely::ecs::EntityType entity_type, std::string id, roguely::common::Point point)
		{
				auto entity = std::make_shared<roguely::ecs::Entity>(entityGroup, id, point, entity_type);

				if (entity_type == roguely::ecs::EntityType::Player)
				{
						player_id = id;
						player = entity;
				}

				entityGroup->entities->emplace_back(entity);

				return entity;
		}

		void Game::add_sprite_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string spritesheet_name, int sprite_in_spritesheet_id, std::string sprite_name)
		{
				auto sprite_component = std::make_shared<roguely::ecs::SpriteComponent>(spritesheet_name, sprite_in_spritesheet_id, sprite_name);
				sprite_component->set_component_name("sprite_component");
				entity->add_component(sprite_component);
		}

		void Game::add_health_component(std::shared_ptr<roguely::ecs::Entity> entity, int h)
		{
				auto health_component = std::make_shared<roguely::ecs::HealthComponent>(h);
				health_component->set_component_name("health_component");
				entity->add_component(health_component);
		}

		void Game::add_stats_component(std::shared_ptr<roguely::ecs::Entity> entity, int a)
		{
				auto stats_component = std::make_shared<roguely::ecs::StatsComponent>(a);
				stats_component->set_component_name("stats_component");
				entity->add_component(stats_component);
		}

		void Game::add_score_component(std::shared_ptr<roguely::ecs::Entity> entity, int s)
		{
				auto score_component = std::make_shared<roguely::ecs::ScoreComponent>(s);
				score_component->set_component_name("score_component");
				entity->add_component(score_component);
		}

		void Game::add_value_component(std::shared_ptr<roguely::ecs::Entity> entity, int v)
		{
				auto value_component = std::make_shared<roguely::ecs::ValueComponent>(v);
				value_component->set_component_name("value_component");
				entity->add_component(value_component);
		}

		void Game::add_inventory_component(std::shared_ptr<roguely::ecs::Entity> entity, std::vector<std::pair<std::string, int>> items)
		{
				auto inventory_component = std::make_shared<roguely::ecs::InventoryComponent>();

				for (auto& item : items)
				{
						inventory_component->add_item(item.first, item.second);
				}

				inventory_component->set_component_name("inventory_component");
				entity->add_component(inventory_component);
		}

		void Game::add_lua_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string n, std::string t, sol::table props)
		{
				auto lua_component = std::make_shared<roguely::ecs::LuaComponent>(n, t, props);
				lua_component->set_component_name(n);
				entity->add_component(lua_component);
		}

		bool Game::remove_entity(std::string entity_group_name, std::string entity_id)
		{
				bool result = false;
				auto entity_group = get_entity_group(entity_group_name);

				if (entity_group != nullptr)
				{
						entity_group->entities->erase(std::remove_if(entity_group->entities->begin(), entity_group->entities->end(),
								[&](std::shared_ptr<roguely::ecs::Entity> e) {
										if (e->get_id() == entity_id) {
												result = true;
												return true;
										}

										return false;
								}), entity_group->entities->end());
				}

				return result;
		}

		std::string Game::generate_uuid()
		{
				boost::uuids::random_generator gen;
				boost::uuids::uuid id = gen();
				return boost::uuids::to_string(id);
		}

		bool Game::is_tile_player_tile(int x, int y, roguely::common::MovementDirection dir)
		{
				return ((dir == roguely::common::MovementDirection::Up && player->y() == y - 1 && player->x() == x) ||
						(dir == roguely::common::MovementDirection::Down && player->y() == y + 1 && player->x() == x) ||
						(dir == roguely::common::MovementDirection::Left && player->y() == y && player->x() == x - 1) ||
						(dir == roguely::common::MovementDirection::Right && player->y() == y && player->x() == x + 1));
		}

		auto Game::is_entity_location_traversable(int x, int y, std::shared_ptr<std::vector<std::shared_ptr<roguely::ecs::Entity>>> entities, roguely::common::WhoAmI whoAmI, roguely::common::MovementDirection dir)
		{
				TileWalkableInfo twi{
						true,
						{ x, y },
						roguely::ecs::EntityType::Ground /* treat everything as ground if its traversable */
				};

				for (const auto& e : *entities)
				{
						if ((dir == roguely::common::MovementDirection::Up && e->y() == y - 1 && e->x() == x) ||
								(dir == roguely::common::MovementDirection::Down && e->y() == y + 1 && e->x() == x) ||
								(dir == roguely::common::MovementDirection::Left && e->y() == y && e->x() == x - 1) ||
								(dir == roguely::common::MovementDirection::Right && e->y() == y && e->x() == x + 1))
						{
								if (e->get_entity_type() == roguely::ecs::EntityType::Enemy ||
										whoAmI == roguely::common::WhoAmI::Enemy) {
										
										twi = {
												false,
												{ e->x(), e->y() },
												e->get_entity_type()
										};

										break;
								}
						}
				}
				
				return std::make_shared<TileWalkableInfo>(twi);
		}

		bool Game::is_tile_on_map_traversable(int x, int y, roguely::common::MovementDirection dir, int tileId)
		{
				if (current_map->map == nullptr) return false;

				bool result = true;

				if (dir == roguely::common::MovementDirection::Up && (*current_map->map)((size_t)y - 1, x) == tileId) result = false;
				else if (dir == roguely::common::MovementDirection::Down && (*current_map->map)((size_t)y + 1, x) == tileId) result = false;
				else if (dir == roguely::common::MovementDirection::Left && (*current_map->map)(y, (size_t)x - 1) == tileId) result = false;
				else if (dir == roguely::common::MovementDirection::Right && (*current_map->map)(y, (size_t)x + 1) == tileId) result = false;

				return result;
		}

		TileWalkableInfo Game::is_tile_walkable(int x, int y, roguely::common::MovementDirection dir, roguely::common::WhoAmI whoAmI, std::vector<std::string> entity_groups_to_check)
		{
				TileWalkableInfo result{ false, { x, y }, roguely::ecs::EntityType::Wall };

				if (!is_tile_on_map_traversable(x, y, dir, 0 /* Wall */)) return result;

				// for enemy movement
				if (whoAmI != roguely::common::WhoAmI::Player && is_tile_player_tile(x, y, dir)) {
						return { false, { x, y }, roguely::ecs::EntityType::Player };
				};

				for (auto& egtc : entity_groups_to_check)
				{
						auto group = std::find_if(entity_groups->begin(), entity_groups->end(),
								[&](const auto& eg) {
										return eg->name == egtc;
								});

						if (group != entity_groups->end())
								result = *is_entity_location_traversable(x, y, (*group)->entities, whoAmI, dir);
				}

				return result;
		}

		bool Game::is_xy_blocked(int x, int y, std::vector<std::string> entity_groups_to_check)
		{
				if (current_map->map == nullptr) return true;
				if (player->x() == x || player->y() == y) return true;
				if ((*current_map->map)(y, x) == 0) return true;

				bool blocked = true;

				for (auto& egtc : entity_groups_to_check)
				{
						auto group = get_entity_group(egtc);
						blocked = (is_entity_location_traversable(x, y, group->entities)) ? false : true;

						if (blocked) break;
				}

				return blocked;
		}

		roguely::common::Point Game::generate_random_point(std::vector<std::string> entity_groups_to_check)
		{
				if (current_map->map == nullptr) return {};

				int c = 0;
				int r = 0;

				do
				{
						c = std::rand() % (current_map->width - 1);
						r = std::rand() % (current_map->height - 1);
				} while (is_xy_blocked(c, r, entity_groups_to_check));

				return { c, r };
		}

		roguely::common::Point Game::get_open_point_for_xy(int x, int y, std::vector<std::string> entity_groups_to_check)
		{
				int left = x - 1;
				int right = x + 1;
				int up = y - 1;
				int down = y + 1;

				if (!is_xy_blocked(left, y, entity_groups_to_check)) return { left, y };
				else if (!is_xy_blocked(right, y, entity_groups_to_check)) return { right, y };
				else if (!is_xy_blocked(x, up, entity_groups_to_check)) return { x, up };
				else if (!is_xy_blocked(x, down, entity_groups_to_check)) return { x, down };

				return generate_random_point(entity_groups_to_check);
		}

		void Game::update_player_viewport_points()
		{
				view_port_x = player->x() - VIEW_PORT_WIDTH / 2;
				view_port_y = player->y() - VIEW_PORT_HEIGHT / 2;

				if (view_port_x < 0) view_port_x = std::max(0, view_port_x);
				if (view_port_x > (current_map->width - VIEW_PORT_WIDTH)) view_port_x = (current_map->width - VIEW_PORT_WIDTH);

				if (view_port_y < 0) view_port_y = std::max(0, view_port_y);
				if (view_port_y > (current_map->height - VIEW_PORT_HEIGHT)) view_port_y = (current_map->height - VIEW_PORT_HEIGHT);

				view_port_width = view_port_x + VIEW_PORT_WIDTH;
				view_port_height = view_port_y + VIEW_PORT_HEIGHT;
		}

		std::shared_ptr<roguely::ecs::Entity> Game::update_entity_position(std::string entity_group_name, std::string entity_id, int x, int y)
		{
				std::shared_ptr<roguely::ecs::Entity> entity{};

				if (entity_id == "player" || player->get_id() == entity_id)
				{
						player->set_point({ x, y });

						update_player_viewport_points();
						rb_fov();
						entity = player;
				}
				else
				{
						auto entity_group = get_entity_group(entity_group_name);

						if (entity_group != nullptr)
						{
								entity = get_entity(entity_group, entity_id);

								if (entity != nullptr) {
										entity->set_point({ x, y });
								}
						}
				}

				return entity;
		}

		int Game::get_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key)
		{
				int result = -1;

				if (component->get_component_name() == "score_component")
				{
						auto sc = std::dynamic_pointer_cast<roguely::ecs::ScoreComponent>(component);
						if (sc != nullptr)
						{
								result = sc->get_score();
						}
				}
				else if (component->get_component_name() == "health_component")
				{
						auto hc = std::static_pointer_cast<roguely::ecs::HealthComponent>(component);
						if (hc != nullptr)
						{
								result = hc->get_health();
						}
				}
				else if (component->get_component_name() == "stats_component")
				{
						auto sc = std::static_pointer_cast<roguely::ecs::StatsComponent>(component);
						if (sc != nullptr)
						{
								result = sc->get_attack();
						}
				}

				return result;
		}

		int Game::get_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key)
		{
				int result = -1;

				if (entity_id == "player" || player->get_id() == entity_id)
				{
						auto component = player->find_component_by_name(component_name);

						if (component != nullptr)
								result = get_component_value(component, key);
						else
						{
								auto entity_group = get_entity_group(entity_group_name);

								if (entity_group != nullptr)
								{
										auto entity = get_entity(entity_group, entity_id);

										if (entity != nullptr)
										{
												auto component = entity->find_component_by_name(component_name);

												if (component != nullptr)
														result = get_component_value(component, key);
										}
								}
						}
				}

				return result;
		}

		bool Game::set_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key, int value)
		{
				bool did_update = false;

				if (component->get_component_name() == "score_component") {
						auto sc = std::dynamic_pointer_cast<roguely::ecs::ScoreComponent>(component);
						if (sc != nullptr)
						{
								sc->set_score(value);
								did_update = true;
						}
				}
				else if (component->get_component_name() == "health_component") {
						auto hc = std::static_pointer_cast<roguely::ecs::HealthComponent>(component);
						if (hc != nullptr)
						{
								hc->set_health(value);
								did_update = true;
						}
				}
				else if (component->get_component_name() == "stats_component") {
						auto sc = std::static_pointer_cast<roguely::ecs::StatsComponent>(component);
						if (sc != nullptr)
						{
								sc->set_attack(value);
								did_update = true;
						}
				}

				return did_update;
		}

		std::shared_ptr<roguely::ecs::Entity> Game::set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, int value)
		{
				if (entity_id == "player" || player->get_id() == entity_id)
				{
						auto component = player->find_component_by_name(component_name);

						if (component != nullptr)
						{
								auto did_update = set_component_value(component, key, value);

								if (did_update)
										return player;
						}
				}
				else
				{
						auto entity_group = get_entity_group(entity_group_name);

						if (entity_group != nullptr)
						{
								auto entity = get_entity(entity_group, entity_id);

								if (entity != nullptr)
								{
										auto component = entity->find_component_by_name(component_name);

										if (component != nullptr)
										{
												auto did_update = set_component_value(component, key, value);

												if (did_update)
														return entity;
										}
								}
						}
				}

				return nullptr;
		}

		std::shared_ptr<roguely::ecs::Entity> Game::set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, std::pair<std::string, int> value)
		{
				if (component_name != "inventory_component") return nullptr;

				if (entity_id == "player" || player->get_id() == entity_id)
				{
						auto component = player->find_component_by_name(component_name);

						if (component != nullptr)
						{
								auto ic = std::dynamic_pointer_cast<std::shared_ptr<roguely::ecs::InventoryComponent>>(component);
								if (ic != nullptr)
								{
										(*ic)->upsert_item(value);
										return player;
								}
						}
				}

				return nullptr;
		}

		// Taken from http://www.roguebasin.com/index.php?title=Eligloscode
		// Modified to fit in my game
		void Game::rb_fov()
		{
				float x = 0, y = 0;

				current_map->light_map = std::make_shared<boost::numeric::ublas::matrix<int>>(current_map->height, current_map->width, 0);

				/*for (int r = 0; r < current_map->height; r++)
				{
						for (int c = 0; c < current_map->width; c++)
						{
								(*current_map->light_map)(r, c) = 0;
						}
				}*/

				for (int i = 0; i < 360; i++)
				{
						x = (float)std::cos(i * 0.01745f);
						y = (float)std::sin(i * 0.01745f);

						float ox = (float)player->x() + 0.5f;
						float oy = (float)player->y() + 0.5f;

						for (int j = 0; j < 40; j++)
						{
								(*current_map->light_map)((int)oy, (int)ox) = 2;

								if ((*current_map->map)((int)oy, (int)ox) == 0 ||
										(*current_map->map)((int)oy, (int)ox) == 35 ||
										(*current_map->map)((int)oy, (int)ox) == 36) // if tile is a wall
										break;

								ox += x;
								oy += y;
						};
				};
		}
}
