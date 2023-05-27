#pragma once

void prepareScene(App* app);
void presentScene(App* app);
void drawBox(App* app, SDL_Texture* texture, SDL_Rect src, int x, int y, int h, int w, float rad, Uint8 red, Uint8 green, Uint8 blue);

SDL_Texture* loadTexture(App* app, const char* filename);
void blit(App* app, SDL_Texture* texture, SDL_Rect dstRect, float angle, SDL_Point center);

