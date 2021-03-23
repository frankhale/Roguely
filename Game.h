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

#pragma once

#include "Common.h"
#include "Entity.h"
#include "LevelGeneration.h"
#include "SpriteSheet.h"
#include "Text.h"

namespace roguely::game
{
		struct TileWalkableInfo
		{
				bool walkable = false;
				roguely::common::Point point{};
				roguely::ecs::EntityType entity_type{};
		};

		class Game
		{
		public:
				Game(sol::table game_config);

				void generate_map_for_testing();
				void generate_map(std::string name, int map_width, int map_height)
				{
						auto map = roguely::level_generation::init_cellular_automata(map_width, map_height);
						roguely::level_generation::perform_cellular_automaton(map, map_width, map_height, 10);

						maps->erase(std::remove_if(maps->begin(), maps->end(),
								[&](std::shared_ptr<roguely::common::Map> m) {
										if (m->name == name) return true;
										
										return false;
								}), maps->end());

						//for (int row = 0; row < 40; row++)
						//{
						//		for (int column = 0; column < 100; column++)
						//		{
						//				if ((*map)(row, column) == 0) {
						//						std::cout << "#";
						//				}
						//				else if ((*map)(row, column) == 9) {
						//						std::cout << ".";
						//				}
						//		}

						//		std::cout << std::endl;
						//}

						maps->emplace_back(std::make_shared<roguely::common::Map>(name, map_width, map_height, map));
				}

				std::shared_ptr<roguely::common::Map> get_map(std::string name);

				void add_sprite_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string spritesheet_name, int sprite_in_spritesheet_id, std::string sprite_name);
				void add_health_component(std::shared_ptr<roguely::ecs::Entity> entity, int h);
				void add_stats_component(std::shared_ptr<roguely::ecs::Entity> entity, int a);
				void add_score_component(std::shared_ptr<roguely::ecs::Entity> entity, int s);
				void add_value_component(std::shared_ptr<roguely::ecs::Entity> entity, int v);
				void add_inventory_component(std::shared_ptr<roguely::ecs::Entity> entity, std::vector<std::pair<std::string, int>> items);
				void add_lua_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string n, std::string t, sol::table props);
				bool remove_entity(std::string entity_group_name, std::string entity_id);

				std::shared_ptr<roguely::ecs::Entity> update_entity_position(std::string entity_group_name, std::string entity_id, int x, int y);
				int get_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key);
				int get_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key);
				bool set_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key, int value);
				std::shared_ptr<roguely::ecs::Entity> set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, int value);
				std::shared_ptr<roguely::ecs::Entity> set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, std::pair<std::string, int> value);

				std::shared_ptr<roguely::ecs::EntityGroup> create_entity_group(std::string name);
				std::shared_ptr<roguely::ecs::Entity> add_entity_to_group(std::shared_ptr<roguely::ecs::EntityGroup> entity_group, roguely::ecs::EntityType entity_type, std::string id, roguely::common::Point point);
				std::shared_ptr<roguely::ecs::EntityGroup> get_entity_group(std::string name);
				std::shared_ptr<roguely::ecs::Entity> get_entity(std::shared_ptr<roguely::ecs::EntityGroup> entity_group, std::string entity_id);

				std::string generate_uuid();
				void switch_map(std::string map_name);

				// There seems to be way too many functions to check to see if a tile is walkable, ugh!
				bool is_tile_player_tile(int x, int y, roguely::common::MovementDirection dir);
				bool is_tile_on_map_traversable(int x, int y, roguely::common::MovementDirection dir, int tileId);
				bool is_entity_location_traversable(int x, int y, std::shared_ptr<std::vector<std::shared_ptr<roguely::ecs::Entity>>> entities)
				{
						return !(std::any_of(entities->begin(), entities->end(), [&](std::shared_ptr<roguely::ecs::Entity> elem) { return elem->x() == x && elem->y() == y; }));
				}
				auto is_entity_location_traversable(int x, int y, std::shared_ptr<std::vector<std::shared_ptr<roguely::ecs::Entity>>> entities, roguely::common::WhoAmI whoAmI, roguely::common::MovementDirection dir);
				TileWalkableInfo is_tile_walkable(int x, int y, roguely::common::MovementDirection dir, roguely::common::WhoAmI whoAmI, std::vector<std::string> entity_groups_to_check);
				bool is_xy_blocked(int x, int y, std::vector<std::string> entity_groups_to_check);

				roguely::common::Point generate_random_point(std::vector<std::string> entity_groups_to_check);
				roguely::common::Point get_open_point_for_xy(int x, int y, std::vector<std::string> entity_groups_to_check);

				void update_player_viewport_points();
				void rb_fov();

				int get_view_port_x() const { return view_port_x; }
				int get_view_port_y() const { return view_port_y; }
				int get_view_port_width() const { return view_port_width; }
				int get_view_port_height() const { return view_port_height; }

				void set_view_port_width(int vpw) 
				{ 
						VIEW_PORT_WIDTH = vpw;
						view_port_width = vpw; 
				}
				void set_view_port_height(int vph) 
				{ 
						VIEW_PORT_HEIGHT = vph;
						view_port_height = vph; 
				}

				std::vector<std::string> get_entity_group_names() 
				{
						std::vector<std::string> results{};
						for (auto& eg : *entity_groups) { results.push_back(eg->name); }
						return results;
				}

				void add_spritesheet(SDL_Renderer* renderer, std::string name, std::string path, int sw, int sh);
				std::shared_ptr<roguely::sprites::SpriteSheet> find_spritesheet(std::string name) {
						auto sheet = std::find_if(sprite_sheets->begin(), sprite_sheets->end(),
								[&](const std::shared_ptr<roguely::sprites::SpriteSheet>& ss) {
										return ss->get_name() == name;
								});

						if (sheet != sprite_sheets->end())
						{
								return *sheet;
						}

						return nullptr;
				}

				void draw_sprite(SDL_Renderer* renderer, std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height);
				void draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y);
				void draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y, int r, int g, int b, int a);
				roguely::common::TextExtents get_text_extents(std::string t, std::string size);
				void play_sound(std::string name);
				void reset(bool reset_ptr);

				void tear_down_sdl();

		private:
				int view_port_x = 0;
				int view_port_y = 0;
				int view_port_width = 0;
				int view_port_height = 0;

				int VIEW_PORT_WIDTH = 0;
				int VIEW_PORT_HEIGHT = 0;

				sol::table game_config;

				Mix_Music* soundtrack{};

				std::shared_ptr<roguely::common::Map> current_map{};				
				std::shared_ptr<roguely::ecs::Entity> player{};				
				std::string player_id{};

				std::shared_ptr<std::vector<std::shared_ptr<roguely::common::Map>>> maps{};
				std::shared_ptr<std::vector<std::shared_ptr<roguely::ecs::EntityGroup>>> entity_groups{};
				std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets{};
				std::shared_ptr<std::vector<std::shared_ptr<roguely::common::Sound>>> sounds{};

				// Corner cut here. This is silly but keeps us going, we need to 
				// rethink having a reusable Text class for variable size fonts.
				std::shared_ptr<roguely::common::Text> text_large{};
				std::shared_ptr<roguely::common::Text> text_medium{};
				std::shared_ptr<roguely::common::Text> text_small{};
		};
}