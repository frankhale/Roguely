#include <iostream>
#include <sstream>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "SpriteSheet.h"
#include "Text.h"
#include "Game.h"
//#include <lua.hpp>

const int WINDOW_WIDTH = 1056;
const int WINDOW_HEIGHT = 768;

#define WINDOW_ICON_PATH "assets/icon.png"
#define GAME_TILESET_PATH "assets/tileset2.png"
#define FONT_PATH "assets/VT323-Regular.ttf"

//int map[10][10] = {
//		{ 0,0,0,0,0,0,0,0,0,0 },
//		{ 0,9,9,9,9,4,9,9,9,0 },
//		{ 0,9,3,9,9,9,9,9,9,0 },
//		{ 0,9,9,9,9,9,14,9,9,0 },
//		{ 0,9,9,9,5,9,9,9,9,0 },
//		{ 0,9,9,9,9,9,9,14,9,0 },
//		{ 0,9,1,9,9,9,9,9,9,0 },
//		{ 0,9,9,9,9,9,2,9,9,0 },
//		{ 0,9,9,12,9,9,9,9,6,0 },
//		{ 0,0,0,0,0,0,0,0,0,0 },
//};

int main(int argc, char* args[])
{
		//std::string lua_code = "a = 2 + 2";
		//lua_State* L = luaL_newstate();
		//int r = luaL_dostring(L, lua_code.c_str());

		//if (r == LUA_OK)
		//{
		//		lua_getglobal(L, "a");
		//		if (lua_isnumber(L, -1))
		//		{
		//				std::cout << lua_tonumber(L, -1) << std::endl;
		//		}
		//}
		//else
		//{
		//		std::cout << lua_tostring(L, -1) << std::endl;
		//}

		//if (SDL_Init(SDL_INIT_VIDEO) < 0)
		//{
		//		std::cout << "Failed to initialize the SDL2 library: " << SDL_GetError() << std::endl;
		//		return -1;
		//}

		IMG_Init(IMG_INIT_PNG);
		TTF_Init();

		SDL_Window* window = SDL_CreateWindow("SDL2 Roguelike",
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED,
				WINDOW_WIDTH, WINDOW_HEIGHT,
				SDL_WINDOW_OPENGL);

		if (!window)
		{
				std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
				return -1;
		}

		SDL_Surface* window_surface = SDL_GetWindowSurface(window);
		SDL_Surface* window_icon_surface = IMG_Load(WINDOW_ICON_PATH);
		SDL_SetWindowIcon(window, window_icon_surface);

		if (!window_surface)
		{
				std::cout << "Failed to get the surface from the window: " << SDL_GetError() << std::endl;
				return -1;
		}

		SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
		SDL_Renderer* icon_renderer = SDL_CreateRenderer(window, -1, 0);

		auto game = std::make_shared<Game>();
		auto sprite_sheet = std::make_shared<SpriteSheet>(renderer, GAME_TILESET_PATH, 24, 24);
		auto text = std::make_shared<Text>();
		text->LoadFont(FONT_PATH, 40);

		bool keep_window_open = true;
		while (keep_window_open)
		{
				SDL_Event e;
				while (SDL_PollEvent(&e) > 0)
				{
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
						SDL_RenderClear(renderer);

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
								/*else if (e.key.keysym.sym == SDLK_SPACE)
								{
										text->DrawText(renderer, 10, WINDOW_HEIGHT - 50, "Space bar Pressed");
								}*/
								break;
						}

						for (int r = 0; r < game->GetViewPortHeight(); r++)
						{
								for (int c = 0; c < game->GetViewPortWidth(); c++)
								{
										int dx = (c * 24) - (game->GetViewPortX() * 24);
										int dy = (r * 24) - (game->GetViewPortY() * 24);

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
																
																sprite_sheet->drawSprite(renderer, enemy_id, dx, dy);
														}
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

						std::ostringstream player_health;
						std::ostringstream player_score;
						std::ostringstream enemies_killed;
						player_health << "Health: " << game->GetPlayerHealth();						
						player_score << "Score: " << game->GetPlayerScore();
						enemies_killed << "Enemies Killed: " << game->GetPlayerEnemiesKilled();

						text->DrawText(renderer, 10, WINDOW_HEIGHT - 7 * 20, player_health.str().c_str());
						text->DrawText(renderer, 10, WINDOW_HEIGHT - 5 * 20, player_score.str().c_str());
						text->DrawText(renderer, 10, WINDOW_HEIGHT - 3 * 20, enemies_killed.str().c_str());

						text->DrawText(renderer, 450, WINDOW_HEIGHT - 7 * 20, game->GetEnemyStatInfo()->str().c_str());

						SDL_RenderPresent(renderer);
						SDL_Delay(1000 / 60);
				}
		}

		SDL_FreeSurface(window_icon_surface);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyRenderer(icon_renderer);
		SDL_DestroyWindow(window);

		TTF_Quit();
		IMG_Quit();
		SDL_Quit();

		return 0;
}
