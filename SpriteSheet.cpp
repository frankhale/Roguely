#include "SpriteSheet.h"

SpriteSheet::SpriteSheet(SDL_Renderer* renderer, const char* path)
{
		SDL_Surface* game_tileset = IMG_Load(path);
		spritesheet_texture = SDL_CreateTextureFromSurface(renderer, game_tileset);
		SDL_FreeSurface(game_tileset);

		const int width = 24;
		const int height = 24;

		sprites[WALL] = { 0, 0, width, height };
		sprites[HEALTH_GEM] = { 24, 0, width, height };
		sprites[ATTACK_BONUS_GEM] = { 2 * 24, 0, width, height };
		sprites[PLAYER] = { 3 * 24, 0, width, height  };
		sprites[SPIDER] = { 4 * 24, 0, width, height };
		sprites[LURCHER] = { 5 * 24, 0, width, height };		
		sprites[GOLDEN_CANDLE] = { 6 * 24, 0, width, height };
		sprites[DOWN_LADDER] = { 7 * 24, 0, width, height };
		sprites[UP_LADDER] = { 8 * 24, 0, width, height };
		sprites[GROUND] = { 9 * 24, 0, width, height };
		sprites[WATER] = { 10 * 24, 0, width, height };
		sprites[GRASS] = { 11 * 24, 0, width, height };
		sprites[CRAB] = { 12 * 24, 0, width, height };
		sprites[TREASURE_CHEST] = { 13 * 24, 0, width, height };
		sprites[COIN] = { 14 * 24, 0, width, height };
		sprites[DOOR] = { 15 * 24, 0, width, height };
}

void SpriteSheet::drawSprite(SDL_Renderer* renderer, int sprite_id, int x, int y)
{
		SDL_Rect dest = { x, y, 24, 24 };
		SDL_RenderCopy(renderer, spritesheet_texture, &sprites[sprite_id], &dest);
}

SpriteSheet::~SpriteSheet()
{
		SDL_DestroyTexture(spritesheet_texture);
}