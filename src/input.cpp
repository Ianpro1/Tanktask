#include <SDL.h>
#include <common.h>
#include <iostream>
void doKeyDown(SDL_KeyboardEvent* event)
{
	if (event->repeat == 0)
	{
		if (event->keysym.scancode == SDL_SCANCODE_UP)
		{
			app.up[0] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN)
		{
			app.down[0] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT)
		{
			app.left[0] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			app.right[0] = 1;
		}

		//player 2
		if (event->keysym.scancode == SDL_SCANCODE_W)
		{
			app.up[1] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_S)
		{
			app.down[1] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_A)
		{
			app.left[1] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_D)
		{
			app.right[1] = 1;
		}


		//other
		if (event->keysym.scancode == SDL_SCANCODE_BACKSPACE)
		{
			app.reset = 1;
		}
		if (event->keysym.scancode == SDL_SCANCODE_SPACE) {
			app.space[0] = 1;
		}
		if (event->keysym.scancode == SDL_SCANCODE_Q) {
			app.space[1] = 1;
		}
	}
}

void doKeyUp(SDL_KeyboardEvent* event)
{
	if (event->repeat == 0)
	{
		if (event->keysym.scancode == SDL_SCANCODE_UP)
		{
			app.up[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN)
		{
			app.down[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT)
		{
			app.left[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			app.right[0] = 0;
		}

		//player2
		if (event->keysym.scancode == SDL_SCANCODE_W)
		{
			app.up[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_S)
		{
			app.down[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_A)
		{
			app.left[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_D)
		{
			app.right[1] = 0;
		}

		//other
		
		if (event->keysym.scancode == SDL_SCANCODE_SPACE) {
			app.space[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_Q) {
			app.space[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_BACKSPACE)
		{
			app.reset = 0;
		}
	}
}

void doInput(void) {

	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			exit(0);
			break;

		case SDL_KEYDOWN:
			doKeyDown(&event.key);
			break;

		case SDL_KEYUP:
			doKeyUp(&event.key);
			break;

		default:
			break;
		}
	}
}
