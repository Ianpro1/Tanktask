#include <SDL.h>
#include <defs.h>
#include <SDL_image.h>
#include <init.h>
using namespace std;

void initSDL(App* app) {

	int rendererFlags, windowFlags;

	rendererFlags = SDL_RENDERER_ACCELERATED;

	windowFlags = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {

		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	app->window = SDL_CreateWindow("Shooter 01", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, windowFlags);

	if (!app->window)
	{
		printf("Failed to open %d x %d window: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_GetError());
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	app->renderer = SDL_CreateRenderer(app->window, -1, rendererFlags);

	if (!app->renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}

	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
}

double getRand() {
	//return double value between [0, 0.99996]
	return (double)rand() / (double)(RAND_MAX + 1);
}

std::vector<mazeWall> InitMaze(int mazeSize) {
	b2Timer timer;
	std::vector<int> completeMaze;
	std::vector<mazeWall> maze;
	int fullcount = mazeSize * mazeSize;
	int requiredcount = fullcount;
	//init maze and unbreakable walls
	std::vector<int> wbreaks;
	wbreaks.push_back(-2);
	wbreaks.push_back(-3);
	maze.push_back(mazeWall(wbreaks));

	for (int i = 1; i < fullcount; i++) {
		wbreaks.clear();

		if (fmod(i, mazeSize) == 0) {
			wbreaks.push_back(-2);
		}
		if (fmod(i + 1, mazeSize) == 0) {
			wbreaks.push_back(-4);
		}
		if (i < mazeSize) {
			wbreaks.push_back(-3);
		}
		if (i >= fullcount - mazeSize) {
			wbreaks.push_back(-1);
		}
		maze.push_back(mazeWall(wbreaks));
	}

	//random blocks
	{
		double prob = getRand();
		double ratio = 0.20;

		prob = ratio * prob;
		int nblocks = floor(prob * fullcount);
		std::vector<int> block_ids;

		for (int i = 0; i < nblocks;) {
			double prob = getRand();
			int id = floor(prob * fullcount);

			if (std::find(block_ids.begin(), block_ids.end(), id) != block_ids.end()) {
				continue;
			}
			else {
				i++;
				block_ids.push_back(id);
				requiredcount -= 1;
				(maze)[id].path = true;
				for (int k = 0; k < 4; k++)
					(maze)[id].wallBreaks[k] = -1;
			}
		}
	}
		/*
		//figure out if blocks are enclosing any maze tile
		std::vector<int> blocks_below;
		for (int i = 0; i < block_ids.size(); i++) {
			int thisBlock = block_ids[i];
			if (thisBlock < fullcount-mazeSize && std::find(block_ids.begin(), block_ids.end(), thisBlock + mazeSize) == block_ids.end()){
				//figure out which blocks belong to the same column as the starting block
				int column = fmod(thisBlock, mazeSize);
				int mazetile = -1;
				blocks_below.clear();
				for (int b : block_ids) {
					if (fmod(b, mazeSize) == column && b > thisBlock) {
						blocks_below.push_back(b);
					}
				}

				//start loop

				int angle = 0; // 7 angles total
				int cur = thisBlock;
				int next;
				while (1) {

					switch (angle) {
						case 0:
							next = 1 + cur;
							break;
						case 1:
							angle = 2;
							next = cur +(1 + mazeSize);
							break;
						case 2:
							next = cur + mazeSize;
							break;
						case 3:
							angle = 4;
							next = cur + (mazeSize - 1);
							break;
						case 4:
							next = cur - 1;
							break;
						case 5:
							angle = 6;
							next = cur - (1 + mazeSize);
							break;
						case 6:
							next = cur - mazeSize;
							break;
						case 7:
							angle = 0;
							next = cur - (mazeSize - 1);
							break;
						default:
							throw std::invalid_argument("invalid angle!");
					}
					//don't forget to look at walls instead of changing the angle
					if (std::find(block_ids.begin(), block_ids.end(), next) == block_ids.end()) {
						break;
					}
					else {

					}
				}
			}
		}
	}
	*/
	//remove random walls
	{
		double prob = getRand();
		double ratio = 0.30;

		prob = ratio * prob;
		int ndel = floor(prob * fullcount);

		for (int i = 0; i < ndel;) {
			double prob2 = getRand();
			double prob3 = getRand();
			int id = floor(prob3 * fullcount);
			int side = floor(prob2 * 4);

			if ((maze)[id].wallBreaks[side] != -1) {
				(maze)[id].wallBreaks[side] = 1;
				i++;
			}
			
		}
	}
	
	//init first 2 walls
	while (1){
		double prob = getRand();
		double prob2 = getRand();
		int id = floor(prob * fullcount);
		int type = floor(4 * prob2);

		if ((maze)[id].wallBreaks[type] != -1) {

			(maze)[id].wallBreaks[type] = 1;
			(maze)[id].path = true;
			completeMaze.push_back(id);

			if (type == 0) {
				int other = id + 1;
				(maze)[other].path = true;
				completeMaze.push_back(other);
			}
			else if (type == 2) {
				int other = id - 1;
				(maze)[other].path = true;
				completeMaze.push_back(other);
			}
			else if (type == 1) {
				int other = id + mazeSize;
				(maze)[other].path = true;
				completeMaze.push_back(other);
			}
			else if (type == 3) {
				int other = id - mazeSize;
				(maze)[other].path = true;
				completeMaze.push_back(other);
			}
			else {
				throw std::invalid_argument("type of wallBreak is unknown");
			}
			break;
		}
	}
	
	//loop
	timer.Reset();
	while (completeMaze.size() < requiredcount) {
		while (1) {

			double prob = getRand();
			double prob2 = getRand();
			prob = pow(prob, 0.4);
			//prob = pow(prob, 2);

			int mazefullsize = completeMaze.size();
			int comp_id = floor(prob * mazefullsize);
			int comp_type = floor(4 * prob2);

			int id = completeMaze[comp_id];
			int other;

			switch (comp_type) {

			case 0:
				other = id + 1;
				break;
			case 1:
				other = id + mazeSize;
				break;
			case 2:
				other = id - 1;
				break;
			case 3:
				other = id - mazeSize;
				break;
			default:
				throw std::invalid_argument("type of wallBreak is unknown");
			}

			if ((maze)[id].wallBreaks[comp_type] != -1 && (maze)[other].path != true) {
				(maze)[other].path = true;
				(maze)[id].wallBreaks[comp_type] = 1;
				completeMaze.push_back(other);
				break;
			}

			if (timer.GetMilliseconds() > 50) {
				maze.clear();
				return maze;
			}
		}
	}

	return maze;
}
