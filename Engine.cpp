/*
* engine.cpp
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

#include "Engine.h"

namespace roguely::sprites
{
		SpriteSheet::SpriteSheet(SDL_Renderer* renderer, std::string n, std::string p, int sw, int sh)
		{
				path = p;
				name = n;
				sprite_width = sw;
				sprite_height = sh;

				auto tileset = IMG_Load(p.c_str());
				auto t_color = SDL_MapRGB(tileset->format, 0, 0, 0);
				SDL_SetColorKey(tileset, SDL_TRUE, t_color);
				spritesheet_texture = SDL_CreateTextureFromSurface(renderer, tileset);
				int total_sprites_on_sheet = tileset->w / sw * tileset->h / sh;
				sprites = std::make_unique<std::vector<std::shared_ptr<SDL_Rect>>>(0);

				for (int y = 0; y < total_sprites_on_sheet / (sh / 2); y++)
				{
						for (int x = 0; x < total_sprites_on_sheet / (sw / 2); x++)
						{
								SDL_Rect rect = { x * sw, y * sh, sw, sh };
								auto r = std::make_shared<SDL_Rect>(rect);
								sprites->emplace_back(r);
						}
				}

				SDL_FreeSurface(tileset);
		}

		void SpriteSheet::draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y)
		{
				draw_sprite(renderer, sprite_id, x, y, 0, 0);
		}

		void SpriteSheet::draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y, int scaled_width, int scaled_height)
		{
				if (sprite_id < 0 || sprite_id > sprites->capacity()) return;

				int width = sprite_width;
				int height = sprite_height;

				if (scaled_width > 0 && scaled_height > 0)
				{
						width = scaled_width;
						height = scaled_height;
				}

				SDL_Rect dest = { x, y, width, height };
				auto sprite_rect = sprites->at(sprite_id);
				SDL_RenderCopy(renderer, spritesheet_texture, &(*sprite_rect), &dest);
		}

		sol::table SpriteSheet::get_sprites_as_lua_table(sol::this_state s)
		{
				sol::state_view lua(s);
				sol::table sprites_table = lua.create_table();

				//std::cout << "getting info for: " << name << std::endl;
				//std::cout << "number of sprites: " << sprites->size() << std::endl;

				for (std::size_t i = 0, sp = sprites->size(); i != sp; ++i)
				{
						auto sprite_rect = sprites->at(i);

						sol::table rect_table = lua.create_table();

						rect_table.set("x", sprite_rect->x);
						rect_table.set("y", sprite_rect->y);
						rect_table.set("w", sprite_rect->w);
						rect_table.set("h", sprite_rect->h);

						sprites_table.set(i, rect_table);

						//std::cout << i << " - " <<
						//		" x = " << sprite_rect->x <<
						//		" y = " << sprite_rect->x <<
						//		" w = " << sprite_rect->w <<
						//		" h = " << sprite_rect->h << std::endl;
				}

				return sprites_table;
		}
}

namespace roguely::level_generation
{
		// Quick and dirty cellular automata that I learned about from YouTube
		// We can do more but currently are just doing the very least to get a 
		// playable level.

		int get_neighbor_wall_count(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int x, int y)
		{
				int wall_count = 0;

				for (int row = y - 1; row <= y + 1; row++)
				{
						for (int col = x - 1; col <= x + 1; col++)
						{
								if (row >= 1 && col >= 1 && row < map_height - 1 && col < map_width - 1)
								{
										if ((*map)(row, col) == 0)
												wall_count++;
								}
								else
								{
										wall_count++;
								}
						}
				}

				return wall_count;
		}

		void perform_cellular_automaton(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int passes)
		{
				for (int p = 0; p < passes; p++)
				{
						auto temp_map = std::make_shared<boost::numeric::ublas::matrix<int>>() = map;

						for (int rows = 0; rows < map_height; rows++)
						{
								for (int columns = 0; columns < map_width; columns++)
								{
										auto neighbor_wall_count = get_neighbor_wall_count(temp_map, map_width, map_height, columns, rows);

										if (neighbor_wall_count > 4)
												(*map)(rows, columns) = 0;
										else
												(*map)(rows, columns) = 9;
								}
						}
				}
		}

		std::shared_ptr<boost::numeric::ublas::matrix<int>> init_cellular_automata(int map_width, int map_height)
		{
				auto map = std::make_shared<boost::numeric::ublas::matrix<int>>(map_height, map_width);

				for (int r = 0; r < map_height; ++r)
				{
						for (int c = 0; c < map_width; ++c)
						{
								auto z = std::rand() % 100 + 1;
								if (z > 48)
										(*map)(r, c) = 9;
								else
										(*map)(r, c) = 0;
						}
				}

				return map;
		}
}

namespace roguely::common
{
		Text::Text()
		{
				font = nullptr;
				text_texture = nullptr;
		}

		int Text::load_font(const char* path, int ptsize)
		{
				font = TTF_OpenFont(path, ptsize);

				if (!font) {
						std::cout << "Unable to load font: " << path << std::endl
								<< "SDL2_ttf Error: " << TTF_GetError() << std::endl;
						return -1;
				}

				return 0;
		}

		TextExtents Text::get_text_extents(const char* text)
		{
				int w{}, h{};

				if (TTF_SizeText(font, text, &w, &h) == 0) {
						return { w, h };
				}

				return {};
		}

		void Text::draw_text(SDL_Renderer* renderer, int x, int y, const char* text)
		{
				draw_text(renderer, x, y, text, text_color);
		}

		void Text::draw_text(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color)
		{
				if (strlen(text) <= 0) return;

				text_texture = nullptr;

				SDL_Rect text_rect;
				SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, color);
				text_rect.x = x;
				text_rect.y = y;
				text_rect.w = text_surface->w;
				text_rect.h = text_surface->h;

				text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
				SDL_FreeSurface(text_surface);

				SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
				SDL_DestroyTexture(text_texture);
		}

		std::shared_ptr<std::queue<Point>> AStarPathFinder::find_path(Point start, Point end, int walkable_tile_id)
		{
				auto path = std::make_shared<std::queue<Point>>();

				AStarNode none{};

				auto start_node = std::make_shared<AStarNode>(none, start);
				auto end_node = std::make_shared<AStarNode>(none, end);
				auto open_list = std::make_shared<std::vector<std::shared_ptr<AStarNode>>>();
				auto closed_list = std::make_shared<std::vector<std::shared_ptr<AStarNode>>>();

				open_list->emplace_back(start_node);

				while (open_list->size() > 0)
				{
						auto current_node = open_list->front();
						int current_index = 0;

						int _index = 0;
						for (const auto& item : *open_list)
						{
								if (item->f < current_node->f)
								{
										current_node = item;
										current_index = _index;
								}
								_index++;
						}

						open_list->erase(open_list->begin() + current_index);
						closed_list->emplace_back(current_node);

						if (current_node->eq(*end_node))
						{
								auto current = current_node;
								while (current != nullptr && current->position != nullptr)
								{
										Point path_point = { current->position->x, current->position->y };
										path->push(path_point);
										current = current->parent;
								}

								return path;
						}

						auto children = std::make_shared<std::vector<std::shared_ptr<AStarNode>>>();

						for (const auto& new_position : pos_array)
						{
								auto node_position = std::make_shared<Point>(current_node->position->x + new_position.x, current_node->position->y + new_position.y);

								if (node_position->x > (map->size2() - 1) || node_position->x < 0 ||
										node_position->y >(map->size1() - 1) || node_position->y < 0) continue;

								if ((*map)(node_position->y, node_position->x) != walkable_tile_id) continue;

								auto child = std::make_shared<AStarNode>(*current_node, *node_position);
								children->emplace_back(child);
						}

						for (const auto& child : *children)
						{
								auto closed_list_result = std::find_if(closed_list->begin(), closed_list->end(),
										[&](const std::shared_ptr<AStarNode>& c) {
												return c->eq(*child);
										});

								if (closed_list_result != closed_list->end() && *closed_list_result != nullptr)
										continue;

								child->g = current_node->g + 1;
								child->h = (int)pow(child->position->x - end_node->position->x, 2) + (int)pow(child->position->y - end_node->position->y, 2);
								child->f = child->g + child->h;

								auto open_node_result = std::find_if(open_list->begin(), open_list->end(),
										[&](const std::shared_ptr<AStarNode>& o) {
												return child->eq(*o) && child->g >= o->g;
										});

								if (open_node_result != open_list->end() && *open_node_result != nullptr)
										continue;

								open_list->emplace_back(child);
						}
				}

				return path;
		}
}

namespace roguely::ecs
{
		void InventoryComponent::upsert_item(std::pair<std::string, int> item_key_value_pair)
		{
				auto item = std::find_if(inventory->begin(), inventory->end(),
						[&](const std::shared_ptr<std::pair<std::string, int>>& i) {
								return (*i).first == item_key_value_pair.first;
						});

				if (item != inventory->end())
				{
						(*item)->second = item_key_value_pair.second;
				}
				else
				{
						add_item(item_key_value_pair.first, item_key_value_pair.second);
				}
		}

		void InventoryComponent::add_item(std::string name, int count)
		{
				std::pair item{ name, count };
				auto p_item = std::make_shared<std::pair<std::string, int>>(item);
				inventory->push_back(p_item);
		};

		void InventoryComponent::remove_item(std::string name, int count)
		{
				auto item = std::find_if(inventory->begin(), inventory->end(),
						[&](const std::shared_ptr<std::pair<std::string, int>>& i) {
								return (*i).first == name;
						});

				if (item != inventory->end())
				{
						(*item)->second -= count;

						if ((*item)->second <= 0) {
								inventory->erase(std::remove_if(inventory->begin(), inventory->end(),
										[&](const std::shared_ptr < std::pair<std::string, int>> i) {
												return (*i).first == name;
										}),
										inventory->end());
						}
				}
		};

		std::shared_ptr<Component> Entity::find_component_by_name(std::string name) const {
				auto component = std::find_if(components->begin(), components->end(),
						[&](const std::shared_ptr<roguely::ecs::Component>& x) {
								return x->get_component_name() == name;
						});

				if (component != components->end())
				{
						return *component;
				}

				return nullptr;
		}

		template<typename T>
		std::shared_ptr<T> Entity::find_component_by_type()
		{
				auto found_component = std::find_if(components->begin(), components->end(),
						[](const auto& c) {
								return std::dynamic_pointer_cast<T>(c) != nullptr;
						});

				std::shared_ptr<T> t_component = nullptr;

				if (found_component != components->end())
				{
						t_component = std::dynamic_pointer_cast<T>(*found_component);

						if (t_component != nullptr)
						{
								return t_component;
						}
				}

				return nullptr;
		}

		void EntityManager::add_sprite_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string spritesheet_name, int sprite_in_spritesheet_id, std::string sprite_name)
		{
				auto sprite_component = std::make_shared<roguely::ecs::SpriteComponent>(spritesheet_name, sprite_in_spritesheet_id, sprite_name);
				sprite_component->set_component_name("sprite_component");
				entity->add_component(sprite_component);
		}

		void EntityManager::add_health_component(std::shared_ptr<roguely::ecs::Entity> entity, int h)
		{
				auto health_component = std::make_shared<roguely::ecs::HealthComponent>(h);
				health_component->set_component_name("health_component");
				entity->add_component(health_component);
		}

		void EntityManager::add_stats_component(std::shared_ptr<roguely::ecs::Entity> entity, int a)
		{
				auto stats_component = std::make_shared<roguely::ecs::StatsComponent>(a);
				stats_component->set_component_name("stats_component");
				entity->add_component(stats_component);
		}

		void EntityManager::add_score_component(std::shared_ptr<roguely::ecs::Entity> entity, int s)
		{
				auto score_component = std::make_shared<roguely::ecs::ScoreComponent>(s);
				score_component->set_component_name("score_component");
				entity->add_component(score_component);
		}

		void EntityManager::add_value_component(std::shared_ptr<roguely::ecs::Entity> entity, int v)
		{
				auto value_component = std::make_shared<roguely::ecs::ValueComponent>(v);
				value_component->set_component_name("value_component");
				entity->add_component(value_component);
		}

		void EntityManager::add_inventory_component(std::shared_ptr<roguely::ecs::Entity> entity, std::vector<std::pair<std::string, int>> items)
		{
				auto inventory_component = std::make_shared<roguely::ecs::InventoryComponent>();

				for (auto& item : items)
				{
						inventory_component->add_item(item.first, item.second);
				}

				inventory_component->set_component_name("inventory_component");
				entity->add_component(inventory_component);
		}

		void EntityManager::add_lua_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string n, std::string t, sol::table props, sol::this_state s)
		{
				sol::state_view lua(s);
				auto lua_component = std::make_shared<roguely::ecs::LuaComponent>(n, t, props, lua.lua_state());
				lua_component->set_component_name(n);
				entity->add_component(lua_component);
		}

		std::shared_ptr<roguely::ecs::EntityGroup> EntityManager::create_entity_group(std::string name)
		{
				auto entityGroup = std::make_shared<roguely::ecs::EntityGroup>();
				entityGroup->name = name;
				entityGroup->entities = std::make_shared<std::vector<std::shared_ptr<roguely::ecs::Entity>>>();
				entity_groups->emplace_back(entityGroup);
				return entityGroup;
		}

		std::string EntityManager::add_entity(std::string entity_group_name, std::string entity_type, int x, int y, sol::table components_table, sol::this_state s)
		{
				sol::state_view lua(s);
				auto e_id = generate_uuid();
				roguely::common::Point e_p = { x, y };
				roguely::ecs::EntityType e_type{};

				if (entity_type == "player") e_type = roguely::ecs::EntityType::Player;
				else if (entity_type == "enemy") e_type = roguely::ecs::EntityType::Enemy;
				else if (entity_type == "npc") e_type = roguely::ecs::EntityType::NPC;
				else if (entity_type == "item") e_type = roguely::ecs::EntityType::Item;
				else if (entity_type == "interactable") e_type = roguely::ecs::EntityType::Interactable;
				else if (entity_type == "ground") e_type = roguely::ecs::EntityType::Ground;
				else if (entity_type == "wall") e_type = roguely::ecs::EntityType::Wall;

				auto entity_group = get_entity_group(entity_group_name);

				if (entity_group == nullptr)
				{
						entity_group = create_entity_group(entity_group_name);
				}

				if (entity_group != nullptr) {
						auto entity = add_entity_to_group(entity_group, e_type, e_id, e_p);

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
																				add_value_component(entity, value);
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
																		add_sprite_component(entity, spritesheet_name, sprite_id, sprite_name);
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
																				add_health_component(entity, health);
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
																				add_stats_component(entity, attack);
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
																				add_score_component(entity, score);
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

																				add_inventory_component(entity, items);
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
																add_lua_component(entity, key, type, props, lua.lua_state());
														}
												}
										}
								}
						}
				}

				return e_id;
		}

		std::string EntityManager::add_entity(std::string entity_group_name, std::string entity_type, sol::table components_table, std::function<roguely::common::Point()> get_random_point, sol::this_state s)
		{
				sol::state_view lua(s);
				auto random_point = get_random_point();
				return add_entity(entity_group_name, entity_type, random_point.x, random_point.y, components_table, lua.lua_state());
		}

		sol::table EntityManager::add_entities(std::string entity_group_name, std::string entity_type, sol::table components_table, int num, std::function<roguely::common::Point()> get_random_point, sol::this_state s)
		{
				sol::state_view lua(s);

				for (int i = 0; i < num; i++) {
						auto random_point = get_random_point();
						add_entity(entity_group_name, entity_type, random_point.x, random_point.y, components_table, lua.lua_state());
				}

				return convert_entity_group_to_lua_table(entity_group_name, lua.lua_state());
		}

		std::shared_ptr<roguely::ecs::Entity> EntityManager::add_entity_to_group(std::shared_ptr<roguely::ecs::EntityGroup> entity_group, roguely::ecs::EntityType entity_type, std::string id, roguely::common::Point point)
		{
				auto entity = std::make_shared<roguely::ecs::Entity>(entity_group, id, point, entity_type);

				if (entity_type == roguely::ecs::EntityType::Player)
				{
						player_id = id;
						player = entity;
				}

				entity_group->entities->emplace_back(entity);

				return entity;
		}

		bool EntityManager::remove_entity(std::string entity_group_name, std::string entity_id)
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

		void EntityManager::remove_entity(std::string entity_group_name, std::string entity_id, sol::this_state s)
		{
				auto result = remove_entity(entity_group_name, entity_id);

				if (result)
						emit_lua_update_for_entity_group(entity_group_name, entity_id, s);
		}

		std::shared_ptr<roguely::ecs::Entity> EntityManager::update_entity_position(std::string entity_group_name, std::string entity_id, int x, int y)
		{
				std::shared_ptr<roguely::ecs::Entity> entity{};

				if (entity_id == "player" || player->get_id() == entity_id)
				{
						player->set_point({ x, y });
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

		void EntityManager::update_entities(std::string entity_group_name, std::string component_name, std::string key, sol::object value, sol::this_state s)
		{
				sol::state_view lua(s);
				auto entity_group = get_entity_group(entity_group_name);

				if (entity_group != nullptr)
				{
						for (const auto& entity : *entity_group->entities)
						{
								auto component = entity->find_component_by_name(component_name);

								if (value.get_type() == sol::type::number)
								{
										int val = value.as<int>();

										if (component != nullptr)
												set_component_value(component, key, val, lua.lua_state());
								}
						}
				}
		}

		void EntityManager::update_entity_position(std::string entity_group_name, sol::table entity_positions)
		{
				auto entity_group = get_entity_group(entity_group_name);

				if (entity_group != nullptr)
				{
						for (auto& c : entity_positions)
						{
								std::string key = c.first.as<std::string>();
								sol::table value_table = c.second.as<sol::table>();

								if (value_table.valid() && key.size() > 0)
								{
										int x = -1;
										int y = -1;

										// find the entity in the group (key is the id of the entity)
										for (auto& cc : value_table)
										{
												if (cc.first.get_type() == sol::type::string && cc.second.get_type() == sol::type::number)
												{
														if (cc.first.as<std::string>() == "x") x = cc.second.as<int>();
														else if (cc.first.as<std::string>() == "y") y = cc.second.as<int>();
												}
										}

										if (x > -1 && y > -1)
										{
												// update the entities x, y position
												auto entity = get_entity(entity_group, key);

												if (entity != nullptr)
														entity->set_point({ x, y });
										}
								}
						}
				}
		}

		std::shared_ptr<roguely::ecs::EntityGroup> EntityManager::get_entity_group(std::string name)
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

		std::shared_ptr<roguely::ecs::Entity> EntityManager::get_entity(std::shared_ptr<roguely::ecs::EntityGroup> entity_group, std::string entity_id)
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

		int EntityManager::get_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key)
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

		int EntityManager::get_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key)
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

		sol::table EntityManager::get_entity_group_points(std::string entity_group_name, sol::this_state s)
		{
				sol::state_view lua(s);
				sol::table entity_group_table = lua.create_table();

				auto entity_group = get_entity_group(entity_group_name);

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

		bool EntityManager::set_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key, int value, sol::this_state s)
		{
				sol::state_view lua(s);
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
				else
				{
						auto lc = std::static_pointer_cast<roguely::ecs::LuaComponent>(component);
						if (lc != nullptr)
						{
								lc->set_property(key, value, lua.lua_state());
								did_update = true;
						}
				}

				return did_update;
		}

		std::shared_ptr<roguely::ecs::Entity> EntityManager::set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, int value, sol::this_state s)
		{
				sol::state_view lua(s);

				if (entity_id == "player" || player->get_id() == entity_id)
				{
						auto component = player->find_component_by_name(component_name);

						if (component != nullptr)
						{
								auto did_update = set_component_value(component, key, value, lua.lua_state());

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
												auto did_update = set_component_value(component, key, value, lua.lua_state());

												if (did_update)
														return entity;
										}
								}
						}
				}

				return nullptr;
		}

		std::shared_ptr<roguely::ecs::Entity> EntityManager::set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, std::pair<std::string, int> value, sol::this_state s)
		{
				sol::state_view lua(s);

				if (entity_id == "player" || player->get_id() == entity_id)
				{
						auto component = player->find_component_by_name(component_name);

						if (component != nullptr)
						{
								if (component_name == "inventory_component")
								{
										auto ic = std::static_pointer_cast<roguely::ecs::InventoryComponent>(component);
										if (ic != nullptr)
										{
												ic->upsert_item(value);
												return player;
										}
								}
								else
								{
										auto lc = std::static_pointer_cast<roguely::ecs::LuaComponent>(component);
										if (lc != nullptr)
										{
												lc->set_property(value.first, value.second, lua.lua_state());
												return player;
										}
								}
						}
				}

				return nullptr;
		}

		void EntityManager::set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, sol::object value, sol::this_state s)
		{
				sol::state_view lua(s);
				std::shared_ptr<roguely::ecs::Entity> entity{};

				if (value.get_type() == sol::type::number)
				{
						entity = set_component_value(entity_group_name, entity_id, component_name, key, value.as<int>(), lua.lua_state());
				}
				else if (value.get_type() == sol::type::table)
				{
						for (auto& kvp : value.as<sol::table>())
						{
								if (kvp.first.get_type() == sol::type::string && kvp.second.get_type() == sol::type::number)
								{
										std::pair p{ kvp.first.as<std::string>(), kvp.second.as<int>() };
										entity = set_component_value(entity_group_name, entity_id, component_name, key, p, lua.lua_state());
								}
						}
				}

				emit_lua_update_for_entity(entity, s);
		}

		// FIXME: Both emit_lua_update_for_entity_group's share similar code, need
		//			  to remove the code duplication.
		void EntityManager::emit_lua_update_for_entity_group(std::string entity_group_name, std::string entity_id, sol::this_state s)
		{
				sol::state_view lua(s);
				auto lua_update = lua["_update"];
				if (lua_update.valid() && lua_update.get_type() == sol::type::function)
				{
						sol::table data_table = lua.create_table();
						data_table.set("entity_group_name", entity_group_name);
						data_table.set("entity_id", entity_id);
						data_table.set("entity_group", convert_entity_group_to_lua_table(entity_group_name, lua.lua_state()));

						lua_update("entity_event", data_table);
				}
		}

		void EntityManager::emit_lua_update_for_entity_group(std::string entity_group_name, sol::this_state s)
		{
				sol::state_view lua(s);
				auto lua_update = lua["_update"];
				if (lua_update.valid() && lua_update.get_type() == sol::type::function)
				{
						sol::table data_table = lua.create_table();
						data_table.set("entity_group_name", entity_group_name);
						data_table.set("entity_group", convert_entity_group_to_lua_table(entity_group_name, lua.lua_state()));

						lua_update("entity_event", data_table);
				}
		}

		void EntityManager::emit_lua_update_for_entity(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s)
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

		sol::table EntityManager::convert_entity_to_lua_table(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s)
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

		sol::table EntityManager::convert_entity_group_to_lua_table(std::string entity_group_name, sol::this_state s)
		{
				sol::state_view lua(s);
				auto entity_group = get_entity_group(entity_group_name);

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

		bool EntityManager::is_xy_player_xy(int x, int y, roguely::common::MovementDirection dir)
		{
				return ((dir == roguely::common::MovementDirection::Up && player->y() == y - 1 && player->x() == x) ||
						(dir == roguely::common::MovementDirection::Down && player->y() == y + 1 && player->x() == x) ||
						(dir == roguely::common::MovementDirection::Left && player->y() == y && player->x() == x - 1) ||
						(dir == roguely::common::MovementDirection::Right && player->y() == y && player->x() == x + 1));
		}

		std::shared_ptr<roguely::ecs::TileWalkableInfo> EntityManager::is_entity_location_traversable(int x, int y, std::string entity_group_name, roguely::common::WhoAmI whoAmI, roguely::common::MovementDirection dir)
		{
				roguely::ecs::TileWalkableInfo twi{
								true,
								{ x, y },
								roguely::ecs::EntityType::Ground /* treat everything as ground if its traversable */
				};

				auto entity_group = get_entity_group(entity_group_name);

				if (entity_group != nullptr)
				{
						for (const auto& e : *entity_group->entities)
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
				}

				return std::make_shared<roguely::ecs::TileWalkableInfo>(twi);
		}

		std::string EntityManager::generate_uuid()
		{
				boost::uuids::random_generator gen;
				boost::uuids::uuid id = gen();
				return boost::uuids::to_string(id);
		}
}

namespace roguely::engine
{
		Engine::Engine()
		{
				std::srand(static_cast<unsigned int>(std::time(nullptr)));

				Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
				Mix_Volume(-1, 2);
				Mix_VolumeMusic(2);

				sprite_sheets = std::make_unique<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>>();
				sounds = std::make_unique<std::vector<std::shared_ptr<roguely::common::Sound>>>();

				reset(false);
		}

		void Engine::tear_down_sdl()
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

				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);

				Mix_Quit();
				TTF_Quit();
				IMG_Quit();
				SDL_Quit();
		}

		int Engine::init_sdl(sol::table game_config)
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
				text_medium = std::make_unique<roguely::common::Text>();
				text_medium->load_font(font_path.c_str(), 40);
				text_large = std::make_unique<roguely::common::Text>();
				text_large->load_font(font_path.c_str(), 63);
				text_small = std::make_unique<roguely::common::Text>();
				text_small->load_font(font_path.c_str(), 26);

				return 0;
		}

		void Engine::reset(bool reset_ptr) {
				if (reset_ptr) {
						maps.reset();
						entity_manager.reset();
				}

				maps = std::make_unique<std::vector<std::shared_ptr<roguely::common::Map>>>();
				entity_manager = std::make_unique<roguely::ecs::EntityManager>();
		}

		void Engine::update_player_viewport_points()
		{
				auto player_position = entity_manager->get_player_point();

				view_port_x = player_position.x - VIEW_PORT_WIDTH / 2;
				view_port_y = player_position.y - VIEW_PORT_HEIGHT / 2;

				if (view_port_x < 0) view_port_x = std::max(0, view_port_x);
				if (view_port_x > (current_map->width - VIEW_PORT_WIDTH)) view_port_x = (current_map->width - VIEW_PORT_WIDTH);

				if (view_port_y < 0) view_port_y = std::max(0, view_port_y);
				if (view_port_y > (current_map->height - VIEW_PORT_HEIGHT)) view_port_y = (current_map->height - VIEW_PORT_HEIGHT);

				view_port_width = view_port_x + VIEW_PORT_WIDTH;
				view_port_height = view_port_y + VIEW_PORT_HEIGHT;
		}

		bool Engine::is_tile_on_map_traversable(int x, int y, roguely::common::MovementDirection dir, int tileId)
		{
				if (current_map->map == nullptr) return false;

				bool result = true;

				if (dir == roguely::common::MovementDirection::Up && (*current_map->map)((size_t)y - 1, x) == tileId) result = false;
				else if (dir == roguely::common::MovementDirection::Down && (*current_map->map)((size_t)y + 1, x) == tileId) result = false;
				else if (dir == roguely::common::MovementDirection::Left && (*current_map->map)(y, (size_t)x - 1) == tileId) result = false;
				else if (dir == roguely::common::MovementDirection::Right && (*current_map->map)(y, (size_t)x + 1) == tileId) result = false;

				return result;
		}

		bool Engine::is_tile_player_tile(int x, int y, roguely::common::MovementDirection dir)
		{
				auto player_position = entity_manager->get_player_point();

				return ((dir == roguely::common::MovementDirection::Up && player_position.y == y - 1 && player_position.x == x) ||
						(dir == roguely::common::MovementDirection::Down && player_position.y == y + 1 && player_position.x == x) ||
						(dir == roguely::common::MovementDirection::Left && player_position.y == y && player_position.x == x - 1) ||
						(dir == roguely::common::MovementDirection::Right && player_position.y == y && player_position.x == x + 1));
		}

		bool Engine::is_xy_blocked(int x, int y)
		{
				if (current_map->map == nullptr) return true;
				if ((*current_map->map)(y, x) == 0) return true;
				
				auto player_position = entity_manager->get_player_point();
				if (player_position.x == x && player_position.y == y) return true;

				bool blocked = true;

				auto entity_groups_to_check = entity_manager->get_entity_group_names();

				for (auto& egtc : entity_groups_to_check)
				{						
						blocked = (entity_manager->is_entity_location_traversable(x, y, egtc)) ? false : true;

						if (blocked) break;
				}

				return blocked;
		}

		roguely::common::Point Engine::get_open_point_for_xy(int x, int y)
		{
				int left = x - 1;
				int right = x + 1;
				int up = y - 1;
				int down = y + 1;

				if (!is_xy_blocked(left, y)) return { left, y };
				else if (!is_xy_blocked(right, y)) return { right, y };
				else if (!is_xy_blocked(x, up)) return { x, up };
				else if (!is_xy_blocked(x, down)) return { x, down };

				return generate_random_point();
		}

		bool Engine::is_tile_walkable(int x, int y, std::string direction, std::string who, sol::table entity_groups_to_check)
		{
				bool result = false;
				std::vector<std::string> entity_group_names;
				roguely::common::MovementDirection movement_direction{};
				roguely::common::WhoAmI who_am_i{};

				for (auto& eg : entity_groups_to_check)
						if (eg.second.get_type() == sol::type::string) entity_group_names.push_back(eg.second.as<std::string>());

				if (direction == "up") { movement_direction = roguely::common::MovementDirection::Up; }
				else if (direction == "down") { movement_direction = roguely::common::MovementDirection::Down; }
				else if (direction == "left") { movement_direction = roguely::common::MovementDirection::Left; }
				else if (direction == "right") { movement_direction = roguely::common::MovementDirection::Right; }

				if (who == "player") { who_am_i = roguely::common::WhoAmI::Player; }
				else if (who == "enemy") { who_am_i = roguely::common::WhoAmI::Enemy; }

				if (!is_tile_on_map_traversable(x, y, movement_direction, 0 /* Wall */)) return false;

				// for enemy movement
				if (who_am_i != roguely::common::WhoAmI::Player && is_tile_player_tile(x, y, movement_direction)) return false;

				for (auto& egtc : entity_group_names)
				{
						result = entity_manager->is_entity_location_traversable(x, y, egtc, who_am_i, movement_direction)->walkable;
				}

				return result;
		}

		roguely::common::Point Engine::generate_random_point()
		{
				if (current_map->map == nullptr) return {};

				int c = 0;
				int r = 0;

				auto entity_groups = entity_manager->get_entity_group_names();

				do
				{
						c = std::rand() % (current_map->width - 1);
						r = std::rand() % (current_map->height - 1);
				} while (is_xy_blocked(c, r));

				return { c, r };
		}

		sol::table Engine::get_entity_group_points(std::string entity_group_name, sol::this_state s)
		{
				sol::state_view lua(s);
				sol::table entity_group_table = lua.create_table();

				auto entity_group = entity_manager->get_entity_group(entity_group_name);

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

		// Adapted from http://www.roguebasin.com/index.php?title=Eligloscode
		void Engine::rb_fov()
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

				auto player_position = entity_manager->get_player_point();

				for (int i = 0; i < 360; i++)
				{
						x = (float)std::cos(i * 0.01745f);
						y = (float)std::sin(i * 0.01745f);

						float ox = (float)player_position.x + 0.5f;
						float oy = (float)player_position.y + 0.5f;

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

		sol::table Engine::get_sprite_info(std::string name, sol::this_state s)
		{
				if (!(name.length() > 0)) return nullptr;

				auto sheet = find_spritesheet(name);

				if (sheet != nullptr)
						return sheet->get_sprites_as_lua_table(s);

				return nullptr;
		}

		sol::table Engine::add_sprite_sheet(SDL_Renderer* renderer, std::string name, std::string path, int sw, int sh, sol::this_state s)
		{
				sol::state_view lua(s);

				if (!(name.length() > 0 && path.length() > 0)) return nullptr;

				add_spritesheet(renderer, name, path, sw, sh);

				auto sheet = find_spritesheet(name);

				if (sheet != nullptr)
						return sheet->get_sprites_as_lua_table(lua.lua_state());

				return lua.create_table();
		}

		sol::table  Engine::get_map(std::string name, bool light, sol::this_state s)
		{
				sol::state_view lua(s);

				auto map = get_map(name);

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

		void  Engine::set_draw_color(SDL_Renderer* renderer, int r, int g, int b, int a)
		{
				SDL_SetRenderDrawColor(renderer, r, g, b, a);
		}

		void  Engine::draw_point(SDL_Renderer* renderer, int x, int y)
		{
				SDL_RenderDrawPoint(renderer, x, y);
		}

		void  Engine::draw_rect(SDL_Renderer* renderer, int x, int y, int w, int h)
		{
				SDL_Rect r = { x, y, w, h };
				SDL_RenderDrawRect(renderer, &r);
		}

		void  Engine::draw_filled_rect(SDL_Renderer* renderer, int x, int y, int w, int h)
		{
				SDL_Rect r = { x, y, w, h };
				SDL_RenderFillRect(renderer, &r);
		}

		sol::table Engine::get_random_point(sol::this_state s)
		{
				sol::state_view lua(s);
				sol::table point_table = lua.create_table();
				std::vector<std::string> entity_groups = entity_manager->get_entity_group_names();

				auto result = generate_random_point();

				point_table.set("x", result.x);
				point_table.set("y", result.y);

				return point_table;
		}

		sol::table Engine::get_open_point_for_xy(int x, int y, sol::this_state s)
		{
				sol::state_view lua(s);
				sol::table point_table = lua.create_table();

				auto result = get_open_point_for_xy(x, y);

				point_table.set("x", result.x);
				point_table.set("y", result.y);

				return point_table;
		}

		void  Engine::send_key_event(std::string key, sol::this_state s)
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

		void  Engine::render(float delta_time, sol::this_state s)
		{
				sol::state_view lua(s);
				auto lua_render = lua["_render"];

				if (lua_render.valid() && lua_render.get_type() == sol::type::function)
						lua_render(delta_time);
		}

		void  Engine::tick(float delta_time, sol::this_state s)
		{
				sol::state_view lua(s);
				auto lua_tick = lua["_tick"];

				if (lua_tick.valid() && lua_tick.get_type() == sol::type::function)
						lua_tick(delta_time);
		}

		void  Engine::render_graphic(SDL_Renderer* renderer, std::string path, int window_width, int x, int y, bool centered, bool scaled, float scaled_factor)
		{
				auto graphic = IMG_Load(path.c_str());
				auto graphic_texture = SDL_CreateTextureFromSurface(renderer, graphic);

				SDL_Rect src = { 0, 0, graphic->w, graphic->h };
				SDL_Rect dest = { x, y, graphic->w, graphic->h };

				if (centered)
						dest = { ((window_width / (2 + (int)scaled_factor)) - (graphic->w / 2)), y, graphic->w, graphic->h };

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

		void Engine::play_sound(std::string name)
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

		void Engine::add_spritesheet(SDL_Renderer* renderer, std::string name, std::string path, int sw, int sh)
		{
				auto ss = std::make_shared<roguely::sprites::SpriteSheet>(renderer, name, path, sw, sh);
				sprite_sheets->emplace_back(ss);
		}

		void Engine::draw_sprite(SDL_Renderer* renderer, std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height)
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

		void Engine::draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y)
		{
				draw_text(renderer, t, size, x, y, 255, 255, 255, 255);
		}

		void Engine::draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y, int r, int g, int b, int a)
		{
				if (!(t.length() > 0)) return;

				auto text = get_text_reference(size);

				SDL_Color text_color = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a };
				(*text)->draw_text(renderer, x, y, t.c_str(), text_color);
		}

		roguely::common::TextExtents Engine::get_text_extents(std::string t, std::string size)
		{
				auto text = get_text_reference(size);
				return (*text)->get_text_extents(t.c_str());
		}

		void Engine::switch_map(std::string name)
		{
				auto map = std::find_if(maps->begin(), maps->end(),
						[&](const std::shared_ptr<roguely::common::Map>& m) {
								return m->name == name;
						});

				if (map != maps->end()) {
						current_map = *map;
						path_finder = std::make_unique<roguely::common::AStarPathFinder>(current_map->map);
				}
		}

		std::shared_ptr<roguely::common::Map> Engine::get_map(std::string name)
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

		bool Engine::check_game_config(sol::table game_config, sol::this_state s)
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

		void Engine::setup_lua_helpers(sol::this_state s)
		{
				sol::state_view lua(s);

				lua.set_function("find_path", [&](int start_x, int start_y, int end_x, int end_y, int walkable_tile_id, sol::this_state s) {
						sol::state_view lua(s);
						auto path = path_finder->find_path({ start_x, start_y }, { end_x, end_y }, walkable_tile_id);

						sol::table path_table = lua.create_table();

						roguely::common::Point start{ start_x, start_y };
						roguely::common::Point end{ end_x, end_y };

						while (!path->empty())
						{
								roguely::common::Point p = path->front();

								if (!p.eq(start) && !p.eq(end))
								{
										path_table.set("x", p.x);
										path_table.set("y", p.y);
								}
								path->pop();
						}

						return path_table;
						});

				lua.set_function("get_sprite_info", [&](std::string sprite_sheet_name, sol::this_state s) {
						sol::state_view lua(s);
						return get_sprite_info(sprite_sheet_name, lua.lua_state());
						});

				lua.set_function("draw_text", [&](std::string t, std::string size, int x, int y) {
						draw_text(renderer, t, size, x, y);
						});

				lua.set_function("draw_text_with_color", [&](std::string t, std::string size, int x, int y, int r, int g, int b, int a) {
						draw_text(renderer, t, size, x, y, r, g, b, a);
						});

				lua.set_function("get_text_extents", [&](std::string t, std::string size, sol::this_state s) {
						sol::state_view lua(s);
						auto extents = get_text_extents(t.c_str(), size);
						sol::table extents_table = lua.create_table();
						extents_table.set("width", extents.width);
						extents_table.set("height", extents.height);
						return extents_table;
						});

				lua.set_function("add_sprite_sheet", [&](std::string name, std::string path, int sw, int sh, sol::this_state s) {
						sol::state_view lua(s);
						return add_sprite_sheet(renderer, name, path, sw, sh, lua.lua_state());
						});

				lua.set_function("draw_sprite", [&](std::string name, int sprite_id, int x, int y) {
						draw_sprite(renderer, name, sprite_id, x, y, 0, 0);
						});

				lua.set_function("draw_sprite_scaled", [&](std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height) {
						draw_sprite(renderer, name, sprite_id, x, y, scaled_width, scaled_height);
						});

				lua.set_function("play_sound", [&](std::string name) {
						play_sound(name);
						});

				lua.set_function("generate_map", [&](std::string name, int map_width, int map_height) {
						generate_map(name, map_width, map_height);
						});

				lua.set_function("add_entity", [&](std::string entity_group, std::string entity_type, int x, int y, sol::table components_table, sol::this_state s) {
						sol::state_view lua(s);
						auto id = entity_manager->add_entity(entity_group, entity_type, x, y, components_table, lua.lua_state());
						entity_manager->emit_lua_update_for_entity_group(entity_group, id, lua.lua_state());
						});

				lua.set_function("add_entities", [&](std::string entity_group_name, std::string entity_type, sol::table components_table, int num, sol::this_state s) {
						sol::state_view lua(s);
						return entity_manager->add_entities(entity_group_name, entity_type, components_table, num, [&]() {
								return generate_random_point();
								},
								lua.lua_state());
						});

				lua.set_function("remove_entity", [&](std::string entity_group_name, std::string entity_id, sol::this_state s) {
						sol::state_view lua(s);
						entity_manager->remove_entity(entity_group_name, entity_id, lua.lua_state());
						});

				lua.set_function("get_component_value", [&](std::string entity_group_name, std::string entity_id, std::string component_name, std::string key) {
						return entity_manager->get_component_value(entity_group_name, entity_id, component_name, key);
						});

				lua.set_function("set_component_value", [&](std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, sol::object value, sol::this_state s) {
						sol::state_view lua(s);
						entity_manager->set_component_value(entity_group_name, entity_id, component_name, key, value, lua.lua_state());
						});

				lua.set_function("switch_map", [&](std::string name) {
						switch_map(name);
						});

				lua.set_function("get_map", [&](std::string name, bool light, sol::this_state s) {
						sol::state_view lua(s);
						return get_map(name, light, lua.lua_state());
						});

				lua.set_function("generate_random_point", [&](sol::table entity_groups_to_check, sol::this_state s) {
						sol::state_view lua(s);
						return get_random_point(s);
						});

				lua.set_function("get_open_point_for_xy", [&](int x, int y, sol::table entity_groups_to_check, sol::this_state s) {
						sol::state_view lua(s);
						return get_open_point_for_xy(x, y, lua.lua_state());
						});

				lua.set_function("is_xy_blocked", [&](int x, int y) {
						return is_xy_blocked(x, y);
						});

				lua.set_function("is_tile_walkable", [&](int x, int y, std::string direction, std::string who, sol::table entity_groups_to_check) {
						return is_tile_walkable(x, y, direction, who, entity_groups_to_check);
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
						sol::state_view lua(s);
						auto entity = entity_manager->update_entity_position(entity_group_name, entity_id, x, y);
						update_player_viewport_points();
						rb_fov();
						if (entity != nullptr)
								if (entity_id == "player") rb_fov();

						return entity_manager->emit_lua_update_for_entity(entity, lua.lua_state());
						});

				lua.set_function("update_entities", [&](std::string entity_group_name, std::string component_name, std::string key, sol::object value, sol::this_state s) {
						sol::state_view lua(s);
						entity_manager->update_entities(entity_group_name, component_name, key, value, lua.lua_state());
						return entity_manager->emit_lua_update_for_entity_group(entity_group_name, lua.lua_state());
						});

				lua.set_function("update_entities_position", [&](std::string entity_group_name, sol::table entity_position_table, sol::this_state s) {
						sol::state_view lua(s);
						entity_manager->update_entity_position(entity_group_name, entity_position_table);
						return entity_manager->emit_lua_update_for_entity_group(entity_group_name, lua.lua_state());
						});

				lua.set_function("get_view_port_x", [&]() {
						return get_view_port_x();
						});

				lua.set_function("get_view_port_y", [&]() {
						return get_view_port_y();
						});

				lua.set_function("get_view_port_width", [&]() {
						return get_view_port_width();
						});

				lua.set_function("get_view_port_height", [&]() {
						return get_view_port_height();
						});

				lua.set_function("update_player_viewport_points", [&]() {
						update_player_viewport_points();
						});

				lua.set_function("fov", [&](std::string name, sol::this_state s) {
						sol::state_view lua(s);
						lua["_update"]("light_map", get_map(name, true, lua.lua_state()));
						});

				lua.set_function("get_entity_group_points", [&](std::string entity_group_name, sol::this_state s) {
						sol::state_view lua(s);
						return get_entity_group_points(entity_group_name, lua.lua_state());
						});

				lua.set_function("draw_graphic", [&](std::string path, int window_width, int x, int y, bool centered, bool scaled, float scaled_factor) {
						render_graphic(renderer, path, window_width, x, y, centered, scaled, scaled_factor);
						});

				lua.set_function("reset", [&](sol::this_state s) {
						sol::state_view lua(s);
						auto lua_init = lua["_init"];

						if (lua_init.valid() && lua_init.get_type() == sol::type::function)
						{
								reset(true);

								lua_init();
						}
						});

				lua.set_function("get_random_number", [&](int start, int end) {
						return std::rand() % end + start;
						});

				lua.set_function("generate_uuid", [&]() {
						return entity_manager->generate_uuid();
						});
		}

		int Engine::game_loop()
		{
				// Timer code from : https://enginedev.stackexchange.com/a/163508/18014
				const int UPDATE_FREQUENCY{ 30 }; // 30 FPS seems fine for what we are doing right now
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

						if (!(game_config.valid() && check_game_config(game_config, lua.lua_state())))
						{
								std::cout << "game script does not define the 'Game' configuration table." << std::endl;
								return -1;
						}

						if (init_sdl(game_config) < 0) return -1;

						setup_lua_helpers(lua.lua_state());

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

						tear_down_sdl();
				}
				else
				{
						sol::error err = game_script;
						std::cout << "Lua script error: "
								<< "\n\t" << err.what() << std::endl;
				}

				return 0;
		}
}
