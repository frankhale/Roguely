/*
* main.hpp
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

// (gci -include *.cpp,*.h,*.lua -recurse | select-string .).Count

#include "Common.h"
#include "Entity.h"
#include "Text.h"
#include "SpriteSheet.h"
#include "Game.h"

SDL_Window* window = nullptr;
SDL_Surface* window_surface = nullptr;
SDL_Renderer* renderer = nullptr;
std::shared_ptr<roguely::game::Game> game{};

int init_sdl(sol::table game_config)
{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		{
				std::cout << "Failed to initialize the SDL2 library\n";
				return -1;
		}

		if (IMG_Init(IMG_INIT_PNG) < 0)
		{
				std::cout << "Failed to initialize the SDL2 image library\n";
				return -1;
		}

		if (TTF_Init() < 0)
		{
				std::cout << "Failed to initialize the SDL2 TTF library\n";
				return -1;
		}

		if (Mix_Init(MIX_INIT_MP3) == 0)
		{
				std::cout << "Failed to initialize the SDL2 mixer library\n";
				std::cout << SDL_GetError() << std::endl;
				return -1;
		}

		std::string window_title = game_config["window_title"];
		std::string window_icon_path = game_config["window_icon_path"];
		int window_width = game_config["window_width"];
		int window_height = game_config["window_height"];

		window = SDL_CreateWindow(window_title.c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				window_width, window_height,
				SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		if (!window)
		{
				std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
				return -1;
		}

		window_surface = SDL_GetWindowSurface(window);

		SDL_Surface* window_icon_surface = IMG_Load(window_icon_path.c_str());
		SDL_SetWindowIcon(window, window_icon_surface);
		SDL_FreeSurface(window_icon_surface);

		if (!window_surface)
		{
				std::cout << "Failed to get the surface from the window: " << SDL_GetError() << std::endl;
				return -1;
		}

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

		return 0;
}

void tear_down_sdl(sol::table game_config)
{
		game->tear_down_sdl();

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		Mix_Quit();
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
}

bool check_game_config(sol::table game_config)
{
		bool result = true;

		// TODO: Add more checks in here so that we can cover 100% of the config

		auto title = game_config["window_title"];
		auto window_width = game_config["window_width"];
		auto window_height = game_config["window_height"];
		auto music = game_config["music"];
		auto soundtrack_path = game_config["soundtrack_path"];
		auto map_width = game_config["map_width"];
		auto map_height = game_config["map_height"];
		auto view_port_width = game_config["view_port_width"];
		auto view_port_height = game_config["view_port_height"];

		if (!(title.valid() && title.get_type() == sol::type::string))
				return false;
		if (!(window_width.valid() && window_width.get_type() == sol::type::number))
				return false;
		if (!(window_height.valid() && window_height.get_type() == sol::type::number))
				return false;
		if (!(music.valid() && music.get_type() == sol::type::boolean))
				return false;
		if (!(soundtrack_path.valid() && soundtrack_path.get_type() == sol::type::string))
				return false;
		if (!(map_width.valid() && map_width.get_type() == sol::type::number))
				return false;
		if (!(map_height.valid() && map_height.get_type() == sol::type::number))
				return false;
		if (!(view_port_width.valid() && view_port_width.get_type() == sol::type::number))
				return false;
		if (!(view_port_height.valid() && view_port_height.get_type() == sol::type::number))
				return false;

		return result;
}

void init_game(sol::table game_config)
{
		std::srand(static_cast<unsigned int>(std::time(nullptr)));	
		game = std::make_shared<roguely::game::Game>(game_config);
}

sol::table get_sprite_info(const std::shared_ptr<roguely::game::Game> &game, std::string name, sol::this_state s)
{
		if (!(name.length() > 0)) return nullptr;

		auto sheet = game->find_spritesheet(name);

		if (sheet != nullptr)
				return sheet->get_sprites_as_lua_table(s);

		return nullptr;
}

sol::table add_sprite_sheet(SDL_Renderer* renderer, const std::shared_ptr<roguely::game::Game> &game, std::string name, std::string path, int sw, int sh, sol::this_state s)
{
		sol::state_view lua(s);

		if (!(name.length() > 0 && path.length() > 0)) return nullptr;

		game->add_spritesheet(renderer, name, path, sw, sh);

		auto sheet = game->find_spritesheet(name);

		if (sheet != nullptr)
				return sheet->get_sprites_as_lua_table(lua.lua_state());

		return lua.create_table();
}

sol::table convert_entity_to_lua_table(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s)
{
		sol::state_view lua(s);
		std::string entity_type{};

		// Don't @ me bruh!
		if (entity->get_entity_type() == roguely::ecs::EntityType::Player) entity_type = "player";
		else if (entity->get_entity_type() == roguely::ecs::EntityType::Enemy) entity_type = "enemy";
		else if (entity->get_entity_type() == roguely::ecs::EntityType::NPC) entity_type = "npc";
		else if (entity->get_entity_type() == roguely::ecs::EntityType::Item) entity_type = "item";
		else if (entity->get_entity_type() == roguely::ecs::EntityType::Interactable) entity_type = "interactable";
		else if (entity->get_entity_type() == roguely::ecs::EntityType::Ground) entity_type = "ground";
		else if (entity->get_entity_type() == roguely::ecs::EntityType::Wall) entity_type = "wall";

		auto e_p = entity->get_point();
		std::string e_id = entity->get_id();
		sol::table entity_info_table = lua.create_table();
		entity_info_table[entity_type] = lua.create_table();
		entity_info_table[entity_type]["id"] = e_id;
		entity_info_table[entity_type]["point"] = lua.create_table();
		entity_info_table[entity_type]["point"]["x"] = e_p.x;
		entity_info_table[entity_type]["point"]["y"] = e_p.y;
		entity_info_table[entity_type]["components"] = lua.create_table();

		entity->for_each_component([&](std::shared_ptr<roguely::ecs::Component> c) {
				if (c != nullptr)
				{
						if (c->get_component_name() == "sprite_component") {
								auto sc = std::dynamic_pointer_cast<roguely::ecs::SpriteComponent>(c);
								if (sc != nullptr)
								{
										entity_info_table[entity_type]["components"]["sprite_component"] = lua.create_table();
										entity_info_table[entity_type]["components"]["sprite_component"]["name"] = sc->get_name();
										entity_info_table[entity_type]["components"]["sprite_component"]["sprite_id"] = sc->get_sprite_id();
										entity_info_table[entity_type]["components"]["sprite_component"]["spritesheet_name"] = sc->get_spritesheet_name();
								}
						}
						else if (c->get_component_name() == "score_component") {
								auto sc = std::dynamic_pointer_cast<roguely::ecs::ScoreComponent>(c);
								if (sc != nullptr)
								{
										entity_info_table[entity_type]["components"]["score_component"] = lua.create_table();
										entity_info_table[entity_type]["components"]["score_component"]["score"] = sc->get_score();
								}
						}
						else if (c->get_component_name() == "health_component") {
								auto hc = std::static_pointer_cast<roguely::ecs::HealthComponent>(c);
								if (hc != nullptr)
								{
										entity_info_table[entity_type]["components"]["health_component"] = lua.create_table();
										entity_info_table[entity_type]["components"]["health_component"]["health"] = hc->get_health();
										entity_info_table[entity_type]["components"]["health_component"]["max_health"] = hc->get_max_health();
								}
						}
						else if (c->get_component_name() == "stats_component") {
								auto sc = std::static_pointer_cast<roguely::ecs::StatsComponent>(c);
								if (sc != nullptr)
								{
										entity_info_table[entity_type]["components"]["stats_component"] = lua.create_table();
										entity_info_table[entity_type]["components"]["stats_component"]["attack"] = sc->get_attack();
								}
						}
						else if (c->get_component_name() == "value_component") {
								auto vc = std::static_pointer_cast<roguely::ecs::ValueComponent>(c);
								if (vc != nullptr)
								{
										entity_info_table[entity_type]["components"]["value_component"] = lua.create_table();
										entity_info_table[entity_type]["components"]["value_component"]["value"] = vc->get_value();
								}
						}
						else if (c->get_component_name() == "inventory_component")
						{
								auto ic = std::static_pointer_cast<roguely::ecs::InventoryComponent>(c);
								if (ic != nullptr)
								{
										entity_info_table[entity_type]["components"]["inventory_component"] = lua.create_table();
										entity_info_table[entity_type]["components"]["inventory_component"]["items"] = lua.create_table();
										ic->for_each_item([&](std::shared_ptr<std::pair<std::string, int>> i) {
												entity_info_table[entity_type]["components"]["inventory_component"]["items"][i->first] = i->second;
												});
								}
						}
						else
						{
								auto lc = std::static_pointer_cast<roguely::ecs::LuaComponent>(c);
								if (lc != nullptr)
								{
										auto name = lc->get_name();

										entity_info_table[entity_type]["components"][name] = lua.create_table();
										entity_info_table[entity_type]["components"][name]["properties"] = lc->get_properties();
								}
						}
				}
				});

		return entity_info_table;
}

sol::table convert_entity_group_to_lua_table(const std::shared_ptr<roguely::game::Game> &game, std::string entity_group_name, sol::this_state s)
{
		sol::state_view lua(s);
		auto entity_group = game->get_entity_group(entity_group_name);

		sol::table entity_group_info_table = lua.create_table();

		for (auto& eg : *entity_group->entities)
		{
				std::ostringstream xy_id;
				auto p = eg->get_point();
				xy_id << p.x << "_" << p.y;

				entity_group_info_table.set(xy_id.str(), convert_entity_to_lua_table(eg, lua.lua_state()));
		}

		return entity_group_info_table;
}

std::string add_entity(const std::shared_ptr<roguely::game::Game> &game, std::string entity_group_name, std::string entity_type, int x, int y, sol::table components_table)
{
		auto e_id = game->generate_uuid();
		roguely::common::Point e_p = { x, y };
		roguely::ecs::EntityType e_type{};

		if (entity_type == "player") e_type = roguely::ecs::EntityType::Player;
		else if (entity_type == "enemy") e_type = roguely::ecs::EntityType::Enemy;
		else if (entity_type == "npc") e_type = roguely::ecs::EntityType::NPC;
		else if (entity_type == "item") e_type = roguely::ecs::EntityType::Item;
		else if (entity_type == "interactable") e_type = roguely::ecs::EntityType::Interactable;
		else if (entity_type == "ground") e_type = roguely::ecs::EntityType::Ground;
		else if (entity_type == "wall") e_type = roguely::ecs::EntityType::Wall;

		auto entity_group = game->get_entity_group(entity_group_name);

		if (entity_group == nullptr)
		{
				entity_group = game->create_entity_group(entity_group_name);
		}

		if (entity_group != nullptr) {
				auto entity = game->add_entity_to_group(entity_group, e_type, e_id, e_p);

				if (components_table.valid())
				{
						// loop over components table and add components
						for (auto& c : components_table)
						{
								std::string key = c.first.as<std::string>();
								sol::table value_table = c.second.as<sol::table>();

								if (value_table.valid()) {
										if (key == "value_component")
										{
												for (auto& cc : value_table)
												{
														if (cc.first.get_type() == sol::type::string && cc.second.get_type() == sol::type::number)
														{
																if (cc.first.as<std::string>() == "value")
																{
																		auto value = cc.second.as<int>();
																		game->add_value_component(entity, value);
																}
														}
												}
										}
										else if (key == "sprite_component")
										{
												std::string sprite_name{};
												std::string spritesheet_name{};
												int sprite_id = 0;

												for (auto& cc : value_table)
												{
														if (cc.first.get_type() == sol::type::string && cc.first.as<std::string>() == "name")
														{
																sprite_name = cc.second.as<std::string>();
														}
														else if (cc.first.get_type() == sol::type::string && cc.first.as<std::string>() == "spritesheet_name")
														{
																spritesheet_name = cc.second.as<std::string>();
														}
														else if (cc.first.get_type() == sol::type::string && cc.first.as<std::string>() == "sprite_id")
														{
																sprite_id = cc.second.as<int>();
														}

														if (sprite_name.length() > 0 && spritesheet_name.length() > 0)
														{
																game->add_sprite_component(entity, spritesheet_name, sprite_id, sprite_name);
														}
												}
										}
										else if (key == "health_component")
										{
												for (auto& cc : value_table)
												{
														if (cc.first.get_type() == sol::type::string && cc.second.get_type() == sol::type::number)
														{
																if (cc.first.as<std::string>() == "health")
																{
																		auto health = cc.second.as<int>();
																		game->add_health_component(entity, health);
																}
														}
												}
										}
										else if (key == "stats_component")
										{
												for (auto& cc : value_table)
												{
														if (cc.first.get_type() == sol::type::string && cc.second.get_type() == sol::type::number)
														{
																if (cc.first.as<std::string>() == "attack")
																{
																		auto attack = cc.second.as<int>();
																		game->add_stats_component(entity, attack);
																}
														}
												}
										}
										else if (key == "score_component")
										{
												for (auto& cc : value_table)
												{
														if (cc.first.get_type() == sol::type::string && cc.second.get_type() == sol::type::number)
														{
																if (cc.first.as<std::string>() == "score")
																{
																		auto score = cc.second.as<int>();
																		game->add_score_component(entity, score);
																}
														}
												}
										}
										else if (key == "inventory_component")
										{
												for (auto& cc : value_table)
												{
														if (cc.first.get_type() == sol::type::string && cc.second.get_type() == sol::type::table)
														{
																sol::table items_table = cc.second.as<sol::table>();

																if (cc.first.as<std::string>() == "items" && items_table.valid())
																{
																		std::vector<std::pair<std::string, int>> items{};

																		for (auto& it : items_table)
																		{
																				if (it.first.get_type() == sol::type::string && it.second.get_type() == sol::type::number)
																				{
																						items.push_back({ it.first.as<std::string>(), it.second.as<int>() });
																				}
																		}

																		game->add_inventory_component(entity, items);
																}
														}
												}
										}
										else
										{
												std::string type{};
												sol::table props{};
												bool has_type = false;
												bool has_properties = false;

												for (auto& cc : value_table)
												{
														if (cc.first.as<std::string>() == "type" && cc.second.get_type() == sol::type::string)
														{
																type = cc.second.as<std::string>();
																has_type = true;
														}
														else if (cc.first.as<std::string>() == "properties" && cc.second.get_type() == sol::type::table)
														{
																props = cc.second.as<sol::table>();
																has_properties = true;
														}
												}

												if (has_type && has_properties)
												{
														game->add_lua_component(entity, key, type, props);
												}
										}
								}
						}
				}
		}

		return e_id;
}

std::string add_entity(const std::shared_ptr<roguely::game::Game> &game, std::string entity_group_name, std::string entity_type, sol::table components_table)
{
		auto entity_groups = game->get_entity_group_names();
		auto random_point = game->generate_random_point(entity_groups);
		return add_entity(game, entity_group_name, entity_type, random_point.x, random_point.y, components_table);
}



sol::table add_entities(const std::shared_ptr<roguely::game::Game> &game, std::string entity_group_name, std::string entity_type, sol::table components_table, int num, sol::this_state s)
{
		sol::state_view lua(s);

		for (int i = 0; i < num; i++)
				add_entity(game, entity_group_name, entity_type, components_table);

		return convert_entity_group_to_lua_table(game, entity_group_name, lua.lua_state());
}

void emit_lua_update_for_entity_group(std::string entity_group_name, std::string entity_id, sol::this_state s)
{
		sol::state_view lua(s);
		auto lua_update = lua["_update"];
		if (lua_update.valid() && lua_update.get_type() == sol::type::function)
		{
				sol::table data_table = lua.create_table();
				data_table.set("entity_group_name", entity_group_name);
				data_table.set("entity_id", entity_id);
				data_table.set("entity_group", convert_entity_group_to_lua_table(game, entity_group_name, lua.lua_state()));

				lua_update("entity_event", data_table);
		}
}

void remove_entity(const std::shared_ptr<roguely::game::Game> &game, std::string entity_group_name, std::string entity_id, sol::this_state s)
{
		//sol::state_view lua(s);
		auto result = game->remove_entity(entity_group_name, entity_id);

		if (result)
				emit_lua_update_for_entity_group(entity_group_name, entity_id, s);
		/*{
				auto lua_update = lua["_update"];
				if (lua_update.valid() && lua_update.get_type() == sol::type::function)
				{
						sol::table data_table = lua.create_table();
						data_table.set("entity_group_name", entity_group_name);
						data_table.set("entity_id", entity_id);
						data_table.set("entity_group", convert_entity_group_to_lua_table(game, entity_group_name, lua.lua_state()));

						lua_update("entity_event", data_table);
				}
		}*/
}

sol::table get_test_map(sol::this_state s)
{
		sol::state_view lua(s);

		const int width = 10;
		const int height = 10;

		int m[width][height] = {
				{0,0,0,0,0,0,0,0,0,0},
				{0,9,9,9,9,9,9,58,42,0},
				{0,9,3,9,4,9,9,9,58,0},
				{0,9,9,9,9,9,5,9,9,0},
				{0,0,0,0,0,0,0,0,9,0},
				{0,9,9,9,9,9,9,9,9,0},
				{0,9,21,9,0,0,0,0,15,0},
				{0,9,9,9,0,6,9,34,9,0},
				{0,22,9,9,0,9,9,9,9,0},
				{0,0,0,0,0,0,0,0,0,0}
		};

		sol::table map_table = lua.create_table();

		for (int r = 1; r <= height; r++)
		{
				sol::table row_table = lua.create_table();
				map_table.set(r, row_table);

				for (int c = 1; c <= width; c++)
				{
						row_table.set(c, m[r - 1][c - 1]);
				}
		}

		return map_table;
}

sol::table get_map(const std::shared_ptr<roguely::game::Game> &game, std::string name, bool light, sol::this_state s)
{
		sol::state_view lua(s);

		auto map = game->get_map(name);

		if (map == nullptr)
		{
				std::ostringstream err;
				err << "Cannot find map: " << name;
				lua["_error"](err.str());

				return lua.create_table();
		}

		auto m = light ? map->light_map : map->map;

		sol::table map_table = lua.create_table();

		for (int r = 1; r <= map->height; r++)
		{
				sol::table row_table = lua.create_table();
				map_table.set(r, row_table);

				for (int c = 1; c <= map->width; c++)
				{
						row_table.set(c, (*m)((size_t)r - 1, (size_t)c - 1));
				}
		}

		return map_table;
}

void set_draw_color(SDL_Renderer* renderer, int r, int g, int b, int a)
{
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void draw_point(SDL_Renderer* renderer, int x, int y)
{
		SDL_RenderDrawPoint(renderer, x, y);
}

void draw_rect(SDL_Renderer* renderer, int x, int y, int w, int h)
{
		SDL_Rect r = { x, y, w, h };
		SDL_RenderDrawRect(renderer, &r);
}

void draw_filled_rect(SDL_Renderer* renderer, int x, int y, int w, int h)
{
		SDL_Rect r = { x, y, w, h };
		SDL_RenderFillRect(renderer, &r);
}

sol::table get_random_point(const std::shared_ptr<roguely::game::Game> &game, sol::table entity_groups_to_check, sol::this_state s)
{
		sol::state_view lua(s);
		sol::table point_table = lua.create_table();

		// TODO: We have several APIs that need entity groups. This needs to be put
		// into it's own function.
		std::vector<std::string> entity_groups;
		for (auto& eg : entity_groups_to_check)
		{
				if (eg.second.get_type() == sol::type::string)
				{
						//std::cout << eg.second.as<std::string>() << std::endl;
						entity_groups.push_back(eg.second.as<std::string>());
				}
		}

		auto result = game->generate_random_point(entity_groups);

		point_table.set("x", result.x);
		point_table.set("y", result.y);

		return point_table;
}

bool is_tile_walkable(const std::shared_ptr<roguely::game::Game> &game, int x, int y, std::string direction, std::string who, sol::table entity_groups_to_check)
{
		std::vector<std::string> entity_groups;
		roguely::common::MovementDirection movement_direction{};
		roguely::common::WhoAmI who_am_i{};

		for (auto& eg : entity_groups_to_check)
				if (eg.second.get_type() == sol::type::string) entity_groups.push_back(eg.second.as<std::string>());

		if (direction == "up") { movement_direction = roguely::common::MovementDirection::Up; }
		else if (direction == "down") { movement_direction = roguely::common::MovementDirection::Down; }
		else if (direction == "left") { movement_direction = roguely::common::MovementDirection::Left; }
		else if (direction == "right") { movement_direction = roguely::common::MovementDirection::Right; }

		if (who == "player") { who_am_i = roguely::common::WhoAmI::Player; }
		else if (who == "enemy") { who_am_i = roguely::common::WhoAmI::Enemy; }

		auto result = game->is_tile_walkable(x, y, movement_direction, who_am_i, entity_groups);

		return result.walkable;
}

void emit_lua_update_for_entity(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s)
{
		sol::state_view lua(s);

		if (entity != nullptr) {
				auto lua_update = lua["_update"];

				if (lua_update.valid() && lua_update.get_type() == sol::type::function)
				{
						auto entity_info_table = convert_entity_to_lua_table(entity, lua.lua_state());
						lua_update("entity_event", entity_info_table);
				}
		}
}

void set_component_value(const std::shared_ptr<roguely::game::Game> &game, std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, sol::object value, sol::this_state s)
{
		std::shared_ptr<roguely::ecs::Entity> entity{};

		if (value.get_type() == sol::type::number)
		{
				entity = game->set_component_value(entity_group_name, entity_id, component_name, key, value.as<int>());
		}
		else if (value.get_type() == sol::type::table)
		{
				for (auto& kvp : value.as<sol::table>())
				{
						if (kvp.first.get_type() == sol::type::string && kvp.second.get_type() == sol::type::number)
						{
								std::pair p{ kvp.first.as<std::string>(), kvp.second.as<int>() };
								entity = game->set_component_value(entity_group_name, entity_id, component_name, key, p);
						}
				}
		}

		emit_lua_update_for_entity(entity, s);
}

void send_key_event(std::string key, sol::this_state s)
{
		sol::state_view lua(s);
		auto lua_update = lua["_update"];

		if (lua_update.valid() && lua_update.get_type() == sol::type::function)
		{
				sol::table key_event_table = lua.create_table();
				key_event_table.set("key", key);
				lua_update("key_event", key_event_table);
		}
}

void render(float delta_time, sol::this_state s)
{
		sol::state_view lua(s);
		auto lua_render = lua["_render"];

		if (lua_render.valid() && lua_render.get_type() == sol::type::function)
		{
				lua_render(delta_time);
		}
}

void tick(float delta_time, sol::this_state s)
{
		sol::state_view lua(s);
		auto lua_tick = lua["_tick"];

		if (lua_tick.valid() && lua_tick.get_type() == sol::type::function)
		{
				lua_tick(delta_time);
		}
}

sol::table get_entity_group_points(const std::shared_ptr<roguely::game::Game> &game, std::string entity_group_name, sol::this_state s)
{
		sol::state_view lua(s);
		sol::table entity_group_table = lua.create_table();

		auto entity_group = game->get_entity_group(entity_group_name);

		if (entity_group != nullptr)
		{
				for (auto& e : *entity_group->entities)
				{
						sol::table point_table = lua.create_table_with(
								"x", e->x(),
								"y", e->y()
						);
						entity_group_table.set(e->get_id(), point_table);
				}
		}

		return entity_group_table;
}

void render_graphic(SDL_Renderer* renderer, std::string path, int window_width, int x, int y, bool centered, bool scaled, float scaled_factor)
{
		auto graphic = IMG_Load(path.c_str());
		auto graphic_texture = SDL_CreateTextureFromSurface(renderer, graphic);

		SDL_Rect dest = { x, y, graphic->w, graphic->h };

		if (centered)
				dest = { ((window_width / (2 + (int)scaled_factor)) - (graphic->w / 2)), y, graphic->w, graphic->h };

		SDL_Rect src = { 0, 0, graphic->w, graphic->h };

		if (scaled)
		{
				SDL_RenderSetScale(renderer, scaled_factor, scaled_factor);
				SDL_RenderCopy(renderer, graphic_texture, &src, &dest);
				SDL_RenderSetScale(renderer, 1, 1);
		}
		else
		{
				SDL_RenderCopy(renderer, graphic_texture, &src, &dest);
		}

		SDL_FreeSurface(graphic);
		SDL_DestroyTexture(graphic_texture);
}

int main(int argc, char* argv[])
{		
		// Timer code from : https://gamedev.stackexchange.com/a/163508/18014
		const int UPDATE_FREQUENCY{ 30 };
		const float CYCLE_TIME{ 1.0f / UPDATE_FREQUENCY };
		float accumulated_seconds{ 0.0f };
		static roguely::common::Timer system_timer;
		static roguely::common::Timer animation_timer;
		static roguely::common::Timer logic_timer;

		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::debug);
		
		auto game_script = lua.load_file("game.lua");

		if (game_script.valid())
		{
				sol::protected_function result = game_script();
				result.set_default_handler(lua["_error"]);

				auto game_config = lua.get<sol::table>("Game");

				if (!game_config.valid() && !check_game_config(game_config))
				{
						std::cout << "Game script does not define the 'Game' configuration table." << std::endl;
						return -1;
				}

				if (init_sdl(game_config) < 0)
						return -1;

				init_game(game_config);

				lua["get_test_map"] = get_test_map;
				
				lua.set_function("get_sprite_info", [&](std::string sprite_sheet_name, sol::this_state s) {
						return get_sprite_info(game, sprite_sheet_name, s);
						});

				lua.set_function("draw_text", [&](std::string t, std::string size, int x, int y) {
						game->draw_text(renderer, t, size, x, y);
						});

				lua.set_function("draw_text_with_color", [&](std::string t, std::string size, int x, int y, int r, int g, int b, int a) {
						game->draw_text(renderer, t, size, x, y, r, g, b, a);
						});

				lua.set_function("get_text_extents", [&](std::string t, std::string size, sol::this_state s) {
						auto extents = game->get_text_extents(t.c_str(), size);
						sol::table extents_table = lua.create_table();
						extents_table.set("width", extents.width);
						extents_table.set("height", extents.height);
						return extents_table;
						});

				lua.set_function("add_sprite_sheet", [&](std::string name, std::string path, int sw, int sh, sol::this_state s) {
						return add_sprite_sheet(renderer, game, name, path, sw, sh, s);
						});

				lua.set_function("draw_sprite", [&](std::string name, int sprite_id, int x, int y) {
						game->draw_sprite(renderer, name, sprite_id, x, y, 0, 0);
						});

				lua.set_function("draw_sprite_scaled", [&](std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height) {
						game->draw_sprite(renderer, name, sprite_id, x, y, scaled_width, scaled_height);
						});

				lua.set_function("play_sound", [&](std::string name) {
						game->play_sound(name);
						});

				lua.set_function("generate_map", [&](std::string name, int map_width, int map_height) {
						game->generate_map(name, map_width, map_height);
						});

				lua.set_function("add_entity", [&](std::string entity_group, std::string entity_type, int x, int y, sol::table components_table, sol::this_state s) {
						auto id = add_entity(game, entity_group, entity_type, x, y, components_table);
						emit_lua_update_for_entity_group(entity_group, id, s);
						});

				lua.set_function("add_entities", [&](std::string entity_group_name, std::string entity_type, sol::table components_table, int num, sol::this_state s) {
						// FIXME: this relies on a map to be generated so we can generate x,y's that are not on walls. 
						// If script writer tries to do this before a map has been generated then we will crash.
						// Need to handle this!

						return add_entities(game, entity_group_name, entity_type, components_table, num, s);
						});

				lua.set_function("remove_entity", [&](std::string entity_group_name, std::string entity_id, sol::this_state s) {
						remove_entity(game, entity_group_name, entity_id, s);
						});

				lua.set_function("get_component_value", [&](std::string entity_group_name, std::string entity_id, std::string component_name, std::string key) {
						return game->get_component_value(entity_group_name, entity_id, component_name, key);
						});

				lua.set_function("set_component_value", [&](std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, sol::object value, sol::this_state s) {
						set_component_value(game, entity_group_name, entity_id, component_name, key, value, s);
						});

				lua.set_function("switch_map", [&](std::string name) {
						game->switch_map(name);
						});

				lua.set_function("get_map", [&](std::string name, bool light, sol::this_state s) {
						return get_map(game, name, light, s);
						});

				lua.set_function("generate_random_point", [&](sol::table entity_groups_to_check, sol::this_state s) {
						return get_random_point(game, entity_groups_to_check, s);
						});

				lua.set_function("is_tile_walkable", [&](int x, int y, std::string direction, std::string who, sol::table entity_groups_to_check) {
						return is_tile_walkable(game, x, y, direction, who, entity_groups_to_check);
						});

				lua.set_function("set_draw_color", [&](int r, int g, int b, int a) {
						set_draw_color(renderer, r, g, b, a);
						});

				lua.set_function("draw_point", [&](int x, int y) {
						draw_point(renderer, x, y);
						});

				lua.set_function("draw_rect", [&](int x, int y, int w, int h) {
						draw_rect(renderer, x, y, w, h);
						});

				lua.set_function("draw_filled_rect", [&](int x, int y, int w, int h) {
						draw_filled_rect(renderer, x, y, w, h);
						});

				lua.set_function("update_entity_position", [&](std::string entity_group_name, std::string entity_id, int x, int y, sol::this_state s) {
						auto entity = game->update_entity_position(entity_group_name, entity_id, x, y);
						if (entity != nullptr)
								if (entity_id == "player") game->rb_fov();

								return emit_lua_update_for_entity(entity, s);
						});

				lua.set_function("get_view_port_x", [&]() {
						return game->get_view_port_x();
						});

				lua.set_function("get_view_port_y", [&]() {
						return game->get_view_port_y();
						});

				lua.set_function("get_view_port_width", [&]() {
						return game->get_view_port_width();
						});

				lua.set_function("get_view_port_height", [&]() {
						return game->get_view_port_height();
						});

				lua.set_function("update_player_viewport_points", [&]() {
						game->update_player_viewport_points();
						});

				lua.set_function("fov", [&](std::string name, sol::this_state s) {
						//game->rb_fov();
						lua["_update"]("light_map", get_map(game, name, true, s));
						});

				lua.set_function("get_entity_group_points", [&](std::string entity_group_name, sol::this_state s) {
						return get_entity_group_points(game, entity_group_name, s);
						});

				lua.set_function("draw_graphic", [&](std::string path, int window_width, int x, int y, bool centered, bool scaled, float scaled_factor) {
						render_graphic(renderer, path, window_width, x, y, centered, scaled, scaled_factor);
						});

				lua.set_function("reset", [&](sol::this_state s) {
						sol::state_view lua(s);
						auto lua_init = lua["_init"];

						if (lua_init.valid() && lua_init.get_type() == sol::type::function)
						{
								game->reset(true);
								
								lua_init();
						}
						});

				lua.set_function("get_random_number", [&](int start, int end) {
						return std::rand() % end + start;
						});
				
				lua.set_function("generate_uuid", [&]() {
						return game->generate_uuid();
						});

				auto lua_init = lua["_init"];

				if (lua_init.valid() && lua_init.get_type() == sol::type::function) lua_init();
								
				bool keep_window_open = true;
				while (keep_window_open)
				{
						system_timer.tick();
						accumulated_seconds += system_timer.elapsed_seconds;

						SDL_Event e;
						while (SDL_PollEvent(&e) > 0) 
						{
								switch (e.type)
								{
								case SDL_QUIT:
										keep_window_open = false;
										break;
								case SDL_KEYDOWN:
										if (e.key.keysym.sym == SDLK_UP ||
												e.key.keysym.sym == SDLK_w)
										{
												send_key_event("up", lua.lua_state());
										}
										else if (e.key.keysym.sym == SDLK_DOWN ||
												e.key.keysym.sym == SDLK_s)
										{
												send_key_event("down", lua.lua_state());
										}
										else if (e.key.keysym.sym == SDLK_LEFT ||
												e.key.keysym.sym == SDLK_a)
										{
												send_key_event("left", lua.lua_state());
										}
										else if (e.key.keysym.sym == SDLK_RIGHT ||
												e.key.keysym.sym == SDLK_d)
										{
												send_key_event("right", lua.lua_state());
										}
										else if (e.key.keysym.sym == SDLK_SPACE)
										{
												send_key_event("space", lua.lua_state());
										}
										break;
								}
						}

						if (std::isgreater(accumulated_seconds, CYCLE_TIME))
						{
								accumulated_seconds = -CYCLE_TIME;
								animation_timer.tick();
								logic_timer.tick();

								SDL_RenderClear(renderer);

								render(animation_timer.elapsed_seconds, lua.lua_state());
								tick(logic_timer.elapsed_seconds, lua.lua_state());

								SDL_RenderPresent(renderer);
						}
				}

				tear_down_sdl(game_config);
		}
		else
		{
				sol::error err = game_script;
				std::cout << "Lua script error: "
						<< "\n\t" << err.what() << std::endl;
		}

		return 0;
}