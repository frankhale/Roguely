/*
* Entity.hpp
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

#include "Common.hpp"
#include <vector>

enum class EntityType
{
		Player,
		Enemy,
		Pickup
};

enum class EntitySubType
{
		Player = 100,
		Coin = 1,
		Health_Gem = 2,
		Attack_Gem = 3,
		Spider = 50,
		Lurcher = 51,
		Crab = 52,
		Bug = 53,
		Fire_Walker = 54,
		Crimson_Shadow = 55,
		Purple_Blob = 56,
		Orange_Blob = 57
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

class Entity
{
public:
		Entity(int id, Point point, EntityType entityType);

		template<class T>
		std::shared_ptr<T> find_component()
		{
				auto found_component = std::find_if(components->begin(), components->end(),
						[](const auto& c) {
								return std::dynamic_pointer_cast<T>(c) != nullptr;
						});
				
				std::shared_ptr<T> t_component = nullptr;

				if (&found_component != nullptr)
				{
						t_component = std::dynamic_pointer_cast<T>(*found_component);

						if (t_component != nullptr)
						{
								return t_component;
						}
				}

				return nullptr;
		}

		auto point() const { return _point; }
		auto id() const { return _id; }
		auto entityType() const { return _entityType; }
		void SetPoint(Point p) { _point.x = p.x; _point.y = p.y; }
		void AddComponent(std::shared_ptr<Component> c) { components->push_back(c); }
		void AddComponents(std::shared_ptr<std::vector<std::shared_ptr<Component>>> c) { components->insert(components->end(), c->begin(), c->end()); }

private:
		Point _point{};
		EntityType _entityType{};
		std::shared_ptr<std::vector<std::shared_ptr<Component>>> components = nullptr;
		int _id = 0;
};