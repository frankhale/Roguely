#pragma once

#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>

class Text
{
public:
		Text();		

		int LoadFont(const char* path, int ptsize);
		void DrawText(SDL_Renderer* renderer, int x, int y, const char* text);

private:
		TTF_Font* font;
		SDL_Texture* text_texture;

		SDL_Color text_color = { 0xFF, 0xFF, 0xFF, 0xFF };
		SDL_Color text_background_color = { 0x00, 0x00, 0x00, 0xFF };
};

