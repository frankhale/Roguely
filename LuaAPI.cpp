/*
* LuaAPI.cpp
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

#include "LuaAPI.h"

sol::table get_sprite_info(std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets, std::string name, sol::this_state s)
{
		if (!(name.length() > 0)) return nullptr;

		auto sheet = std::find_if(sprite_sheets->begin(), sprite_sheets->end(),
				[&](const std::shared_ptr<roguely::sprites::SpriteSheet>& ss) {
						return ss->get_name() == name;
				});

		if (sheet != sprite_sheets->end())
		{
				return (*sheet)->get_sprites_as_lua_table(s);
		}

		return nullptr;
}

sol::table add_sprite_sheet(SDL_Renderer* renderer, std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets, std::string name, std::string path, int sw, int sh, sol::this_state s)
{
		if (!(name.length() > 0 && path.length() > 0)) return nullptr;

		auto ss = std::make_shared<roguely::sprites::SpriteSheet>(renderer, name, path, sw, sh);
		sprite_sheets->emplace_back(ss);

		return get_sprite_info(sprite_sheets, name, s);
}

void draw_sprite(SDL_Renderer* renderer, std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets, std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height)
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

void draw_text(SDL_Renderer* renderer, std::shared_ptr<roguely::common::Text> text, std::string t, int x, int y)
{
		if (!(t.length() > 0)) return;

		text->draw_text(renderer, x, y, t.c_str());
}

void play_sound(std::shared_ptr<std::vector<std::shared_ptr<roguely::common::Sound>>> sounds, std::string name)
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
						else if (c->get_component_name() == "lua_component")
						{
								auto lc = std::static_pointer_cast<roguely::ecs::LuaComponent>(c);
								if (lc != nullptr)
								{
										entity_info_table[entity_type]["components"][lc->get_type()] = lua.create_table();
										entity_info_table[entity_type]["components"][lc->get_type()][lc->get_name()] = lc->get_properties();
								}
						}
				}
				});

		return entity_info_table;
}

sol::table convert_entity_group_to_lua_table(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, sol::this_state s)
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

std::string add_entity(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_type, int x, int y, sol::table components_table)
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

		//std::cout << "entityGroup = " << entityGroup
		//		<< " | entityType = " << entityType
		//		<< " | x = " << x
		//		<< " | y = " << y
		//		<< std::endl;

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
										else if (key == "lua_component")
										{
												for (auto& cc : value_table)
												{
														std::string name{};
														std::string type{};
														sol::table props{};

														// FIXME: Need to check to make sure the incoming types are what we expect

														if (cc.first.as<std::string>() == "name")
														{
																name = cc.second.as<std::string>();
														}
														else if (cc.first.as<std::string>() == "type")
														{
																type = cc.second.as<std::string>();
														}
														else if (cc.first.as<std::string>() == "properties")
														{
																props = cc.second.as<sol::table>();
														}

														if (name.size() > 0 && type.size() > 0)
														{
																game->add_lua_component(entity, name, type, props);
														}
												}
										}
								}
						}
				}
		}

		return e_id;
}

std::string add_entity(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_type, sol::table components_table)
{
		auto entity_groups = game->get_entity_group_names();
		auto random_point = game->generate_random_point(entity_groups);
		return add_entity(game, entity_group_name, entity_type, random_point.x, random_point.y, components_table);
}

sol::table add_entities(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_type, sol::table components_table, int num, sol::this_state s)
{
		sol::state_view lua(s);

		for (int i = 0; i < num; i++)
				add_entity(game, entity_group_name, entity_type, components_table);

		return convert_entity_group_to_lua_table(game, entity_group_name, lua.lua_state());
}

void remove_entity(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_id, sol::this_state s)
{
		sol::state_view lua(s);
		auto result = game->remove_entity(entity_group_name, entity_id);

		if (result)
		{
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

sol::table get_map(std::shared_ptr<roguely::game::Game> game, std::string name, bool light, sol::this_state s)
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

sol::table get_random_point(std::shared_ptr<roguely::game::Game> game, sol::table entity_groups_to_check, sol::this_state s)
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

bool is_tile_walkable(std::shared_ptr<roguely::game::Game> game, int x, int y, std::string direction, std::string who, sol::table entity_groups_to_check)
{
		std::vector<std::string> entity_groups;
		roguely::common::MovementDirection movement_direction{};
		roguely::common::WhoAmI who_am_i{};

		for (auto& eg : entity_groups_to_check)
		{
				if (eg.second.get_type() == sol::type::string)
				{
						//std::cout << "is_tile_walkable" << std::endl;
						//std::cout << eg.second.as<std::string>() << std::endl;
						entity_groups.push_back(eg.second.as<std::string>());
				}
		}

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

void set_component_value(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, sol::object value, sol::this_state s)
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

sol::table get_entity_group_points(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, sol::this_state s)
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