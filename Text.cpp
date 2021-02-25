#include "Text.h"

Text::Text()
{
    font = nullptr;
    text_texture = nullptr;
}

Text::~Text()
{
    SDL_DestroyTexture(text_texture);
}

int Text::LoadFont(const char* path, int ptsize)
{
    font = TTF_OpenFont(path, ptsize);
    if (!font) {
        std::cout << "Unable to load font: " << path << std::endl
            << "SDL2_ttf Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    return 0;
}

void Text::DrawText(SDL_Renderer* renderer, int x, int y, const char* text)
{    
    text_texture = nullptr;

    SDL_Rect text_rect;
    SDL_Surface* text_surface = TTF_RenderText_Blended(font, text, text_color);
    text_rect.x = x;
    text_rect.y = y;
    text_rect.w = text_surface->w;
    text_rect.h = text_surface->h;

    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);    
        
    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);    
    SDL_FreeSurface(text_surface);
}
