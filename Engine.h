/*
* engine.h
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

#define SOL_ALL_SAFETIES_ON 1

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>
#include <mpg123.h>
#include <queue>
#include <sstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <sol/sol.hpp>
#include <vector>

namespace roguely::common
{
		enum class MovementDirection
		{
				Left,
				Right,
				Up,
				Down
		};

		enum class WhoAmI
		{
				Player,
				Enemy
		};

		struct Point
		{
				Point() { x = -1; y = -1; }
				Point(int x, int y)
				{
						this->x = x;
						this->y = y;
				}

				bool eq(Point p) { return p.x == x && p.y == y; }

				int x = -1;
				int y = -1;
		};

		struct Sound
		{
				std::string name;
				Mix_Chunk* sound;

		public:
				void play() { Mix_PlayChannel(-1, sound, 0); }
		};

		struct Map
		{
		public:
				Map(std::string n, int w, int h, std::shared_ptr<boost::numeric::ublas::matrix<int>> m)
				{
						name = n;
						map = m;
						width = w;
						height = h;
						light_map = std::make_shared<boost::numeric::ublas::matrix<int>>(h, w);
				}

				std::string name{};
				int width = 0;
				int height = 0;
				std::shared_ptr<boost::numeric::ublas::matrix<int>> map{};
				std::shared_ptr<boost::numeric::ublas::matrix<int>> light_map{};
		};

		// ref: https://enginedev.stackexchange.com/a/163508/18014
		struct Timer
		{
				Uint64 previous_ticks{};
				float elapsed_seconds{};

				void tick()
				{
						const Uint64 current_ticks{ SDL_GetPerformanceCounter() };
						const Uint64 delta{ current_ticks - previous_ticks };
						previous_ticks = current_ticks;
						static const Uint64 TICKS_PER_SECOND{ SDL_GetPerformanceFrequency() };
						elapsed_seconds = delta / static_cast<float>(TICKS_PER_SECOND);
				}
		};

		struct TextExtents
		{
				int width, height;
		};

		class Text
		{
		public:
				Text();

				int load_font(const char* path, int ptsize);
				void draw_text(SDL_Renderer* renderer, int x, int y, const char* text);
				void draw_text(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color);
				TextExtents get_text_extents(const char* text);

		private:
				TTF_Font* font;
				SDL_Texture* text_texture;

				SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };
				SDL_Color text_background_color = { 0x00, 0x00, 0x00, 0xFF };
		};

		struct AStarNode
		{
				AStarNode() {}
				AStarNode(const AStarNode& p, const Point& pos)
				{
						if (&parent != nullptr)
								parent = std::make_shared<AStarNode>(p);

						if (&position != nullptr)
								position = std::make_shared<Point>(pos);
				}

				bool eq(const AStarNode& x)
				{
						if (&x == nullptr) return false;

						return (position->x == x.position->x &&
								position->y == x.position->y);
				}

				std::shared_ptr<AStarNode> parent{};
				std::shared_ptr<Point> position{};
				int f = 0;
				int g = 0;
				int h = 0;
		};

		class AStarPathFinder
		{
		public:
				AStarPathFinder(std::shared_ptr<boost::numeric::ublas::matrix<int>> map)
				{
						this->map = map; // std::make_shared<boost::numeric::ublas::matrix<int>>(map);
						max_iterations = (int)(this->map->size1() * this->map->size2());
				}

				std::shared_ptr<std::queue<Point>> find_path(Point start, Point end, int walkable_tile_id);

		private:
				std::shared_ptr<boost::numeric::ublas::matrix<int>> map;

				Point pos_array[8] = {
						{ 0, -1 },
						{ 0, 1 },
						{ -1, 0 },
						{ 1, 0 },
						{ -1, -1 },
						{ -1, 1 },
						{ 1, -1 },
						{ 1, 1 }
				};

				int max_iterations = 0;
		};
}

namespace roguely::sprites
{
		class SpriteSheet
		{
		public:
				SpriteSheet(SDL_Renderer* renderer, std::string n, std::string p, int sw, int sh);
				~SpriteSheet()
				{
						SDL_DestroyTexture(spritesheet_texture);
				}

				void draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y);
				void draw_sprite(SDL_Renderer* renderer, int sprite_id, int x, int y, int scaled_width, int scaled_height);

				SDL_Texture* get_spritesheet_texture() const { return spritesheet_texture; }

				auto get_name() const { return name; }

				sol::table get_sprites_as_lua_table(sol::this_state s);

		private:
				std::string name;
				std::string path;
				int sprite_width = 0;
				int sprite_height = 0;
				std::unique_ptr<std::vector<std::shared_ptr<SDL_Rect>>> sprites{};
				SDL_Texture* spritesheet_texture{};
		};
}

namespace roguely::level_generation
{
		// Quick and dirty cellular automata that I learned about from YouTube. We 
		// can do more but currently are just doing the very least to get a 
		// playable level.

		int get_neighbor_wall_count(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int x, int y);
		void perform_cellular_automaton(std::shared_ptr<boost::numeric::ublas::matrix<int>> map, int map_width, int map_height, int passes);
		std::shared_ptr<boost::numeric::ublas::matrix<int>> init_cellular_automata(int map_width, int map_height);
}

namespace roguely::ecs
{
		// We aren't using all these yet. I am simply putting them in place for
		// later.
		enum class EntityType
		{
				Player,
				Enemy,
				NPC,
				Item,
				Interactable,
				Ground,
				Wall
		};

		struct TileWalkableInfo
		{
				bool walkable = false;
				roguely::common::Point point{};
				roguely::ecs::EntityType entity_type{};
		};

		// There are some built in components and the rest are defined in Lua 
		// through the LuaComponent.
		//
		// I suspect I'll add a AnimationComponent here to capture the state needed
		// to support simple animation.
		class Component {
		public:
				virtual ~Component() {};

				auto get_component_name() const { return component_name; }
				void set_component_name(std::string name) { component_name = name; }

		private:
				std::string component_name{};
		};

		class HealthComponent : public Component
		{
		public:
				HealthComponent(int h) { max_health = h; health = h; }

				int get_health() const { return health; }
				void set_health(int h) { health = h; }
				void add_health(int h) { health += h; }
				void remove_health(int h) { health -= h; }
				int get_max_health() const { return max_health; }
				void set_max_health(int mh) { max_health = mh; }

		private:
				int max_health = 0;
				int health = 0;
		};

		class StatsComponent : public Component
		{
		public:
				StatsComponent(int a) { attack = a; }

				int get_attack() const { return attack; }
				void set_attack(int a) { attack = a; }

		private:
				int attack = 0;
		};

		class ScoreComponent : public Component
		{
		public:
				ScoreComponent(int s) { score = s; }
				auto get_score() const { return score; }
				void update_score(int s) { score += s; }
				void set_score(int s) { score = s; }

		private:
				int score = 0;
		};

		class SpriteComponent : public Component
		{
		public:
				SpriteComponent(std::string ss_name, int sprite_in_spritesheet_id, std::string n)
				{
						spritesheet_name = ss_name;
						sprite_id = sprite_in_spritesheet_id;
						name = n;
				}

				auto get_name() const { return name; }
				auto get_spritesheet_name() const { return spritesheet_name; }
				auto get_sprite_id() const { return sprite_id; }

		private:
				std::string spritesheet_name;
				std::string name;
				int sprite_id = 0;
		};

		class ValueComponent : public Component
		{
		public:
				ValueComponent(int v) { value = v; }
				auto get_value() const { return value; }

		private:
				int value = 0;
		};

		// Currently not used.
		class InventoryComponent : public Component
		{
		public:
				InventoryComponent() { inventory = std::make_unique<std::vector<std::shared_ptr<std::pair<std::string, int>>>>(); }

				void upsert_item(std::pair<std::string, int> item_key_value_pair);
				void add_item(std::string name, int count);
				void remove_item(std::string name, int count);
				void for_each_item(std::function<void(std::shared_ptr<std::pair<std::string, int>>&)> fi) { for (auto& i : *inventory) fi(i); }

		private:
				std::unique_ptr<std::vector<std::shared_ptr<std::pair<std::string, int>>>> inventory;
		};

		class LuaComponent : public Component
		{
		public:
				LuaComponent(std::string n, std::string t, sol::table props, sol::this_state s) {
						sol::state_view lua(s);
						name = n;
						type = t;
						properties = lua.create_table();
						for (const auto& p : props)
						{
								properties.set(p.first, p.second);
						}
				}
				auto get_name() const { return name; }
				auto get_type() const { return type; }
				auto get_properties() const { return properties; }
				void set_property(std::string name, int value, sol::this_state s) { properties.set(name, value); }
				void set_properties(sol::table props, sol::this_state s) { properties = props; }

		private:
				std::string name;
				std::string type;
				sol::table properties;
		};

		class Entity;

		struct EntityGroup
		{
				std::string name{};
				std::shared_ptr<std::vector<std::shared_ptr<Entity>>> entities{};
		};

		class Entity
		{
		public:
				Entity(std::shared_ptr<EntityGroup> eg, std::string id, roguely::common::Point p, EntityType et)
				{
						this->id = id;
						point = p;
						entity_group = eg;
						entity_type = et;
						components = std::make_unique<std::vector<std::shared_ptr<Component>>>();
				}

				template<typename T>
				std::shared_ptr<T> find_component_by_type();
				std::shared_ptr<Component> find_component_by_name(std::string name) const;

				auto x() const { return point.x; }
				auto y() const { return point.y; }
				auto get_point() const { return point; }
				void set_point(roguely::common::Point p) { point = p; }
				auto get_id() const { return id; }
				auto get_entity_type() const { return entity_type; }
				auto get_entity_group() const { return entity_group; }
				void set_entity_group(std::shared_ptr<EntityGroup> eg) { entity_group = eg; }
				void add_component(std::shared_ptr<Component> c) { components->emplace_back(c); }
				void add_components(std::unique_ptr<std::vector<std::shared_ptr<Component>>> c) { components->insert(components->end(), c->begin(), c->end()); }
				void for_each_component(std::function<void(std::shared_ptr<Component>&)> fc) { for (auto& c : *components) fc(c); }

		private:
				std::shared_ptr<EntityGroup> entity_group{};
				roguely::common::Point point{};
				EntityType entity_type{};
				std::unique_ptr<std::vector<std::shared_ptr<Component>>> components{};
				std::string id{};
		};

		class EntityManager
		{
		public:
				EntityManager()
				{
						entity_groups = std::make_unique<std::vector<std::shared_ptr<roguely::ecs::EntityGroup>>>();
				}

				void add_sprite_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string spritesheet_name, int sprite_in_spritesheet_id, std::string sprite_name);
				void add_health_component(std::shared_ptr<roguely::ecs::Entity> entity, int h);
				void add_stats_component(std::shared_ptr<roguely::ecs::Entity> entity, int a);
				void add_score_component(std::shared_ptr<roguely::ecs::Entity> entity, int s);
				void add_value_component(std::shared_ptr<roguely::ecs::Entity> entity, int v);
				void add_inventory_component(std::shared_ptr<roguely::ecs::Entity> entity, std::vector<std::pair<std::string, int>> items);
				void add_lua_component(std::shared_ptr<roguely::ecs::Entity> entity, std::string n, std::string t, sol::table props, sol::this_state s);

				std::shared_ptr<roguely::ecs::EntityGroup> create_entity_group(std::string name);
				std::string add_entity(std::string entity_group_name, std::string entity_type, int x, int y, sol::table components_table, sol::this_state s);
				std::string add_entity(std::string entity_group_name, std::string entity_type, sol::table components_table, std::function<roguely::common::Point()> get_random_point, sol::this_state s);
				sol::table add_entities(std::string entity_group_name, std::string entity_type, sol::table components_table, int num, std::function<roguely::common::Point()> get_random_point, sol::this_state s);
				std::shared_ptr<roguely::ecs::Entity> add_entity_to_group(std::shared_ptr<roguely::ecs::EntityGroup> entity_group, roguely::ecs::EntityType entity_type, std::string id, roguely::common::Point point);
				bool remove_entity(std::string entity_group_name, std::string entity_id);
				void remove_entity(std::string entity_group_name, std::string entity_id, sol::this_state s);

				void update_entities(std::string entity_group_name, std::string component_name, std::string key, sol::object value, sol::this_state s);
				void update_entities(std::string entity_group_name, sol::table entity_positions);
				void update_entity_position(std::string entity_group_name, sol::table entity_positions);				
				std::shared_ptr<roguely::ecs::Entity> update_entity_position(std::string entity_group_name, std::string entity_id, int x, int y);
				
				sol::table get_entity_group_points(std::string entity_group_name, sol::this_state s);
				std::vector<std::string> get_entity_group_names()
				{
						std::vector<std::string> results{};
						for (auto& eg : *entity_groups) { results.push_back(eg->name); }
						return results;
				}
				std::shared_ptr<roguely::ecs::EntityGroup> get_entity_group(std::string name);
				std::shared_ptr<roguely::ecs::Entity> get_entity(std::shared_ptr<roguely::ecs::EntityGroup> entity_group, std::string entity_id);				
				int get_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key);
				int get_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key);
				void set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, sol::object value, sol::this_state s);
				bool set_component_value(std::shared_ptr<roguely::ecs::Component> component, std::string key, int value, sol::this_state s);
				std::shared_ptr<roguely::ecs::Entity> set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, int value, sol::this_state s);
				std::shared_ptr<roguely::ecs::Entity> set_component_value(std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, std::pair<std::string, int> value, sol::this_state s);

				bool is_xy_player_xy(int x, int y, roguely::common::MovementDirection dir);
				bool is_entity_location_traversable(int x, int y, std::string entity_group_name)
				{
						auto entity_group = get_entity_group(entity_group_name);
						if (entity_group == nullptr) return false;

						return !(std::any_of(entity_group->entities->begin(), entity_group->entities->end(), [&](std::shared_ptr<roguely::ecs::Entity> elem) { return elem->x() == x && elem->y() == y; }));
				}
				std::shared_ptr<roguely::ecs::TileWalkableInfo> is_entity_location_traversable(int x, int y, std::string entity_group_name, roguely::common::WhoAmI whoAmI, roguely::common::MovementDirection dir);

				void emit_lua_update_for_entity_group(std::string entity_group_name, std::string entity_id, sol::this_state s);
				void emit_lua_update_for_entity_group(std::string entity_group_name, sol::this_state s);
				void emit_lua_update_for_entity(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s);
				sol::table convert_entity_to_lua_table(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s);
				sol::table convert_entity_group_to_lua_table(std::string entity_group_name, sol::this_state s);

				std::string generate_uuid();
				roguely::common::Point get_player_point() const { return player->get_point(); }

		private:
				std::string player_id{};
				std::shared_ptr<roguely::ecs::Entity> player{};
				std::unique_ptr<std::vector<std::shared_ptr<roguely::ecs::EntityGroup>>> entity_groups{};
		};
}

namespace roguely::engine
{
		class Engine
		{
		public:
				Engine();

				bool is_xy_blocked(int x, int y);
				roguely::common::Point generate_random_point();
				roguely::common::Point get_open_point_for_xy(int x, int y);
				bool is_tile_player_tile(int x, int y, roguely::common::MovementDirection dir);
				bool is_tile_on_map_traversable(int x, int y, roguely::common::MovementDirection dir, int tileId);
				void switch_map(std::string map_name);				
				void generate_map(std::string name, int map_width, int map_height);
				std::shared_ptr<roguely::common::Map> get_map(std::string name);
				void update_player_viewport_points();
				void rb_fov();
				int get_view_port_x() const { return view_port_x; }
				int get_view_port_y() const { return view_port_y; }
				int get_view_port_width() const { return view_port_width; }
				int get_view_port_height() const { return view_port_height; }
				void set_view_port_width(int vpw) { VIEW_PORT_WIDTH = vpw; view_port_width = vpw; }
				void set_view_port_height(int vph) { VIEW_PORT_HEIGHT = vph; view_port_height = vph; }
				void add_spritesheet(SDL_Renderer* renderer, std::string name, std::string path, int sw, int sh);
				std::shared_ptr<roguely::sprites::SpriteSheet> find_spritesheet(std::string name);
				void draw_sprite(SDL_Renderer* renderer, std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height);
				void draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y);
				void draw_text(SDL_Renderer* renderer, std::string t, std::string size, int x, int y, int r, int g, int b, int a);
				roguely::common::TextExtents get_text_extents(std::string t, std::string size);
				void play_sound(std::string name);
				void reset(bool reset_ptr);
				void tear_down_sdl();
				int init_sdl(sol::table game_config);
				int game_loop();

				sol::table get_sprite_info(std::string name, sol::this_state s);
				sol::table add_sprite_sheet(SDL_Renderer* renderer, std::string name, std::string path, int sw, int sh, sol::this_state s);
				void setup_lua_helpers(sol::this_state s);
				sol::table get_map(std::string name, bool light, sol::this_state s);
				sol::table get_random_point(sol::this_state s);
				sol::table get_open_point_for_xy(int x, int y, sol::this_state s);
				sol::table get_entity_group_points(std::string entity_group_name, sol::this_state s);

				void set_draw_color(SDL_Renderer* renderer, int r, int g, int b, int a);
				void draw_point(SDL_Renderer* renderer, int x, int y);
				void draw_rect(SDL_Renderer* renderer, int x, int y, int w, int h);
				void draw_filled_rect(SDL_Renderer* renderer, int x, int y, int w, int h);
				bool is_tile_walkable(int x, int y, std::string direction, std::string who, sol::table entity_groups_to_check);
				void send_key_event(std::string key, sol::this_state s);
				void render(float delta_time, sol::this_state s);
				void tick(float delta_time, sol::this_state s);
				void render_graphic(SDL_Renderer* renderer, std::string path, int window_width, int x, int y, bool centered, bool scaled, float scaled_factor);

		private:

				bool check_game_config(sol::table game_config, sol::this_state s);
				auto get_text_reference(std::string size)
				{
						auto text = &text_small;

						if (size == "small")
								text = &text_small;
						else if (size == "medium")
								text = &text_medium;
						else if (size == "large")
								text = &text_large;

						return text;
				}

				int view_port_x = 0;
				int view_port_y = 0;
				int view_port_width = 0;
				int view_port_height = 0;

				int VIEW_PORT_WIDTH = 0;
				int VIEW_PORT_HEIGHT = 0;

				std::string player_id{};

				std::shared_ptr<roguely::common::Map> current_map{};
				std::unique_ptr<std::vector<std::shared_ptr<roguely::common::Map>>> maps{};
				std::unique_ptr<roguely::ecs::EntityManager> entity_manager{};
				std::unique_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets{};
				std::unique_ptr<std::vector<std::shared_ptr<roguely::common::Sound>>> sounds{};
				std::unique_ptr<roguely::common::AStarPathFinder> path_finder{};

				// Corner cut here. This is silly but keeps us going, we need to 
				// rethink having a reusable Text class for variable size fonts.
				std::unique_ptr<roguely::common::Text> text_large{};
				std::unique_ptr<roguely::common::Text> text_medium{};
				std::unique_ptr<roguely::common::Text> text_small{};

				SDL_Window* window = nullptr;
				SDL_Surface* window_surface = nullptr;
				SDL_Renderer* renderer = nullptr;
				Mix_Music* soundtrack{};
		};
}