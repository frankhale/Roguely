/*
* Entity.h
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

namespace roguely::ecs
{
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
				HealthComponent(int health) { max_health = health; health = health; }

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
		private:
				int score = 0;
		};

		class SpriteComponent : public Component
		{
		public:
				SpriteComponent(std::string spritesheet_name, int sprite_in_spritesheet_id, std::string n)
				{
						spritesheet = spritesheet_name;
						id = sprite_in_spritesheet_id;
						name = n;
				}

				auto get_name() const { return name; }

		private:
				std::string spritesheet;
				std::string name;
				int id = 0;
		};

		class ValueComponent : public Component
		{
		public:
				ValueComponent(int v) { value = v; }
				auto get_value() const { return value; }
		private:
				int value = 0;
		};

		class InventoryComponent : public Component
		{
		public:
				InventoryComponent() { inventory = std::make_unique<std::vector<std::pair<std::string, int>>>(); }

				void upsert_item(std::pair<std::string, int> item_key_value_pair)
				{
						auto item = std::find_if(inventory->begin(), inventory->end(),
								[&](const std::pair<std::string, int>& i) {
										return i.first == item_key_value_pair.first;
								});

						if (item != inventory->end())
						{
								item->second = item_key_value_pair.second;
						}
						else
						{
								add_item(item_key_value_pair.first, item_key_value_pair.second);
						}
				}

				void add_item(std::string name, int count)
				{
						std::pair item{ name, count };
						inventory->push_back(item);
				};

				void remove_item(std::string name, int count)
				{
						auto item = std::find_if(inventory->begin(), inventory->end(),
								[&](const std::pair<std::string, int>& i) {
										return i.first == name;
								});

						if (item != inventory->end())
						{
								item->second -= count;

								if (item->second <= 0) {
										inventory->erase(std::remove_if(inventory->begin(), inventory->end(),
												[&](const std::pair<std::string, int> i) {
														return i.first == name;
												}),
												inventory->end());
								}
						}
				};

		private:
				std::unique_ptr<std::vector<std::pair<std::string, int>>> inventory;
		};

		class LuaComponent : public Component
		{
		public:
				LuaComponent(std::string n, std::string t, sol::table props) { name = n; type = t; properties = props; }

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
				Entity(std::shared_ptr<EntityGroup> entityGroup, std::string id, roguely::common::Point point, EntityType entityType);

				template<typename T>
				std::shared_ptr<T> find_component_by_type()
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

				std::shared_ptr<Component> find_component_by_name(std::string name) const {
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

				auto x() const { return _point.x; }
				auto y() const { return _point.y; }

				auto get_point() const { return _point; }
				auto get_id() const { return _id; }
				auto get_entity_type() const { return _entityType; }
				auto get_entity_group() const { return _entityGroup; }
				void set_entity_group(std::shared_ptr<EntityGroup> entityGroup) { _entityGroup = entityGroup; }
				void set_point(roguely::common::Point p) { _point.x = p.x; _point.y = p.y; }
				void add_component(std::shared_ptr<Component> c) { components->emplace_back(c); }
				void add_components(std::unique_ptr<std::vector<std::shared_ptr<Component>>> c) { components->insert(components->end(), c->begin(), c->end()); }

		private:
				std::shared_ptr<EntityGroup> _entityGroup{};
				roguely::common::Point _point{};
				EntityType _entityType{};
				std::unique_ptr<std::vector<std::shared_ptr<Component>>> components{};
				std::string _id{};
		};
}