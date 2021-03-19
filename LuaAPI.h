/*
* LuaAPI.h
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
#include "Game.h"
#include "SpriteSheet.h"
#include "Text.h"

sol::table get_sprite_info(std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets, std::string name, sol::this_state s);
sol::table add_sprite_sheet(SDL_Renderer* renderer, std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets, std::string name, std::string path, int sw, int sh, sol::this_state s);
void draw_sprite(SDL_Renderer* renderer, std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets, std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height);
void draw_text(SDL_Renderer* renderer, std::shared_ptr<roguely::common::Text> text, std::string t, int x, int y);
void play_sound(std::shared_ptr<std::vector<std::shared_ptr<roguely::common::Sound>>> sounds, std::string name);
std::string add_entity(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_type, int x, int y, sol::table components_table);
std::string add_entity(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_type, sol::table components_table);
sol::table add_entities(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_type, sol::table components_table, int num, sol::this_state s);
void remove_entity(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_id, sol::this_state s);
void set_component_value(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, std::string entity_id, std::string component_name, std::string key, sol::object value, sol::this_state s);
void set_draw_color(SDL_Renderer* renderer, int r, int g, int b, int a);
void draw_point(SDL_Renderer* renderer, int x, int y);
void draw_rect(SDL_Renderer* renderer, int x, int y, int w, int h);
void draw_filled_rect(SDL_Renderer* renderer, int x, int y, int w, int h);
sol::table get_test_map(sol::this_state s);
sol::table get_map(std::shared_ptr<roguely::game::Game> game, std::string name, bool light, sol::this_state s);
sol::table get_random_point(std::shared_ptr<roguely::game::Game> game, sol::table entity_groups_to_check, sol::this_state s);
bool is_tile_walkable(std::shared_ptr<roguely::game::Game> game, int x, int y, std::string direction, std::string who, sol::table entity_groups_to_check);
sol::table get_entity_group_points(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, sol::this_state s);
// internal
void send_key_event(std::string key, sol::this_state s);
void render(float delta_time, sol::this_state s);
void tick(float delta_time, sol::this_state s);
sol::table convert_entity_to_lua_table(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s);
sol::table convert_entity_group_to_lua_table(std::shared_ptr<roguely::game::Game> game, std::string entity_group_name, sol::this_state s);
void emit_lua_update_for_entity(std::shared_ptr<roguely::ecs::Entity> entity, sol::this_state s);