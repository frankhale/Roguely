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

int Text::LoadFont(const char* path)
{
    font = TTF_OpenFont(path, 40);
    if (!font) {
        std::cout << "Unable to load font: " << path << std::endl
            << "SDL2_ttf Error: " << TTF_GetError() << std::endl;
        return -1;
    }
}

void Text::DrawText(SDL_Renderer* renderer, int x, int y, const char* text)
{    
    SDL_Rect text_rect;
    SDL_Surface* text_surface = TTF_RenderText_Shaded(font, text, text_color, text_background_color);
    text_rect.x = x;
    text_rect.y = y;
    text_rect.w = text_surface->w;
    text_rect.h = text_surface->h;

    text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);
    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
}
