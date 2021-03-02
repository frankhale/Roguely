#include <iostream>
#include <sstream>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include "SpriteSheet.h"
#include "Text.h"
#include "Game.h"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 768;
const int SPRITE_WIDTH = 32;
const int SPRITE_HEIGHT = 32;
const bool MUSIC = false;

const std::string WINDOW_TITLE = "Simple SDL2 based Roguelike";
const std::string WINDOW_ICON_PATH = "assets/icon.png";
const std::string GAME_TILESET_PATH = "assets/roguelike.png";
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

std::ostringstream player_health;
std::ostringstream player_score;
std::ostringstream enemies_killed;

// ref: https://gamedev.stackexchange.com/a/163508/18014
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

		if (MUSIC)
		{
				Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096);
				mix_music = Mix_LoadMUS(MUSIC_PATH.c_str());
				Mix_Volume(-1, 5);
				Mix_VolumeMusic(5);
				Mix_PlayMusic(mix_music, 1);
		}

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

void init_game()
{
		game = std::make_shared<Game>();
		sprite_sheet = std::make_shared<SpriteSheet>(renderer, GAME_TILESET_PATH.c_str(), SPRITE_WIDTH, SPRITE_HEIGHT);
		text_medium = std::make_shared<Text>();
		text_medium->LoadFont(FONT_PATH.c_str(), 40);
		text_large = std::make_shared<Text>();
		text_large->LoadFont(FONT_PATH.c_str(), 63);
		text_small = std::make_shared<Text>();
		text_small->LoadFont(FONT_PATH.c_str(), 26);
}

int calculate_health_bar_width(int health, int starting_health, int health_bar_max_width)
{
		int hw = health_bar_max_width;

		if (health < starting_health)
				hw = ((health * (100 / starting_health)) * health_bar_max_width) / 100;

		return hw;
}

void render_game(double delta_time)
{
		static double combat_log_render = .0f;

		if (strlen(game->GetWinLoseMessage().c_str()) <= 1)
		{
				for (int r = 0; r < game->GetViewPortHeight(); r++)
				{
						for (int c = 0; c < game->GetViewPortWidth(); c++)
						{
								int dx = (c * SPRITE_WIDTH) - (game->GetViewPortX() * SPRITE_WIDTH);
								int dy = (r * SPRITE_HEIGHT) - (game->GetViewPortY() * SPRITE_HEIGHT);

								if (game->LightMap()[r][c] == 2)
								{
										sprite_sheet->drawSprite(renderer, game->Map()[r][c], dx, dy);

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

														if (elem.id == 50) enemy_id = SPIDER;
														else if (elem.id == 51)enemy_id = LURCHER;
														else if (elem.id == 52)enemy_id = CRAB;
														else if (elem.id == 53)enemy_id = BUG;
														else if (elem.id == 54)enemy_id = FIRE_WALKER;

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

												if (combat_log_render >= .25) {
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
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderDrawRect(renderer, &info_panel_rect);

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

				player_health << game->GetPlayerHealth();
				player_score << game->GetPlayerScore();
				text_medium->DrawText(renderer, (SPRITE_WIDTH * 3 + 70), 28, player_health.str().c_str());
				text_large->DrawText(renderer, 40, 90, player_score.str().c_str());

				//enemies_killed << "Enemies Killed: " << game->GetPlayerEnemiesKilled();
				/*text->DrawText(renderer, 35, WINDOW_HEIGHT - 4 * 20, enemies_killed.str().c_str());

				text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 8 * 20, game->GetPlayerCombatInfo().c_str());
				text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 6 * 20, game->GetEnemyStatInfo().c_str());
				text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 4 * 20, game->GetEnemyCombatInfo().c_str());*/
		}
		else
		{
				text_large->DrawText(renderer, WINDOW_WIDTH / 2 - (static_cast<int>(game->GetWinLoseMessage().length()) * 13), WINDOW_HEIGHT / 2 - 80, game->GetWinLoseMessage().c_str());
		
				player_score << "Final Score: " << game->GetPlayerScore();
				text_large->DrawText(renderer, 20, 20, player_score.str().c_str());

				enemies_killed << "Total Enemies Killed: " << game->GetPlayerEnemiesKilled();
				text_large->DrawText(renderer, 20, 70, enemies_killed.str().c_str());
		}

		combat_log_render += delta_time;
}

int main(int argc, char* args[])
{
		// ref: https://gamedev.stackexchange.com/a/163508/18014
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
								break;
						}
				}

				player_health.str("");
				player_score.str("");
				enemies_killed.str("");

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);

				static Timer animation_timer;
				while (std::isgreater(accumulated_seconds, CYCLE_TIME))
				{
						// Reset the accumulator
						accumulated_seconds = -CYCLE_TIME;

						animation_timer.tick();

						render_game(animation_timer.elapsed_seconds);

						SDL_RenderPresent(renderer);
				}
		}

		tear_down_sdl();

		return 0;
}
