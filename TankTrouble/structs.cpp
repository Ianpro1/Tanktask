#include <structs.h>
//#include <SDL2_gfxPrimitives.h>
#include <draw.h>
#include <defs.h>


Timer::Timer() {

	initial_ts = 0;
}

void Timer::Reset(int currentTimestep) {

	initial_ts = currentTimestep;
}

double Timer::GetMilliseconds(int currentTimestep, double timeStep) {

	int diff = currentTimestep - initial_ts;
	return timeStep * (double)diff;
}

Api::Api(int mazeSize, int playerCount) {
	
	size = mazeSize * mazeSize + mazeSize;
	rowSize = mazeSize;
	horizontalWalls = new bool[size];
	verticalWalls = new bool[size];

	int bcoordinateCount = playerCount * 10;
	bulletsPosXY = new float[bcoordinateCount];
	bulletsVelXY = new float[bcoordinateCount];
	
	int pcoordinateCount = playerCount * 2;
	tanksPosXY = new float[pcoordinateCount];
	tanksVelXY = new float[pcoordinateCount];
}

void Api::prepareWalls(bool* topWalls, bool* leftWalls, bool* rightWalls, bool* bottomWalls) {
	for (int i = 0; i < size; i++) {
		horizontalWalls[i] = 0;
		verticalWalls[i] = 0;
	}

	//using bottomWalls and rightwalls as reference
	for (int i = 0; i < size; i++) {
		if (i < rowSize) {
			horizontalWalls[i] += topWalls[i];
			verticalWalls[i] += leftWalls[i];
		}
		else if (i>= size - rowSize){
			horizontalWalls[i] += bottomWalls[i-rowSize];
			verticalWalls[i] += rightWalls[i-rowSize];

		}
		else {
			horizontalWalls[i] += bottomWalls[i - rowSize];
			horizontalWalls[i] += topWalls[i];
			verticalWalls[i] += rightWalls[i - rowSize];
			verticalWalls[i] += leftWalls[i];
		}
	}
}

Api::~Api() {
	delete horizontalWalls;
	delete verticalWalls;
	delete bulletsPosXY;
	delete bulletsVelXY;
	delete tanksPosXY;
	delete tanksVelXY;
}

//tank
tank::tank(App* appl, b2World* world, float x, float y, float rad, bool render) {
	app = appl;
	head_state = 0;
	regularAmmo = 5;
	m_world = world;
	width = 3.2;
	height = 4;
	//tanks m_body
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody; //this will be a dynamic body
	bodyDef.position.Set(x, y); //set the starting position
	bodyDef.angle = rad; //set the starting angle

	b2PolygonShape bodyShape;
	bodyShape.SetAsBox(width / 2, height / 2);

	b2FixtureDef fixDef;
	fixDef.density = 1;
	fixDef.shape = &bodyShape;
	m_body = m_world->CreateBody(&bodyDef);
	m_body->CreateFixture(&fixDef);

	b2PolygonShape bodyShape2;
	bodyShape2.SetAsBox(0.5, 0.35, b2Vec2(0, -height/2 - 0.35), 0);

	b2FixtureDef fixDef2;
	fixDef2.density = 1;
	fixDef2.shape = &bodyShape2;
	m_body->CreateFixture(&fixDef2);

	//body
	Vel = b2Vec2_zero;

	if (render) {
		//SDL components
		body.texture = loadTexture(app, "assets/tanktrouble_raw/tankbody.png");
		SDL_Point bodyCenter;
		bodyCenter.x = 16;
		bodyCenter.y = 20;
		body.center = bodyCenter;
		SDL_QueryTexture(body.texture, NULL, NULL, &body.posRect.w, &body.posRect.h);

		//head default
		headTextures[0] = loadTexture(app, "assets/tanktrouble_raw/tankhead1.png");
		headTextures[1] = loadTexture(app, "assets/tanktrouble_raw/tankhead2.png");
		headTextures[2] = loadTexture(app, "assets/tanktrouble_raw/tankhead3.png");
		headCenter[0].x = 12;
		headCenter[0].y = 27;
		headCenter[1].x = 12;
		headCenter[1].y = 25;
		headCenter[2].x = 12;
		headCenter[2].y = 20;
		SDL_QueryTexture(headTextures[0], NULL, NULL, &headPosRect[0].w, &headPosRect[0].h);
		SDL_QueryTexture(headTextures[1], NULL, NULL, &headPosRect[1].w, &headPosRect[1].h);
		SDL_QueryTexture(headTextures[2], NULL, NULL, &headPosRect[2].w, &headPosRect[2].h);

		bodycolor = loadTexture(app, "assets/tanktrouble_raw/tankcolor.png");
	}
}

b2Transform tank::GetTransfrom() {
	return m_body->GetTransform();
}

/*
void tank::draw() {

	b2Vec2 pos = m_body->GetPosition();
	rectangleRGBA(app->renderer, (pos.x - width / 2) * MTOPIXEL, (pos.y - height / 2) * MTOPIXEL, (pos.x + width / 2) * MTOPIXEL, (pos.y + height / 2) * MTOPIXEL, 255, 0, 0, 1000);
	//rectangleRGBA(app.renderer, (pos.x - width / 2) * MTOPIXEL, (pos.y - height / 2) * MTOPIXEL, (pos.x + width / 2) * MTOPIXEL, (pos.y + height / 2) * MTOPIXEL, 255, 0, 0, 1000);
}
*/
tank::~tank() {
	m_world->DestroyBody(m_body);
}


/*
//box
box::box(int x, int y, int h, int w, float rad, Uint8 red, Uint8 green, Uint8 blue) {
	texture = loadTexture(app, "assets/tanktrouble_raw/game.png");
	SDL_SetTextureColorMod(texture,red, green, blue);
	src.x = 149;
	src.y = 99;
	src.h = 20;
	src.w = 20;

	dst.w = w;
	dst.h = h;
	dst.x = x;
	dst.y = y;

	angle = rad;
}
*/
/*
//player
player::player(int init_x, int init_y, int init_angle) {
	//body
	center[0].x = 16;
	center[0].y = 20;
	texture[0] = loadTexture("assets/tanktrouble_raw/tankbody.png");
	SDL_QueryTexture(texture[0], NULL, NULL, &posRect[0].w, &posRect[0].h);
	//head
	center[1].x = 12;
	center[1].y = 27;
	texture[1] = loadTexture("assets/tanktrouble_raw/tankhead1.png");
	SDL_QueryTexture(texture[1], NULL, NULL, &posRect[1].w, &posRect[1].h);

	angle = init_angle;

	posRect[0].x = init_x;
	posRect[0].y = init_y;

	posRect[1].x = init_x + 4;
	posRect[1].y = init_y - 7;
}

void player::newState(int x, int y, int rad) {

	posRect[0].x = x;
	posRect[0].y = y;

	posRect[1].x = x + 4;
	posRect[1].y = y - 7;

	angle = rad;
}

void player::draw() {
	blit(texture[0], posRect[0], angle, center[0]);
	blit(texture[1], posRect[1], angle, center[1]);
}
*/

Bullet::Bullet(b2World* world, int player_id, float x, float y, int currentTimestep, b2Vec2 velocity) {

	id = player_id;
	//Bullet
	lifespan.Reset(currentTimestep);
	m_world = world;
	//circles are drawn with offset of -2(r, r), the center of the physical physical object is then at the circle's true center (shown on screen)
	bodyDef.type = b2_dynamicBody;
	bodyDef.bullet = true;
	bodyDef.angle = 0;
	bodyDef.position.Set(x, y);

	body = world->CreateBody(&bodyDef);

	bodyShape.m_radius = m_radius;
	//bodyShape.m_p = -2 * b2Vec2(m_radius, m_radius);

	bodyFix.shape = &bodyShape;
	bodyFix.friction = 0;
	bodyFix.density = 1;
	bodyFix.filter.groupIndex = -2;
	bodyFix.restitution = 1;
	body->CreateFixture(&bodyFix);
	body->SetLinearVelocity(velocity);
}

b2Transform Bullet::GetTransform() {
	return body->GetTransform();
}


Bullet::~Bullet() {
	m_world->DestroyBody(body);
}

//bullet
double Bullet::GetLifespan(int internalTime, double timestep) {
	return lifespan.GetMilliseconds(internalTime,timestep);
}

/*
//maze
maze::maze(b2World* world, float x, float y, float w, float h) {
	
	bodyDef.type = b2_staticBody;
	width = w;
	height = h;
	m_world = world;
	bodyShape.SetAsBox(w/2, h/2);
	bodyDef.position.Set(x, y);
	body = m_world->CreateBody(&bodyDef);
	bodyFix.shape = &bodyShape;
	bodyFix.filter.groupIndex = -1;
	body->CreateFixture(&bodyFix);
	textureSrc.x = 533;
	textureSrc.y = 75;
	textureSrc.w = 2;
	textureSrc.h = 2;
	textureDst.w = width * MTOPIXEL;
	textureDst.h = height * MTOPIXEL;
}

maze::~maze() {
	m_world->DestroyBody(body);
}

b2Transform maze::GetTransform() {
	return body->GetTransform();
}

void maze::draw(SDL_Texture* gameTexture) {
	b2Vec2 pos = body->GetPosition();
	textureDst.x = (pos.x-width/2) * MTOPIXEL;
	textureDst.y = (pos.y-height/2) * MTOPIXEL;
	SDL_RenderCopy(app.renderer, gameTexture, &textureSrc, &textureDst);
	//rectangleRGBA(app.renderer, (pos.x - width/2) * MTOPIXEL, (pos.y - height/2) * MTOPIXEL, (pos.x + width/2) * MTOPIXEL, (pos.y + height/2) * MTOPIXEL, 255, 0, 0, 1000);
}
*/

mazeWall::mazeWall(std::vector<int> breaks) {

	for (int type : breaks) {
		if (type < 0) {
			//unbreakable
			if (type == -4) {
				wallBreaks[0] = -1;
			}
			else {
				wallBreaks[abs(type)] = -1;
			}
		}
		else {
			//break
			wallBreaks[type] = 1;
		}
	}
	//types 0=>right, 1=>down, 2=>left, 3=>top, -1=>unbreakable
	//special ids -1==>unbreakable down, -3=> unbreakable top, -4 => unbreakable right
}

//mazeSetup

mazeSetup::mazeSetup(App* appl, int size) {
	app = appl;
	rowSize = size;
	fullsize = size * size;
	topWalls = new bool[fullsize];
	rightWalls = new bool[fullsize];
	bottomWalls = new bool[fullsize];
	leftWalls = new bool[fullsize];
}
/*
void mazeSetup::drawTop(int x, int y) {

	rectangleRGBA(app->renderer, x + l / 2 + w, y - l / 2 - w, x - l / 2 - w, y - l / 2, 255, 0, 0, 1000);
}

void mazeSetup::drawBottom(int x, int y) {

	rectangleRGBA(app->renderer, x + l / 2 + w, y + l / 2 + w, x - l / 2 - w, y + l / 2, 255, 0, 0, 1000);
}

void mazeSetup::drawLeft(int x, int y) {

	rectangleRGBA(app->renderer, x - l / 2 - w, y + l / 2 + w, x - l / 2, y - l / 2 - w, 255, 0, 0, 1000);
}

void mazeSetup::drawRight(int x, int y) {

	rectangleRGBA(app->renderer, x + l / 2 + w, y + l / 2 + w, x + l / 2, y - l / 2 - w, 255, 0, 0, 1000);
}
*/
void mazeSetup::buildMaze(std::vector<mazeWall> maze) {

	for (int i = 0; i < fullsize; i++) {
		topWalls[i] = 1;
		bottomWalls[i] = 1;
		leftWalls[i] = 1;
		rightWalls[i] = 1;
	}
	
	for (int i = 0; i < maze.size(); i++) {
		if (maze[i].wallBreaks[0] == 1) {
			rightWalls[i] = 0;
			leftWalls[i + 1] = 0;
		}
		if (maze[i].wallBreaks[1] == 1) {
			bottomWalls[i] = 0;
			topWalls[i + rowSize] = 0;
		}
		if (maze[i].wallBreaks[2] == 1) {
			leftWalls[i] = 0;
			rightWalls[i - 1] = 0;
		}
		if (maze[i].wallBreaks[3] == 1) {
			topWalls[i] = 0;
			bottomWalls[i - rowSize] = 0;
		}
	}

	//correct maze
	
	//remove bottoms from top
	for (int i = rowSize; i < fullsize; i++) {
		if (topWalls[i] == 1) {
			bottomWalls[i - rowSize] = 0;
		}
	}
	//remove right from left
	for (int i = 1; i < fullsize; i++) {
		if (fmod(i, rowSize) == 0) {
			continue;
		}
		if (leftWalls[i] == 1) {
			rightWalls[i - 1] = 0;
		}
	}
}

std::vector<mazeFix*> mazeSetup::build(b2World* world, std::vector<mazeWall> m_maze, int x, int y) {

	for (int i = 0; i < fullsize; i++) {
		topWalls[i] = 1;
		bottomWalls[i] = 1;
		leftWalls[i] = 1;
		rightWalls[i] = 1;
	}

	for (int i = 0; i < m_maze.size(); i++) {
		if (m_maze[i].wallBreaks[0] == 1) {
			rightWalls[i] = 0;
			leftWalls[i + 1] = 0;
		}
		if (m_maze[i].wallBreaks[1] == 1) {
			bottomWalls[i] = 0;
			topWalls[i + rowSize] = 0;
		}
		if (m_maze[i].wallBreaks[2] == 1) {
			leftWalls[i] = 0;
			rightWalls[i - 1] = 0;
		}
		if (m_maze[i].wallBreaks[3] == 1) {
			topWalls[i] = 0;
			bottomWalls[i - rowSize] = 0;
		}
	}

	//before correction, register blocks for tiling
	blocks.clear();
	for (int i = 0; i < fullsize; i++) {
		if (bottomWalls[i] == 1 && topWalls[i] == 1 && leftWalls[i] == 1 && rightWalls[i] == 1) {
			blocks.push_back(i);
		}
	}

	//correct maze

	//remove bottoms from top
	for (int i = rowSize; i < fullsize; i++) {
		if (topWalls[i] == 1) {
			bottomWalls[i - rowSize] = 0;
		}
	}
	//remove right from left
	for (int i = 1; i < fullsize; i++) {
		if (fmod(i, rowSize) == 0) {
			continue;
		}
		if (leftWalls[i] == 1) {
			rightWalls[i - 1] = 0;
		}
	}
	
	//remove wall between blocks
	for (int i = 0; i < blocks.size(); i++) {
		for (int other = 0; other < blocks.size(); other++) {

			if (other == i)
				continue;
			int cur_id = blocks[i];

			if (fmod(cur_id+1, rowSize) != 0 && blocks[other] == cur_id+1) {
				rightWalls[cur_id] = 0;
				leftWalls[cur_id + 1] = 0;
			}

			if (fmod(cur_id, rowSize) != 0 && blocks[other] == cur_id - 1) {
				rightWalls[cur_id-1] = 0;
				leftWalls[cur_id] = 0;
			}

			if (cur_id >= rowSize && blocks[other] == cur_id - rowSize) {
				topWalls[cur_id] = 0;
				bottomWalls[cur_id -rowSize] = 0;
			}

			if (cur_id < fullsize - rowSize && blocks[other] == cur_id + rowSize) {
				topWalls[cur_id + rowSize] = 0;
				bottomWalls[cur_id] = 0;
			}
		}
	}
	
	//remove wall between a block and the outside
	for (int id : blocks) {
		if (id < rowSize) {
			topWalls[id] = 0;
		}
		if (id >= fullsize - rowSize) {
			bottomWalls[id] = 0;
		}
		if (fmod(id, rowSize) == 0) {
			leftWalls[id] = 0;
		}
		if (fmod(id+1, rowSize) == 0) {
			rightWalls[id] = 0;
		}
	}

	std::vector<mazeFix*> walls;
	float distance = l / 2 ;

	for (int i = 0; i < rowSize; i++) {
		for (int k = 0; k < rowSize; k++) {

			int cur_x = x + k * (l);
			int cur_y = y + i * (l);

			if (topWalls[rowSize * i + k] == 1) {
				walls.push_back(new mazeFix(app, world, cur_x, cur_y-distance, l+w, w));
			}
			if (bottomWalls[rowSize * i + k] == 1) {
				walls.push_back(new mazeFix(app, world, cur_x, cur_y + distance, l+w, w));
			}
			if (rightWalls[i * rowSize + k] == 1) {
				walls.push_back(new mazeFix(app, world, cur_x + distance, cur_y, w, l+w));
			}
			if (leftWalls[i * rowSize + k] == 1) {
				walls.push_back(new mazeFix(app, world, cur_x -distance, cur_y, w, l+w));
			}
		}
	}
	return walls;
}

/*
void mazeSetup::drawMaze(int x, int y) {

	for (int i = 0; i < rowSize; i++) {
		for (int k = 0; k < rowSize; k++) {

			int cur_x = x + k * (l + w) * MTOPIXEL;
			int cur_y = y + i * (l + w) * MTOPIXEL;

			if (topWalls[rowSize * i + k] == 1) {
				drawTop(cur_x, cur_y);
			}
			if (bottomWalls[rowSize * i + k] == 1) {
				drawBottom(cur_x, cur_y);
			}
			if (rightWalls[i * rowSize + k] == 1) {
				drawRight(cur_x, cur_y);
			}
			if (leftWalls[i * rowSize + k] == 1) {
				drawLeft(cur_x, cur_y);
			}

		}
	}
}
*/
mazeSetup::~mazeSetup() {
	delete topWalls;
	delete bottomWalls;
	delete leftWalls;
	delete rightWalls;
}


mazeFix::mazeFix(App* appl, b2World* world, float x, float y, float w, float h) {
	app = appl;
	pos = b2Vec2(x, y);
	width = w;
	height = h;
	bodyShape.SetAsBox(w / 2, h / 2, b2Vec2(x, y), 0);
	bodyFix.shape = &bodyShape;
	bodyFix.filter.groupIndex = -1;
	textureSrc.x = 533;
	textureSrc.y = 75;
	textureSrc.w = 2;
	textureSrc.h = 2;
	textureDst.w = width * MTOPIXEL;
	textureDst.h = height * MTOPIXEL;
}

void mazeFix::draw(SDL_Texture* gameTexture) {
	textureDst.x = (pos.x - width / 2 ) * MTOPIXEL;
	textureDst.y = (pos.y - height / 2) * MTOPIXEL;
	SDL_RenderCopy(app->renderer, gameTexture, &textureSrc, &textureDst);
	//rectangleRGBA(app.renderer, (pos.x - width / 2) * MTOPIXEL, (pos.y - height / 2) * MTOPIXEL, (pos.x + width / 2) * MTOPIXEL, (pos.y + height / 2) * MTOPIXEL, 255, 0, 0, 1000);
	//rectangleRGBA(app.renderer, (pos.x - width / 2) * MTOPIXEL, (pos.y - height / 2) * MTOPIXEL, (pos.x + width / 2) * MTOPIXEL, (pos.y + height / 2) * MTOPIXEL, 255, 0, 0, 1000);
}


App::App(bool userInp) {
	userInputs = userInp;
	reset = 1;
}