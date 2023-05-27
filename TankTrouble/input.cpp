#include <SDL.h>
#include <iostream>
#include <input.h>


void App::doKeyDown(SDL_KeyboardEvent* event)
{
	if (event->repeat == 0)
	{
		if (event->keysym.scancode == SDL_SCANCODE_UP)
		{
			up[0] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN)
		{
			down[0] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT)
		{
			left[0] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			right[0] = 1;
		}

		//player 2
		if (event->keysym.scancode == SDL_SCANCODE_W)
		{
			up[1] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_S)
		{
			down[1] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_A)
		{
			left[1] = 1;
		}

		if (event->keysym.scancode == SDL_SCANCODE_D)
		{
			right[1] = 1;
		}


		//other
		if (event->keysym.scancode == SDL_SCANCODE_BACKSPACE)
		{
			reset = 1;
		}
		if (event->keysym.scancode == SDL_SCANCODE_SPACE) {
			space[0] = 1;
		}
		if (event->keysym.scancode == SDL_SCANCODE_Q) {
			space[1] = 1;
		}
	}
}

void App::doKeyUp(SDL_KeyboardEvent* event)
{
	if (event->repeat == 0)
	{
		if (event->keysym.scancode == SDL_SCANCODE_UP)
		{
			up[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_DOWN)
		{
			down[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_LEFT)
		{
			left[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_RIGHT)
		{
			right[0] = 0;
		}

		//player2
		if (event->keysym.scancode == SDL_SCANCODE_W)
		{
			up[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_S)
		{
			down[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_A)
		{
			left[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_D)
		{
			right[1] = 0;
		}

		//other
		
		if (event->keysym.scancode == SDL_SCANCODE_SPACE) {
			space[0] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_Q) {
			space[1] = 0;
		}

		if (event->keysym.scancode == SDL_SCANCODE_BACKSPACE)
		{
			reset = 0;
		}
	}
}


void App::doInput() {
	if (userInputs) {
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
}
