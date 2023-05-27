#include <SDL.h>
#include <SDL_image.h>
#include <defs.h>
#include <structs.h>
#include <draw.h>


void prepareScene(App* app)
{
	SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
	SDL_RenderClear(app->renderer);
}

void presentScene(App* app)
{
	SDL_RenderPresent(app->renderer);
}

SDL_Texture* loadTexture(App* app, const char* filename)
{
	SDL_Texture* texture;

	// Get the current working directory
	char* basePath = SDL_GetBasePath();

	// Concatenate the base path and the filename
	std::string fullPath = std::string(basePath) + std::string(filename);

	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "Loading %s", fullPath.c_str());

	// Load the texture using the full path
	texture = IMG_LoadTexture(app->renderer, fullPath.c_str());

	// Free the base path string
	SDL_free(basePath);

	return texture;
}

void blit(App* app, SDL_Texture* texture, SDL_Rect dstRect, float angle, SDL_Point center)
{
	SDL_Rect srcRect;
	srcRect.x = 0;
	srcRect.y = 0;
	SDL_QueryTexture(texture, NULL, NULL, &srcRect.w, &srcRect.h);

	SDL_RenderCopyEx(app->renderer, texture, &srcRect, &dstRect, angle * RADTODEG, &center, SDL_FLIP_NONE);
}

void drawBox(App* app, SDL_Texture* texture, SDL_Rect src, int x, int y, int h, int w, float rad, Uint8 red, Uint8 green, Uint8 blue) {
	
	SDL_Rect dst;
	SDL_SetTextureColorMod(texture, red, green, blue);

	dst.w = w;
	dst.h = h;
	dst.x = x;
	dst.y = y;


	SDL_RenderCopyEx(app->renderer, texture, &src, &dst, rad * RADTODEG, NULL, SDL_FLIP_NONE);
}