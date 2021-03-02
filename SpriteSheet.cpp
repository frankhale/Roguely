#include "SpriteSheet.h"

SpriteSheet::SpriteSheet(SDL_Renderer* renderer, const char* path, int sw, int sh)
{
		SDL_Surface* game_tileset = IMG_Load(path);
		auto t_color = SDL_MapRGB(game_tileset->format, 0, 0, 0);
		SDL_SetColorKey(game_tileset, SDL_TRUE, t_color);

		spritesheet_texture = SDL_CreateTextureFromSurface(renderer, game_tileset);
		SDL_FreeSurface(game_tileset);

		sprite_width = sw;
		sprite_height = sh;

		sprites[WALL] = { 0, 0, sprite_width, sprite_height };
		sprites[HEALTH_GEM] = { sprite_width, 0, sprite_width, sprite_height };
		sprites[ATTACK_BONUS_GEM] = { 2 * sprite_width, 0, sprite_width, sprite_height };
		sprites[PLAYER] = { 3 * sprite_width, 0, sprite_width, sprite_height  };
		sprites[SPIDER] = { 4 * sprite_width, 0, sprite_width, sprite_height };
		sprites[LURCHER] = { 5 * sprite_width, 0, sprite_width, sprite_height };
		sprites[GOLDEN_CANDLE] = { 6 * sprite_width, 0, sprite_width, sprite_height };
		sprites[DOWN_LADDER] = { 7 * sprite_width, 0, sprite_width, sprite_height };
		sprites[UP_LADDER] = { 8 * sprite_width, 0, sprite_width, sprite_height };
		sprites[GROUND] = { 9 * sprite_width, 0, sprite_width, sprite_height };
		sprites[WATER] = { 10 * sprite_width, 0, sprite_width, sprite_height };
		sprites[GRASS] = { 11 * sprite_width, 0, sprite_width, sprite_height };
		sprites[CRAB] = { 12 * sprite_width, 0, sprite_width, sprite_height };
		sprites[TREASURE_CHEST] = { 13 * sprite_width, 0, sprite_width, sprite_height };
		sprites[COIN] = { 14 * sprite_width, 0, sprite_width, sprite_height };				
		sprites[DOOR] = { 15 * sprite_width, 0, sprite_width, sprite_height };
		sprites[WALL_SECRET_DOOR] = { 0, 1 * sprite_height, sprite_width, sprite_height };
		sprites[BUG] = { sprite_width * 1, 1 * sprite_height, sprite_width, sprite_height };
		sprites[FIRE_WALKER] = { 5 * sprite_width, sprite_height, sprite_width, sprite_height };
		sprites[HIDDEN] = { sprite_width * 2, 1 * sprite_height, sprite_width, sprite_height };
		sprites[HEART] = { 0, 3 * sprite_height, sprite_width, sprite_height };
		sprites[CRIMSON_SHADOW] = { sprite_width * 2, 2 * sprite_height, sprite_width, sprite_height };
		sprites[WALL_WITH_GRASS_1] = { sprite_width * 3, 2 * sprite_height, sprite_width, sprite_height };
		sprites[WALL_WITH_GRASS_2] = { sprite_width * 4, 2 * sprite_height, sprite_width, sprite_height };
}

void SpriteSheet::drawSprite(SDL_Renderer* renderer, int sprite_id, int x, int y)
{		
		drawSprite(renderer, sprite_id, x, y, 0, 0);
}

void SpriteSheet::drawSprite(SDL_Renderer* renderer, int sprite_id, int x, int y, int scaled_width, int scaled_height)
{
		int width = sprite_width;
		int height = sprite_height;

		if (scaled_width > 0 && scaled_height > 0)
		{
				width = scaled_width;
				height = scaled_height;
		}

		SDL_Rect dest = { x, y, width, height };
		SDL_RenderCopy(renderer, spritesheet_texture, &sprites[sprite_id], &dest);
}

SpriteSheet::~SpriteSheet()
{
		SDL_DestroyTexture(spritesheet_texture);
}