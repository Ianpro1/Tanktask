#include <SDL.h>
#include <common.h>
#include <SDL_image.h>
#include <defs.h>
#include <structs.h>
void prepareScene(void)
{
	SDL_SetRenderDrawColor(app.renderer, 255, 255, 255, 255);
	SDL_RenderClear(app.renderer);
}

void presentScene(void)
{
	SDL_RenderPresent(app.renderer);
}

SDL_Texture* loadTexture(const char* filename)
{
	SDL_Texture* texture;

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", filename);

	texture = IMG_LoadTexture(app.renderer, filename);

	return texture;
}

void blit(SDL_Texture* texture, SDL_Rect dstRect, float angle, SDL_Point center)
{
	SDL_Rect srcRect;
	srcRect.x = 0;
	srcRect.y = 0;
	SDL_QueryTexture(texture, NULL, NULL, &srcRect.w, &srcRect.h);

	SDL_RenderCopyEx(app.renderer, texture, &srcRect, &dstRect, angle * RADTODEG, &center, SDL_FLIP_NONE);
}

void drawBox(SDL_Texture* texture, SDL_Rect src, int x, int y, int h, int w, float rad, Uint8 red, Uint8 green, Uint8 blue) {
	
	SDL_Rect dst;
	SDL_SetTextureColorMod(texture, red, green, blue);

	dst.w = w;
	dst.h = h;
	dst.x = x;
	dst.y = y;


	SDL_RenderCopyEx(app.renderer, texture, &src, &dst, rad * RADTODEG, NULL, SDL_FLIP_NONE);
}