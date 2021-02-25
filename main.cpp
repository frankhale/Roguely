#include <iostream>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "SpriteSheet.h"
#include "Text.h"
//#include <lua.hpp>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define WINDOW_ICON_PATH "assets/icon.png"
#define GAME_TILESET_PATH "assets/tileset2.png"
#define FONT_PATH "assets/VT323-Regular.ttf"

int map[10][10] = {
		{ 0,0,0,0,0,0,0,0,0,0 },
		{ 0,9,9,9,9,4,9,9,9,0 },
		{ 0,9,3,9,9,9,9,9,9,0 },
		{ 0,9,9,9,9,9,14,9,9,0 },
		{ 0,9,9,9,5,9,9,9,9,0 },
		{ 0,9,9,9,9,9,9,14,9,0 },
		{ 0,9,1,9,9,9,9,9,9,0 },
		{ 0,9,9,9,9,9,2,9,9,0 },
		{ 0,9,9,12,9,9,9,9,6,0 },
		{ 0,0,0,0,0,0,0,0,0,0 },
};

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

		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
				std::cout << "Failed to initialize the SDL2 library: " << SDL_GetError() << std::endl;
				return -1;
		}

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

		//int ww = 0;
		//int wh = 0;
		//SDL_GetWindowSize(window, &ww, &wh);

		//std::cout << "window width = " << ww << std::endl;
		//std::cout << "window height = " << wh << std::endl;

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

		auto sprite_sheet = std::make_shared<SpriteSheet>(renderer, GAME_TILESET_PATH);
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
										text->DrawText(renderer, 10, WINDOW_HEIGHT - 50, "Up Key Pressed");
								}
								else if (e.key.keysym.sym == SDLK_DOWN)
								{
										text->DrawText(renderer, 10, WINDOW_HEIGHT - 50, "Down Key Pressed");
								}
								else if (e.key.keysym.sym == SDLK_LEFT)
								{
										text->DrawText(renderer, 10, WINDOW_HEIGHT - 50, "Left Key Pressed");
								}
								else if (e.key.keysym.sym == SDLK_RIGHT)
								{
										text->DrawText(renderer, 10, WINDOW_HEIGHT - 50, "Right Key Pressed");
								}
								else if (e.key.keysym.sym == SDLK_SPACE)
								{
										text->DrawText(renderer, 10, WINDOW_HEIGHT - 50, "Space bar Pressed");
								}
								break;
						}

						text->DrawText(renderer, 10, 10, "Sprites in my spritesheet:");

						// Draw sprite sheet tiles
						for (int i = 0; i < sprite_sheet->totalSprites(); i++)
						{
								sprite_sheet->drawSprite(renderer, i, (10 + i * 24), 70);
						}

						text->DrawText(renderer, 10, 110, "Simple tilemap:");

						// Draw simple tile based map
						for (int r = 0; r < 10; r++)
						{
								for (int c = 0; c < 10; c++)
								{
										sprite_sheet->drawSprite(renderer, map[r][c], 10 + c * 24, 160 + r * 24);
								}
						}

						SDL_RenderPresent(renderer);

						SDL_Delay(1000/60);
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
