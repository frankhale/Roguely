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
#include "LuaAPI.h"
 
// TODO: Move sprite_sheets, sounds, text_* to game class

SDL_Window* window = nullptr;
SDL_Surface* window_surface = nullptr;
SDL_Renderer* renderer = nullptr;
Mix_Music* soundtrack = nullptr;
std::shared_ptr<roguely::game::Game> game = nullptr;
std::shared_ptr<std::vector<std::shared_ptr<roguely::common::Sound>>> sounds = nullptr;
std::shared_ptr<roguely::common::Text> text_large = nullptr;
std::shared_ptr<roguely::common::Text> text_medium = nullptr;
std::shared_ptr<roguely::common::Text> text_small = nullptr;
std::shared_ptr<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>> sprite_sheets = nullptr;

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
		if ((bool)game_config["music"])
		{
				Mix_FreeMusic(soundtrack);
		}

		for (auto& s : *sounds)
		{
				Mix_FreeChunk(s->sound);
		}

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		Mix_Quit();
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
}

void play_soundtrack(std::string soundtrack_path)
{
		soundtrack = Mix_LoadMUS(soundtrack_path.c_str());
		Mix_PlayMusic(soundtrack, 1);
}

// THIS IS ONLY HERE TO MAKE VCPKG INCLUDE MPG123 SO THAT SDL_MIXER WILL WORK
int __DUMMY_ONLY_HERE_TO_MAKE_VCPKG_INCLUDE_MPG123_FOR_SDL_MIXER__()
{
		return mpg123_feature(MPG123_FEATURE_DECODE_LAYER3);
}

bool check_game_config(sol::table game_config)
{
		bool result = true;

		// TODO: Add more checks in here so that we can cover 100% of the config

		__DUMMY_ONLY_HERE_TO_MAKE_VCPKG_INCLUDE_MPG123_FOR_SDL_MIXER__();

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

		std::string font_path = game_config["font_path"];

		game = std::make_shared<roguely::game::Game>();
		game->set_view_port_width(game_config["view_port_width"]);
		game->set_view_port_height(game_config["view_port_height"]);

		sprite_sheets = std::make_shared<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>>();
		text_medium = std::make_shared<roguely::common::Text>();
		text_medium->load_font(font_path.c_str(), 40);
		text_large = std::make_shared<roguely::common::Text>();
		text_large->load_font(font_path.c_str(), 63);
		text_small = std::make_shared<roguely::common::Text>();
		text_small->load_font(font_path.c_str(), 26);

		Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
		Mix_Volume(-1, 3);
		Mix_VolumeMusic(3);

		sounds = std::make_shared<std::vector<std::shared_ptr<roguely::common::Sound>>>();

		if ((bool)game_config["music"])
		{
				std::string soundtrack_path = game_config["soundtrack_path"];
				play_soundtrack(soundtrack_path);
		}

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
									Mix_LoadWAV(sound.second.as<std::string>().c_str()) };

								auto ss = std::make_shared<roguely::common::Sound>(s);

								sounds->emplace_back(ss);
						}
				}
		}
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

				// FIXME: This is temporary as we had some issues passing around Lua 
				// state to various functions which was blowing us up. Things were 
				// blowing up and I didn't find a quick fix. We will revisit this soon
				// the Lua integration and some of the  global variables at the top
				// will be rolled into the Game class.

				lua["get_test_map"] = get_test_map;
				
				lua.set_function("get_sprite_info", [&](std::string sprite_sheet_name, sol::this_state s) {
						return get_sprite_info(sprite_sheets, sprite_sheet_name, s);
						});

				lua.set_function("draw_text", [&](std::string t, std::string size, int x, int y) {
						std::shared_ptr<roguely::common::Text> text = text_small;

						if (size == "small")
								text = text_small;
						else if (size == "medium")
								text = text_medium;
						else if (size == "large")
								text = text_large;

						draw_text(renderer, text, t, x, y);
						});

				lua.set_function("get_text_extents", [&](std::string t, std::string size, sol::this_state s) {
						std::shared_ptr<roguely::common::Text> text = text_small;

						if (size == "small")
								text = text_small;
						else if (size == "medium")
								text = text_medium;
						else if (size == "large")
								text = text_large;

						auto extents = text->get_text_extents(t.c_str());
						sol::table extents_table = lua.create_table();
						extents_table.set("width", extents.width);
						extents_table.set("height", extents.height);
						return extents_table;
						});

				lua.set_function("add_sprite_sheet", [&](std::string name, std::string path, int sw, int sh, sol::this_state s) {
						return add_sprite_sheet(renderer, sprite_sheets, name, path, sw, sh, s);
						});

				lua.set_function("draw_sprite", [&](std::string name, int sprite_id, int x, int y) {
						draw_sprite(renderer, sprite_sheets, name, sprite_id, x, y, 0, 0);
						});

				lua.set_function("draw_sprite_scaled", [&](std::string name, int sprite_id, int x, int y, int scaled_width, int scaled_height) {
						draw_sprite(renderer, sprite_sheets, name, sprite_id, x, y, scaled_width, scaled_height);
						});

				lua.set_function("play_sound", [&](std::string name) {
						play_sound(sounds, name);
						});

				lua.set_function("generate_map", [&](std::string name, int map_width, int map_height) {
						game->generate_map(name, map_width, map_height);
						});

				lua.set_function("add_entity", [&](std::string entity_group, std::string entity_type, int x, int y, sol::table components_table) {
						return add_entity(game, entity_group, entity_type, x, y, components_table);
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
								game->reset();
								sprite_sheets.reset();
								sprite_sheets = std::make_shared<std::vector<std::shared_ptr<roguely::sprites::SpriteSheet>>>();
								
								lua_init();
						}
						});

				lua.set_function("get_random_number", [&](int start, int end) {
						return std::rand() % end + start;
						});

				lua.set_function("get_entity_group_without", [&](std::string entity_group_name, std::string id) {
						//convert_entity_group_to_lua_table
						
						});

				auto lua_init = lua["_init"];

				if (lua_init.valid() && lua_init.get_type() == sol::type::function)
				{
						lua_init();
				}
								
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
								
								tick(logic_timer.elapsed_seconds, lua.lua_state());

								SDL_RenderClear(renderer);

								render(animation_timer.elapsed_seconds, lua.lua_state());

								SDL_RenderPresent(renderer);
						}
				}

				sprite_sheets.reset();

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