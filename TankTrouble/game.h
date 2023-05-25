#include <SDL.h>
#include <iostream>
#include <init.h>
#include <structs.h>
#include <box2d.h>
#include <array>
#include <set>

struct TankTrouble {
	const int tableSize = 256;
	float* cosTable;
	SDL_Texture* gameTexture;
	SDL_Rect bulletSrc;
	SDL_Rect bulletDst;
	SDL_Rect light_grayTileSrc;
	SDL_Rect light_grayTileDst;
	SDL_Rect grayTileSrc;
	SDL_Rect grayTileDst;
	b2World* world;
	float timeStep;      //the length of time passed to simulate (seconds)
	int32 velocityIterations;   //how strongly to correct velocity
	int32 positionIterations;   //how strongly to correct position
	std::vector<mazeFix*> walls;
	std::vector<Bullet*> bullets;
	std::vector<tank*> tanks;
	b2Vec2* pos;
	float angles[2] = { 0, 0 };
	std::set<Bullet*> bulletScheduledForRemoval;
	b2BodyDef mazeDef;
	b2Body* mazeBody;
	END theEnd;
	const int mapSize = 5;
	std::vector<mazeWall> mazeState;
	mazeSetup* map;
	float bulletMaxVelocity;
	std::vector<int> tiles;
	Api* api;
	bool SDL_render;
	float renderDelay_ms;

	//need to make internal time for timers
	float internal_time;

	TankTrouble(App* app, bool render, float userTimestep = 1 / 24.0, float SDLDelay_ms = 10.0);

	void cosTableInit();

	float cosLookup(float x);

	float sinLookup(float x);

	bool step(App* app);

	std::array<float, 110> retrieveData();

	~TankTrouble();
};

