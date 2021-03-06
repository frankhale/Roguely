/*
* main.cpp
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

#include <array>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "Common.hpp"
#include "Game.hpp"
#include "SpriteSheet.hpp"
#include "Text.hpp"
//#include "LevelGeneration.hpp"

const std::string WINDOW_TITLE = "Roguely - A simple Roguelike in SDL and C++";
const std::string WINDOW_ICON_PATH = "assets/icon.png";
const std::string LOGO_PATH = "assets/roguely-logo.png";
const std::string START_GAME_PATH = "assets/press-space-bar-to-play.png";
const std::string CREDITS_PATH = "assets/credits.png";
const std::string TILESET_PATH = "assets/roguelike.png";
const std::string FONT_PATH = "assets/VT323-Regular.ttf";
const std::string MUSIC_PATH = "assets/ExitExitProper.mp3";

SDL_Window* window = nullptr;
SDL_Surface* window_surface = nullptr;
SDL_Renderer* renderer = nullptr;
Mix_Music* mix_music = nullptr;

std::shared_ptr<Game> game = nullptr;
std::shared_ptr<SpriteSheet> sprite_sheet = nullptr;
std::shared_ptr<Text> text_medium = nullptr;
std::shared_ptr<Text> text_large = nullptr;
std::shared_ptr<Text> text_small = nullptr;

std::ostringstream player_health_text;
std::ostringstream player_score_text;
std::ostringstream enemies_killed_text;

bool game_started = false;
bool dead = false;

int init_sdl(std::string window_title)
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

		if (MIX_INIT_MP3 != Mix_Init(MIX_INIT_MP3))
		{
				std::cout << "Failed to initialize the SDL2 mixer library\n";
				return -1;
		}

		window = SDL_CreateWindow(window_title.c_str(),
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				WINDOW_WIDTH, WINDOW_HEIGHT,
				SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		if (!window)
		{
				std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
				return -1;
		}

		window_surface = SDL_GetWindowSurface(window);

		SDL_Surface* window_icon_surface = IMG_Load(WINDOW_ICON_PATH.c_str());
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

void tear_down_sdl()
{
		if (MUSIC)
		{
				Mix_FreeMusic(mix_music);
		}

		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		Mix_Quit();
		TTF_Quit();
		IMG_Quit();
		SDL_Quit();
}

void play_soundtrack()
{
		Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
		mix_music = Mix_LoadMUS(MUSIC_PATH.c_str());
		Mix_Volume(-1, 3);
		Mix_VolumeMusic(3);
		Mix_PlayMusic(mix_music, 1);
}

void init_game()
{
		std::srand(static_cast<unsigned int>(std::time(nullptr)));

		game = std::make_shared<Game>();
		sprite_sheet = std::make_shared<SpriteSheet>(renderer, TILESET_PATH.c_str(), SPRITE_WIDTH, SPRITE_HEIGHT);
		text_medium = std::make_shared<Text>();
		text_medium->LoadFont(FONT_PATH.c_str(), 40);
		text_large = std::make_shared<Text>();
		text_large->LoadFont(FONT_PATH.c_str(), 63);
		text_small = std::make_shared<Text>();
		text_small->LoadFont(FONT_PATH.c_str(), 26);

		if (MUSIC)
		{
				play_soundtrack();
		}
}

void reset_game()
{
		game.reset();
		game = std::make_shared<Game>();
}

int calculate_health_bar_width(int health, int starting_health, int health_bar_max_width)
{
		int hw = health_bar_max_width;

		if (health < starting_health)
				hw = ((health * (100 / starting_health)) * health_bar_max_width) / 100;

		return hw;
}

void render_graphic(std::string path, int x, int y, bool centered, bool scaled, float scaled_factor)
{
		auto graphic = IMG_Load(path.c_str());
		auto graphic_texture = SDL_CreateTextureFromSurface(renderer, graphic);

		SDL_Rect dest = { x, y, graphic->w, graphic->h };

		if (centered)
				dest = { ((WINDOW_WIDTH / (2 + (int)scaled_factor)) - (graphic->w / 2)), y, graphic->w, graphic->h };

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

void render_title_screen(double delta_time)
{
		render_graphic(LOGO_PATH.c_str(), 0, 20, true, true, 2);

		sprite_sheet->drawSprite(renderer, ORANGE_BLOB, WINDOW_WIDTH / 2 - 256, 325);
		sprite_sheet->drawSprite(renderer, CRIMSON_SHADOW, WINDOW_WIDTH / 2 - 192, 325);
		sprite_sheet->drawSprite(renderer, SPIDER, WINDOW_WIDTH / 2 - 128, 325);
		sprite_sheet->drawSprite(renderer, LURCHER, WINDOW_WIDTH / 2 - 64, 325);
		sprite_sheet->drawSprite(renderer, PLAYER, WINDOW_WIDTH / 2, 325);
		sprite_sheet->drawSprite(renderer, CRAB, WINDOW_WIDTH / 2 + 64, 325);
		sprite_sheet->drawSprite(renderer, FIRE_WALKER, WINDOW_WIDTH / 2 + 128, 325);
		sprite_sheet->drawSprite(renderer, MANTIS, WINDOW_WIDTH / 2 + 192, 325);
		sprite_sheet->drawSprite(renderer, PURPLE_BLOB, WINDOW_WIDTH / 2 + 256, 325);

		render_graphic(START_GAME_PATH.c_str(), 0, 215, true, true, 2);
		render_graphic(CREDITS_PATH.c_str(), 0, 300, true, true, 2);
}

void render_win_or_death_screen(double delta_time)
{
		std::string win_lose_message = game->GetWinLoseMessage();

		auto text_extents = text_large->GetTextExtents(win_lose_message.c_str());

		text_large->DrawText(renderer, WINDOW_WIDTH / 2 - text_extents.width / 2, WINDOW_HEIGHT / 2 - text_extents.height / 2, win_lose_message.c_str());

		player_score_text << "Final Score: " << game->GetPlayerScore();
		text_large->DrawText(renderer, 20, 20, player_score_text.str().c_str());

		enemies_killed_text << "Total Enemies Killed: " << game->GetPlayerEnemiesKilled();
		text_large->DrawText(renderer, 20, 70, enemies_killed_text.str().c_str());

		text_medium->DrawText(renderer, WINDOW_WIDTH / 2 - text_extents.width / 2 - 250, WINDOW_HEIGHT - 100, "Press the space bar to return to the title screen...");
}

void render_game(double delta_time)
{
		static double combat_log_render = .0f;

		for (int r = 0; r < game->GetViewPortHeight(); r++)
		{
				for (int c = 0; c < game->GetViewPortWidth(); c++)
				{
						int dx = (c * SPRITE_WIDTH) - (game->GetViewPortX() * SPRITE_WIDTH);
						int dy = (r * SPRITE_HEIGHT) - (game->GetViewPortY() * SPRITE_HEIGHT);

						if ((*game->LightMap())[r][c] == 2)
						{
								sprite_sheet->drawSprite(renderer, (*game->Map())[r][c], dx, dy);

								for (auto& elem : **game->GetCoins())
								{
										if (elem.point.x == c &&
												elem.point.y == r)
										{
												sprite_sheet->drawSprite(renderer, COIN, dx, dy);
										}
								}

								for (auto& elem : **game->GetHealthGems())
								{
										if (elem.point.x == c &&
												elem.point.y == r)
										{
												sprite_sheet->drawSprite(renderer, HEALTH_GEM, dx, dy);
										}
								}

								for (auto& elem : **game->GetEnemies())
								{
										if (elem.point.x == c &&
												elem.point.y == r)
										{
												int enemy_id = 0;

												if (elem.id == (int)EntitySubType::Spider) enemy_id = SPIDER;
												else if (elem.id == (int)EntitySubType::Lurcher)enemy_id = LURCHER;
												else if (elem.id == (int)EntitySubType::Crab)enemy_id = CRAB;
												else if (elem.id == (int)EntitySubType::Bug)enemy_id = BUG;
												else if (elem.id == (int)EntitySubType::Fire_Walker)enemy_id = FIRE_WALKER;
												else if (elem.id == (int)EntitySubType::Crimson_Shadow)enemy_id = CRIMSON_SHADOW;
												else if (elem.id == (int)EntitySubType::Purple_Blob)enemy_id = PURPLE_BLOB;

												auto enemy_stat_component = game->find_component<StatComponent>(elem.components);

												if (enemy_stat_component != nullptr)
												{
														auto e_hw = calculate_health_bar_width(enemy_stat_component->GetHealth(), enemy_stat_component->GetStartingHealth(), 32);

														SDL_Rect health_panel_rect = { dx, dy - 8, e_hw, 6 };
														SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red for enemy
														SDL_RenderFillRect(renderer, &health_panel_rect);
												}

												sprite_sheet->drawSprite(renderer, enemy_id, dx, dy);
										}
								}

								for (auto& elem : **game->GetTreasureChests())
								{
										if (elem.point.x == c &&
												elem.point.y == r)
										{
												sprite_sheet->drawSprite(renderer, TREASURE_CHEST, dx, dy);
										}
								}

								for (auto& elem : **game->GetBonus())
								{
										if (elem.point.x == c &&
												elem.point.y == r)
										{
												// TODO: Bonus can be anything but right now it's only attack gems
												sprite_sheet->drawSprite(renderer, ATTACK_BONUS_GEM, dx, dy);
										}
								}

								auto golden_candle = game->GetGoldenCandle();
								if (golden_candle.point.x == c && golden_candle.point.y == r)
								{
										sprite_sheet->drawSprite(renderer, GOLDEN_CANDLE, dx, dy);
								}

								if (r == game->GetPlayerY() && c == game->GetPlayerX())
								{
										auto p_hw = calculate_health_bar_width(game->GetPlayerHealth(), game->GetPlayerStartingHealth(), 32);

										SDL_Rect health_panel_rect = { dx, dy - 8, p_hw, 6 };
										SDL_SetRenderDrawColor(renderer, 8, 138, 41, 255); // green for player
										SDL_RenderFillRect(renderer, &health_panel_rect);

										sprite_sheet->drawSprite(renderer, 3, dx, dy);
								}

								if (game->GetCombatLog()->size() > 0)
								{
										auto combat_log = game->GetCombatLog()->front();
										auto c_x = (combat_log->point.x * SPRITE_WIDTH) - (game->GetViewPortX() * SPRITE_WIDTH);
										auto c_y = (combat_log->point.y * SPRITE_HEIGHT) - (game->GetViewPortY() * SPRITE_HEIGHT);

										SDL_Color combat_log_color{};
										if (combat_log->who == WhoAmI::Player)
										{
												combat_log_color = { 0, 255, 0, 255 };
										}
										else if (combat_log->who == WhoAmI::Enemy)
										{
												combat_log_color = { 255, 0, 0, 255 };
										}

										text_small->DrawText(renderer, c_x, c_y - 36, combat_log->message.c_str(), combat_log_color);

										if (combat_log_render >= .4) {
												game->GetCombatLog()->pop();
												combat_log_render = .0f;
										}
								}
						}
						else
						{
								sprite_sheet->drawSprite(renderer, HIDDEN, dx, dy);
						}
				}
		}

		SDL_Rect info_panel_rect = { 10, 10, 290,  150 };
		SDL_SetRenderDrawColor(renderer, 28, 28, 28, 128);
		SDL_RenderFillRect(renderer, &info_panel_rect);
		/*SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &info_panel_rect);*/

		sprite_sheet->drawSprite(renderer, HEART, 20, 20, SPRITE_WIDTH * 2, SPRITE_HEIGHT * 2);

		auto p_hw = calculate_health_bar_width(game->GetPlayerHealth(), game->GetPlayerStartingHealth(), 200);

		SDL_Rect health_panel_rect = { (SPRITE_WIDTH * 2 + 20), 36, p_hw, 24 };
		SDL_Rect health_panel_rect_underlay = { (SPRITE_WIDTH * 2 + 20), 36, 200, 24 };

		SDL_SetRenderDrawColor(renderer, 33, 33, 33, 255);
		SDL_RenderFillRect(renderer, &health_panel_rect_underlay);

		if (game->GetPlayerHealth() <= game->GetPlayerStartingHealth() / 3)
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red player's health is in trouble
		else
				SDL_SetRenderDrawColor(renderer, 8, 138, 41, 255); // green for player

		SDL_RenderFillRect(renderer, &health_panel_rect);

		player_health_text << game->GetPlayerHealth();
		player_score_text << game->GetPlayerScore();
		text_medium->DrawText(renderer, (SPRITE_WIDTH * 3 + 70), 28, player_health_text.str().c_str());
		text_large->DrawText(renderer, 40, 90, player_score_text.str().c_str());

		//enemies_killed << "Enemies Killed: " << game->GetPlayerEnemiesKilled();
		/*text->DrawText(renderer, 35, WINDOW_HEIGHT - 4 * 20, enemies_killed.str().c_str());

		text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 8 * 20, game->GetPlayerCombatInfo().c_str());
		text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 6 * 20, game->GetEnemyStatInfo().c_str());
		text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 4 * 20, game->GetEnemyCombatInfo().c_str());*/

		combat_log_render += delta_time;
}

void render_mini_map(std::shared_ptr<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>> map, Point player_position)
{
		for (int r = 0; r < MAP_HEIGHT; r++)
		{
				for (int c = 0; c < MAP_WIDTH; c++)
				{
						int dx = c + (WINDOW_WIDTH - 150);
						int dy = r + 10;

						if ((*map)[r][c] == 0)
						{
								SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
								SDL_RenderDrawPoint(renderer, dx, dy);
						}
						else if ((*map)[r][c] == 9)
						{
								SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
								SDL_RenderDrawPoint(renderer, dx, dy);
						}

						if (dx == player_position.x + (WINDOW_WIDTH - 150) && dy == player_position.y + 10)
						{
								SDL_Rect player_rect = { dx - 3, dy - 3, 6, 6 };
								SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
								SDL_RenderFillRect(renderer, &player_rect);
						}
				}
		}
}

//void render_sandbox(double delta_time, std::shared_ptr<std::array<std::array<int, MAP_HEIGHT>, MAP_WIDTH>> map)
//{
//		for (int r = 0; r < MAP_HEIGHT; r++)
//		{
//				for (int c = 0; c < MAP_WIDTH; c++)
//				{
//						int dx = (c * 5) + (WINDOW_WIDTH / 2) - (MAP_WIDTH * 2 + 75);
//						int dy = (r * 5) + (WINDOW_HEIGHT / 2) - (MAP_HEIGHT * 2 + 75);
//
//						SDL_Rect rect = { dx, dy, 5, 5 };
//
//						if ((*map)[c][r] == 0)
//						{
//								SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
//								SDL_RenderFillRect(renderer, &rect);
//						}
//						else if ((*map)[c][r] == 1)
//						{
//								SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
//								SDL_RenderDrawRect(renderer, &rect);
//						}
//				}
//		}
//}

int main(int argc, char* args[])
{
		// Timer code from : https://gamedev.stackexchange.com/a/163508/18014
		const int UPDATE_FREQUENCY{ 60 };
		const float CYCLE_TIME{ 1.0f / UPDATE_FREQUENCY };
		static Timer system_timer;
		float accumulated_seconds{ 0.0f };

		if (init_sdl(WINDOW_TITLE) < 0)
		{
				return -1;
		}

		init_game();

		bool keep_window_open = true;
		while (keep_window_open)
		{
				// Update clock
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
										game->MovePlayerUp();
								}
								else if (e.key.keysym.sym == SDLK_DOWN ||
										e.key.keysym.sym == SDLK_s)
								{
										game->MovePlayerDown();
								}
								else if (e.key.keysym.sym == SDLK_LEFT ||
										e.key.keysym.sym == SDLK_a)
								{
										game->MovePlayerLeft();
								}
								else if (e.key.keysym.sym == SDLK_RIGHT ||
										e.key.keysym.sym == SDLK_d)
								{
										game->MovePlayerRight();
								}
								else if (e.key.keysym.sym == SDLK_SPACE)
								{
										if (dead)
										{
												dead = false;
												game_started = false;
												reset_game();
										}
										else if (!game_started)
										{
												game_started = true;
										}

										//auto m = init_cellular_automata();
										//the_map = m;
										//perform_cellular_automaton(the_map, 10);
								}
								break;
						}
				}

				player_health_text.str("");
				player_score_text.str("");
				enemies_killed_text.str("");

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);

				static Timer animation_timer;
				static Timer logic_timer;

				while (std::isgreater(accumulated_seconds, CYCLE_TIME))
				{
						accumulated_seconds = -CYCLE_TIME;
						logic_timer.tick();
						animation_timer.tick();

						if (!game_started)
						{
								render_title_screen(animation_timer.elapsed_seconds);
						}
						else if (strlen(game->GetWinLoseMessage().c_str()) > 1)
						{
								dead = true;
								render_win_or_death_screen(animation_timer.elapsed_seconds);
						}
						else
						{
								game->HandleLogicTimer(logic_timer.elapsed_seconds);
								render_game(animation_timer.elapsed_seconds);
								render_mini_map(game->GetMap(), game->GetPlayerPoint());
						}

						// Testing cellular automata generation
						//render_sandbox(renderer, animation_timer.elapsed_seconds);

						SDL_RenderPresent(renderer);
				}
		}

		tear_down_sdl();

		return 0;
}
