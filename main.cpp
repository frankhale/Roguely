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
const bool MUSIC = true;

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
std::shared_ptr<Text> text = nullptr;
std::ostringstream player_health;
std::ostringstream player_score;
std::ostringstream enemies_killed;

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
		text = std::make_shared<Text>();
		text->LoadFont(FONT_PATH.c_str(), 40);
}

void render_game()
{
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
												sprite_sheet->drawSprite(renderer, 3, dx, dy);
										}
								}
								else
								{
										sprite_sheet->drawSprite(renderer, HIDDEN, dx, dy);
								}
						}
				}
		}
		else
		{
				text->DrawText(renderer, 10, 10, game->GetWinLoseMessage().c_str());
		}

		SDL_Rect info_panel_rect = { 20, WINDOW_HEIGHT - 175, WINDOW_WIDTH - 40,  150 };
		SDL_SetRenderDrawColor(renderer, 28, 28, 28, 192);		
		SDL_RenderFillRect(renderer, &info_panel_rect);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &info_panel_rect);

		player_health << "Health: " << game->GetPlayerHealth();
		player_score << "Score: " << game->GetPlayerScore();
		enemies_killed << "Enemies Killed: " << game->GetPlayerEnemiesKilled();

		text->DrawText(renderer, 35, WINDOW_HEIGHT - 8 * 20, player_health.str().c_str());
		text->DrawText(renderer, 35, WINDOW_HEIGHT - 6 * 20, player_score.str().c_str());
		text->DrawText(renderer, 35, WINDOW_HEIGHT - 4 * 20, enemies_killed.str().c_str());

		text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 8 * 20, game->GetPlayerCombatInfo().c_str());
		text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 6 * 20, game->GetEnemyStatInfo().c_str());
		text->DrawText(renderer, WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT - 4 * 20, game->GetEnemyCombatInfo().c_str());
}

int main(int argc, char* args[])
{
		if (init_sdl(WINDOW_TITLE) < 0)
		{
				return -1;
		}

		init_game();

		bool keep_window_open = true;
		while (keep_window_open)
		{
				SDL_Event e;
				while (SDL_PollEvent(&e) > 0)
				{
						switch (e.type)
						{
						case SDL_QUIT:
								keep_window_open = false;
								break;
						case SDL_KEYDOWN:
								if (e.key.keysym.sym == SDLK_UP)
								{
										game->MovePlayerUp();
								}
								else if (e.key.keysym.sym == SDLK_DOWN)
								{
										game->MovePlayerDown();
								}
								else if (e.key.keysym.sym == SDLK_LEFT)
								{
										game->MovePlayerLeft();
								}
								else if (e.key.keysym.sym == SDLK_RIGHT)
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

				render_game();

				SDL_RenderPresent(renderer);
				SDL_Delay(1000 / 60);
		}

		tear_down_sdl();

		return 0;
}
