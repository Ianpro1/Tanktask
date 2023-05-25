#pragma once

void prepareScene(void);

void presentScene(void);
void drawBox(SDL_Texture* texture, SDL_Rect src, int x, int y, int h, int w, float rad, Uint8 red, Uint8 green, Uint8 blue);

SDL_Texture* loadTexture(const char* filename);
void blit(SDL_Texture* texture, SDL_Rect dstRect, float angle, SDL_Point center);