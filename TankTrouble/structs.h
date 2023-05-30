#pragma once
#include <box2d.h>
#include <SDL.h>
#include <vector>
#include <iostream>


struct Timer {
	
	int initial_ts;

	Timer();

	/// Reset the timer.
	void Reset(int currentTimestep);

	/// Get the time since construction or the last reset.
	double GetSeconds(int currentTimestep, double timestep);
	int GetTimeSteps(int currentTimestep);

};

struct Api {
	int size;
	int rowSize;
	bool* horizontalWalls;
	bool* verticalWalls;

	float* bulletsPosXY;
	float* bulletsVelXY;
	float* tanksPosXY;
	float* tanksVelXY;

	Api(int mazeSize, int playerCount);

	void prepareWalls(bool* topWalls, bool* leftWalls, bool* rightWalls, bool* bottomWalls);

	~Api();
};


struct App {
	SDL_Renderer* renderer;
	SDL_Window* window;
	int up[2] = { 0, 0 };
	int down[2] = { 0, 0 };
	int left[2] = { 0, 0 };
	int right[2] = { 0, 0 };
	int space[2] = { 0, 0 };
	int reset;
	bool userInputs;

	App(bool userInp);
	void doInput();
	void doKeyUp(SDL_KeyboardEvent* event);
	void doKeyDown(SDL_KeyboardEvent* event);
};

typedef struct {
	SDL_Point center;
	SDL_Rect posRect;
	SDL_Texture* texture;
	float angle;
} RotatingEntity;

typedef struct {
	SDL_Texture* texture;
	int x, y;
} Entity;


/*
struct player {

	SDL_Point center[2];
	SDL_Rect posRect[2];
	SDL_Texture* texture[2];
	float angle;

	player(int init_x, int init_y, int init_angle);

	void newState(int x, int y, int rad);

	void draw();
};
*/

/*
struct box {
	SDL_Rect src;
	SDL_Texture* texture;
	SDL_Rect dst;
	float angle;

	box(int x, int y, int h, int w, float rad, Uint8 red, Uint8 green, Uint8 blue);

};
*/

class tank {
public:
	//body
	App* app;
	float width;
	float height;
	RotatingEntity body;
	SDL_Point headCenter[3];
	SDL_Rect headPosRect[3];
	SDL_Texture* headTextures[3];
	int head_state;
	int regularAmmo;
	b2Vec2 Vel;
	b2Body* m_body;
	b2World* m_world;
	SDL_Texture* bodycolor;

	tank(App* app, b2World* world, float x, float y, float rad, bool render);
	//void draw();
	~tank();

	b2Transform GetTransfrom();
};

class Bullet {
public:
	b2BodyDef bodyDef;
	b2Body* body;
	b2CircleShape bodyShape;
	b2FixtureDef bodyFix;
	b2World* m_world;
	Timer lifespan;
	float m_radius = 0.45;
	int id;

	Bullet(b2World* world, int player_id, float x, float y, int currentTimestep, b2Vec2 velocity = b2Vec2_zero);
	double GetLifespan(int internalTime, double timestep);
	b2Transform GetTransform();

	~Bullet();
};

/*
class maze {
public:
	b2Body* body;
	b2BodyDef bodyDef;
	b2PolygonShape bodyShape;
	b2World* m_world;
	b2FixtureDef bodyFix;
	SDL_Rect textureSrc;
	SDL_Rect textureDst;
	float width;
	float height;
	int type;
	maze(b2World* world, float x, float y, float w, float h);

	b2Transform GetTransform();
	void draw(SDL_Texture* gameTexture);
	~maze();
};*/


struct mazeWall {
	int wallBreaks[4] = { 0, 0, 0, 0 };
	bool path = false;

	mazeWall(std::vector<int> breaks);
};

class mazeFix {
public:
	App* app;
	b2PolygonShape bodyShape;
	b2FixtureDef bodyFix;
	SDL_Rect textureSrc;
	SDL_Rect textureDst;
	float width;
	float height;
	int type;
	b2Vec2 pos;

	mazeFix(App* app, b2World* world, float x, float y, float w, float h);

	void draw(SDL_Texture* texture);
};

class mazeSetup {
public:
	int l = 10;
	int w = 1;
	App* app;
	bool* topWalls;
	bool* leftWalls;
	bool* rightWalls;
	bool* bottomWalls;
	int fullsize;
	int rowSize;
	std::vector<int> blocks;

	mazeSetup(App* app, int size);

	//void drawTop(int x, int y);

	//void drawBottom(int x, int y);

	//void drawLeft(int x, int y);

	//void drawRight(int x, int y);

	void buildMaze(std::vector<mazeWall> maze);

	std::vector<mazeFix*> build(b2World* world,std::vector<mazeWall> maze, int x, int y);

	//void drawMaze(int x, int y);

	~mazeSetup();
};

struct END {

	Timer timer;
	bool is;
};


