#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "SpriteSheet.h"
#include "Text.h"

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
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Failed to initialize the SDL2 library\n";
        return -1;
    }

    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("SDL2 Roguelike",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        900, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        std::cout << "Failed to create window\n";
        return -1;
    }

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    if (!window_surface)
    {
        std::cout << "Failed to get the surface from the window\n";
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Renderer* icon_renderer = SDL_CreateRenderer(window, -1, 0);

    auto sprite_sheet = std::make_shared<SpriteSheet>(renderer, GAME_TILESET_PATH);
    auto text = std::make_shared<Text>();
    text->LoadFont(FONT_PATH);

    SDL_Surface* window_icon_surface = IMG_Load(WINDOW_ICON_PATH);
    SDL_SetWindowIcon(window, window_icon_surface);

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
            }

            text->DrawText(renderer, 10, 10, "Sprites in my spritesheet:");

            // Draw sprite sheet tiles
            for (int i = 0; i < 16; i++)
            {
                sprite_sheet->drawSprite(renderer, i, (10 + i * 24), 70);
            }

            text->DrawText(renderer, 10, 110, "Simple tilemap:");

            for (int r = 0; r < 10; r++)
            {
                for (int c = 0; c < 10; c++)
                {
                    sprite_sheet->drawSprite(renderer, map[r][c], 10+c*24, 160+r*24);
                }
            }

            SDL_RenderPresent(renderer);            
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
