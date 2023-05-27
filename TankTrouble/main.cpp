#include <iostream>
#include <array>
#include <game.h>
#include <input.h>
#include <defs.h>
using namespace std;

/*
const int tableSize = 256;
float cosTable[tableSize];

void cosTableInit() {

	for (int i = 0; i < tableSize; i++) {
		cosTable[i] = cos(M_PI * i / tableSize);
	}
}

float cosLookup(float x) {

	float side = fmod(abs(x), M_PI * 2);
	float remain = fmod(abs(x), M_PI);

	int i = floor(remain / M_PI * tableSize);

	if (side < M_PI) {
		return cosTable[i];
	}
	else {
		return -cosTable[i];
	}
}

float sinLookup(float x) {
	return cosLookup(x - M_PI / 2);
}

*/
/*
struct TankTrouble {
	const int tableSize = 256;
	float *cosTable;
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

	TankTrouble(App* app, bool render) {

		SDL_render = render;

		
		cosTable = new float[tableSize];
		cosTableInit();
		
		if (render) {
			initSDL();
			gameTexture = loadTexture("assets/tanktrouble_raw/game.png");
			bulletSrc.x = 639;
			bulletSrc.y = 92;
			bulletSrc.w = 9;
			bulletSrc.h = 9;
			bulletDst.w = 9;
			bulletDst.h = 9;
			light_grayTileSrc.x = 688;
			light_grayTileSrc.y = 181;
			light_grayTileSrc.w = 2;
			light_grayTileSrc.h = 2;
			light_grayTileDst.w = 100;
			light_grayTileDst.h = 100;
			grayTileSrc.x = 300;
			grayTileSrc.y = 146;
			grayTileSrc.w = 2;
			grayTileSrc.h = 2;
			grayTileDst.w = 100;
			grayTileDst.h = 100;
		}	

		world = new b2World(b2Vec2(0.0f, 0.0f));
		timeStep = 1 / 24.0;      //the length of time passed to simulate (seconds)
		velocityIterations = 8;   //how strongly to correct velocity
		positionIterations = 8;   //how strongly to correct position
		world->Step(timeStep, velocityIterations, positionIterations);
		tanks.push_back(new tank(world, 0, 0, 0, render));
		tanks.push_back(new tank(world, 0, 0, 0, render));
		pos = new b2Vec2[2];

		if (SDL_render) {
			SDL_SetTextureColorMod(tanks[0]->bodycolor, 245, 0, 0);
			SDL_SetTextureColorMod(tanks[1]->bodycolor, 0, 245, 0);
		}


		mazeDef.type = b2_staticBody;
		mazeDef.position.Set(0, 0);
		mazeBody = world->CreateBody(&mazeDef);
		srand(time(0));
		map = new mazeSetup(mapSize);
		bulletMaxVelocity = 4.6;
		app->reset = 1;
		api = new Api(mapSize, 2);
	}

	void cosTableInit() {

		for (int i = 0; i < tableSize; i++) {
			cosTable[i] = cos(M_PI * i / tableSize);
		}
	}

	float cosLookup(float x) {

		float side = fmod(abs(x), M_PI * 2);
		float remain = fmod(abs(x), M_PI);

		int i = floor(remain / M_PI * tableSize);

		if (side < M_PI) {
			return cosTable[i];
		}
		else {
			return -cosTable[i];
		}
	}

	float sinLookup(float x) {
		return cosLookup(x - M_PI / 2);
	}

	bool step(App* app) {

		if (app->reset) {
			world->DestroyBody(mazeBody);
			mazeBody = world->CreateBody(&mazeDef);
			theEnd.is = false;
			app->reset = 0;
			//init maze
			while (1) {
				mazeState = InitMaze(mapSize);
				if (mazeState.empty() != 1) {
					for (mazeFix* wall : walls) {
						delete wall;
					}
					walls.clear();
					tiles.clear();
					walls = map->build(world, mazeState, 8, 15);
					for (int i = 0; i < walls.size(); i++) {
						mazeBody->CreateFixture(&walls[i]->bodyFix);
					}
					break;
				}
			}
			//create the array input to describe mazeSetup
			api->prepareWalls(map->topWalls, map->leftWalls, map->rightWalls, map->bottomWalls);

			// init tiles
			for (int i = 0; i < map->fullsize; i++) {
				if (std::find(map->blocks.begin(), map->blocks.end(), i) != map->blocks.end()) {
					tiles.push_back(0);
					continue;
				}
				double prob = getRand();
				if (prob > 0.75) {
					tiles.push_back(2);
				}
				else {
					tiles.push_back(1);
				}
			}

			//set player spawn
			for (int p = 0; p < tanks.size(); p++) {

				app->regularAmmo[p] = 5;
				app->regularCooldown[p].Reset();

				while (1) {
					double prob = getRand();
					double prob2 = getRand();
					int id = floor(prob * map->fullsize);

					if (std::find(map->blocks.begin(), map->blocks.end(), id) != map->blocks.end()) {
						continue;
					}

					int k = fmod(id, map->rowSize);
					int i = (id - k) / map->rowSize;
					int x = 8 + k * (map->l);
					int y = 15 + i * (map->l);

					float startAngle = prob2 * 2 * M_PI;
					tanks[p]->m_body->SetTransform(b2Vec2(x, y), startAngle);
					map->blocks.push_back(id);
					break;
				}
			}
		}

		world->Step(timeStep, velocityIterations, positionIterations);

		if (SDL_render) {
			prepareScene();
			//draw tiles
			for (int i = 0; i < map->rowSize; i++) {
				for (int k = 0; k < map->rowSize; k++) {

					int cur_x = (8 + k * (map->l) - map->l / 2) * MTOPIXEL;
					int cur_y = (15 + i * (map->l) - map->l / 2) * MTOPIXEL;
					int tile_id = i * map->rowSize + k;

					if (tiles[tile_id] == 1) {
						light_grayTileDst.x = cur_x;
						light_grayTileDst.y = cur_y;
						SDL_RenderCopy(app->renderer, gameTexture, &light_grayTileSrc, &light_grayTileDst);
					}
					else if (tiles[tile_id] == 2) {
						grayTileDst.x = cur_x;
						grayTileDst.y = cur_y;
						SDL_RenderCopy(app->renderer, gameTexture, &grayTileSrc, &grayTileDst);
					}
				}
			}

			//draw walls
			for (mazeFix* it : walls) {
				it->draw(gameTexture);
			}

			doInput();
		}

		for (int p = 0; p < tanks.size(); p++) {

			if (app->left[p] && app->right[p]) {
				tanks[p]->m_body->SetAngularVelocity(0);
			}
			else if (app->left[p]) {
				tanks[p]->m_body->SetAngularVelocity(-75 * DEGTORAD);
			}
			else if (app->right[p]) {
				tanks[p]->m_body->SetAngularVelocity(75 * DEGTORAD);
			}
			else {
				tanks[p]->m_body->SetAngularVelocity(0);
			}

			pos[p] = tanks[p]->m_body->GetPosition();
			angles[p] = tanks[p]->m_body->GetAngle();

			if (app->up[p] && app->down[p]) {
				tanks[p]->Vel.y = -1.1 * cosLookup(angles[p]);
				tanks[p]->Vel.x = 1.1 * sinLookup(angles[p]);
			}
			else if (app->up[p]) {
				tanks[p]->Vel.y = -4.2 * cosLookup(angles[p]);
				tanks[p]->Vel.x = 4.2 * sinLookup(angles[p]);
			}
			else if (app->down[p]) {
				tanks[p]->Vel.y = 3.2 * cosLookup(angles[p]);
				tanks[p]->Vel.x = -3.2 * sinLookup(angles[p]);
			}
			else {
				tanks[p]->Vel.x = 0;
				tanks[p]->Vel.y = 0;
			}

			tanks[p]->m_body->SetLinearVelocity(tanks[p]->Vel);

			//shoot
			//bullets can clip through wall if tanks are too close
			if (app->space[p]) {
				//best solution is to raytrace and look for instakills before shooting
				b2Vec2 pt = b2Vec2(pos[p].x + 3.5 * sin(angles[p]), pos[p].y - 3.5 * cos(angles[p]));
				bool insta = false;
				int insta_others = -1;

				for (b2Fixture* fixture = mazeBody->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
					if (fixture->TestPoint(pt)) {
						insta = true;
					}
				}

				//insta others
				for (int others = 0; others < tanks.size(); others++) {
					if (others != p) {
						for (b2Fixture* fixture = tanks[others]->m_body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
							if (fixture->TestPoint(pt)) {
								insta_others = others;
							}
						}
					}
				}

				if (insta_others != -1) {
					theEnd.is = true;
					theEnd.timer.Reset();
					tanks[insta_others]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);
					app->space[p] = 0;
					app->regularAmmo[p] += 1;

					if (SDL_render) {
						bulletDst.x = pt.x * MTOPIXEL - 4.5;
						bulletDst.y = pt.y * MTOPIXEL - 4.5;
						SDL_RenderCopy(app->renderer, gameTexture, &bulletSrc, &bulletDst);
					}
				}
				else if (insta) {
					theEnd.is = true;
					theEnd.timer.Reset();
					tanks[p]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);
					app->space[p] = 0;

					if (SDL_render) {
						bulletDst.x = pt.x * MTOPIXEL - 4.5;
						bulletDst.y = pt.y * MTOPIXEL - 4.5;
						SDL_RenderCopy(app->renderer, gameTexture, &bulletSrc, &bulletDst);
					}
				}
				else {
					app->space[p] = 0;
					b2Vec2 bulletVel = b2Vec2(bulletMaxVelocity * sin(angles[p]), -bulletMaxVelocity * cos(angles[p]));
					bullets.push_back(new Bullet(world, p, pt.x, pt.y, bulletVel));
				}

				tanks[p]->head_state = 9;
			}

			//draw tanks and handle deaths
			//draw tanks
			if (SDL_render) {

				tanks[p]->body.posRect.x = (pos[p].x - tanks[p]->width * 0.5) * MTOPIXEL;
				tanks[p]->body.posRect.y = (pos[p].y - tanks[p]->height * 0.5) * MTOPIXEL;
				tanks[p]->body.angle = angles[p];
				blit(tanks[p]->bodycolor, tanks[p]->body.posRect, tanks[p]->body.angle, tanks[p]->body.center);
				blit(tanks[p]->body.texture, tanks[p]->body.posRect, tanks[p]->body.angle, tanks[p]->body.center);

				int thisHead = tanks[p]->head_state;
				float offy;

				if (thisHead > 0) {
					if (thisHead < 5 || thisHead > 8) {
						thisHead = 1;
						offy = 5;
					}
					else {
						thisHead = 2;
						offy = 0;
					}
					tanks[p]->head_state -= 1;
				}
				else {
					offy = 7;
				}

				tanks[p]->headPosRect[thisHead].x = (pos[p].x - tanks[p]->width * 0.5) * MTOPIXEL + 4;
				tanks[p]->headPosRect[thisHead].y = (pos[p].y - tanks[p]->height * 0.5) * MTOPIXEL - offy;
				blit(tanks[p]->headTextures[thisHead], tanks[p]->headPosRect[thisHead], tanks[p]->body.angle, tanks[p]->headCenter[thisHead]);
			}

			//handle deaths
			//note: one bullet can kill 2 players which is not intended (but not worrying)
			for (b2ContactEdge* ce = tanks[p]->m_body->GetContactList(); ce; ce = ce->next) {
				if (ce->contact->IsTouching()) {
					for (Bullet* b : bullets) {
						if (ce->other == b->body) {
							theEnd.is = true;
							theEnd.timer.Reset();
							tanks[p]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);
							bulletScheduledForRemoval.insert(b);
							break;
						}
					}
				}
			}
		}

		//draw bullets and queue death
		for (Bullet* it : bullets) {
			float time = it->GetLifespan();
			if (time > 10000) {
				bulletScheduledForRemoval.insert(it);
			}

			if (SDL_render) {

				b2Transform bulletTransform = it->GetTransform();
				//filledCircleRGBA(app->renderer, bulletTransform.p.x * MTOPIXEL, bulletTransform.p.y * MTOPIXEL, it->m_radius * MTOPIXEL, 255, 0, 0, 100);

				bulletDst.x = bulletTransform.p.x * MTOPIXEL - 4.5;
				bulletDst.y = bulletTransform.p.y * MTOPIXEL - 4.5;
				SDL_RenderCopy(app->renderer, gameTexture, &bulletSrc, &bulletDst);
			}
		}

		if (SDL_render) {
			//circleRGBA(app->renderer, pt.x * MTOPIXEL, pt.y * MTOPIXEL, 1, 0, 0, 255, 1000);
			presentScene();
			SDL_Delay(10);
		}

		//handle end
		if (theEnd.is && theEnd.timer.GetMilliseconds() > 3000) {
			app->reset = 1;
			//remove bullets from map
			for (Bullet* it : bullets) {
				bulletScheduledForRemoval.insert(it);
			}
		}

		//process removal schedule
		std::set<Bullet*>::iterator it = bulletScheduledForRemoval.begin();
		std::set<Bullet*>::iterator end = bulletScheduledForRemoval.end();
		for (; it != end; ++it) {
			Bullet* dyingBall = *it;

			//delete ball... physics body is destroyed here
			delete dyingBall;

			//remove it from list of bullets
			std::vector<Bullet*>::iterator it = std::find(bullets.begin(), bullets.end(), dyingBall);
			if (it != bullets.end())
				bullets.erase(it);
		}
		//clear this list for next time
		bulletScheduledForRemoval.clear();


		//retrieve game information for api
		for (int i = 0; i < 20; i++) {
			api->bulletsPosXY[i] = 0;
			api->bulletsVelXY[i] = 0;
		}
		for (int i = 0; i < bullets.size(); i++) {

			b2Vec2 pb = bullets[i]->body->GetPosition();
			b2Vec2 vb = bullets[i]->body->GetLinearVelocity();

			api->bulletsPosXY[i] = pb.x;
			api->bulletsPosXY[i + (int64_t)10] = pb.y;
			api->bulletsVelXY[i] = vb.x;
			api->bulletsVelXY[i + (int64_t)10] = vb.y;

		}
		for (int i = 0; i < 2; i++) {
			b2Vec2 pt = tanks[i]->m_body->GetPosition();
			b2Vec2 vt = tanks[i]->m_body->GetLinearVelocity();

			api->tanksPosXY[i] = pt.x;
			api->tanksPosXY[i + 2] = pt.y;
			api->tanksVelXY[i] = vt.x;
			api->tanksVelXY[i + 2] = vt.y;
		}

		if (app->reset) {
			return 1;
		}
		else {
			return 0;
		}
	}

	std::array<float, 108> retrieveData() {
		std::array<float, 108> data;

		//userData
		
		//bullets
		for (int i = 0; i < 20; i++) {
			data[i] = api->bulletsPosXY[i];
			data[i + (int64_t)20] = api->bulletsVelXY[i];
		}
		//tanks
		for (int i = 0; i < 4; i++) {
			data[i + (int64_t)40] = api->tanksPosXY[i];
			data[i + (int64_t)44] = api->tanksVelXY[i];
		}
		//maze
		for (int i = 0; i < 30; i++) {
			data[i + (int64_t)48] = api->horizontalWalls[i];
			data[i+ (int64_t)78] = api->verticalWalls[i];
		}

		return data;
	}

	~TankTrouble() {
		//from constructor
		delete map;
		delete api;
		delete pos;
		delete world;
		delete cosTable;
	}
};*/

/*
int main(int argc, char* argv[]) {

	cosTableInit();

	memset(&app, 0, sizeof(App));

	initSDL();

	SDL_Texture* gameTexture = loadTexture("assets/tanktrouble_raw/game.png");

	SDL_Rect bulletSrc;
	bulletSrc.x = 639;
	bulletSrc.y = 92;
	bulletSrc.w = 9;
	bulletSrc.h = 9;

	SDL_Rect bulletDst;
	bulletDst.w = 9;
	bulletDst.h = 9;

	SDL_Rect light_grayTileSrc;
	light_grayTileSrc.x = 688;
	light_grayTileSrc.y = 181;
	light_grayTileSrc.w = 2;
	light_grayTileSrc.h = 2;
	SDL_Rect light_grayTileDst;
	light_grayTileDst.w = 100;
	light_grayTileDst.h = 100;

	SDL_Rect grayTileSrc;
	grayTileSrc.x = 300;
	grayTileSrc.y = 146;
	grayTileSrc.w = 2;
	grayTileSrc.h = 2;
	SDL_Rect grayTileDst;
	grayTileDst.w = 100;
	grayTileDst.h = 100;

//

//box2d section
	b2World* world = new b2World(b2Vec2(0, 0));

	//default is 1/24.0
	float timeStep = 1 / 24.0;      //the length of time passed to simulate (seconds)
	int32 velocityIterations = 8;   //how strongly to correct velocity
	int32 positionIterations = 8;   //how strongly to correct position

	world->Step(timeStep, velocityIterations, positionIterations);

	//NOTE: don't forget y-axis is decreasing function / upside down
	//boxes are draw at their corner meaning the box's physical center is offset by half their sides

	//maze
	std::vector<mazeFix*> walls;

	//bullet
	std::vector<Bullet*> bullets;

	//tank
	std::vector<tank*> tanks;

	tanks.push_back(new tank(world, 0, 0, 0));
	tanks.push_back(new tank(world, 0, 0, 0));

	b2Vec2* pos = new b2Vec2[2];
	float angles[2];

	SDL_SetTextureColorMod(tanks[0]->bodycolor, 245, 0, 0);
	SDL_SetTextureColorMod(tanks[1]->bodycolor, 0, 245, 0);

	std::set<Bullet*> bulletScheduledForRemoval;

	b2BodyDef mazeDef;
	mazeDef.type = b2_staticBody;
	mazeDef.position.Set(0, 0);
	b2Body* mazeBody = world->CreateBody(&mazeDef);

//main loop
	srand(time(0));

	END theEnd;
	const int mapSize = 5;
	int running = 1;
	std::vector<mazeWall> mazeState;
	mazeSetup map(mapSize);
	float bulletMaxVelocity = 4.6;
	std::vector<int> tiles;
	app.reset = 1;
	
	//api information
	Api api(mapSize, 2);

	while (running) {

		if (app.reset) {
			world->DestroyBody(mazeBody);
			mazeBody = world->CreateBody(&mazeDef);
			theEnd.is = false;
			app.reset = 0;
			//init maze
			while (1) {
				mazeState = InitMaze(mapSize);
				if (mazeState.empty() != 1) {
					for (mazeFix* wall : walls) {
						delete wall;
					}
					walls.clear();
					tiles.clear();
					walls = map.build(world, mazeState, 8, 15);
					for (int i = 0; i < walls.size(); i++) {
						mazeBody->CreateFixture(&walls[i]->bodyFix);
					}
					break;
				}
			}
			//create the array input to describe mazeSetup
			api.prepareWalls(map.topWalls, map.leftWalls, map.rightWalls, map.bottomWalls);

			// init tiles
			for (int i = 0; i < map.fullsize; i++) {
				if (std::find(map.blocks.begin(), map.blocks.end(), i) != map.blocks.end()) {
					tiles.push_back(0);
					continue;
				}
				double prob = getRand();
				if (prob > 0.75) {
					tiles.push_back(2);
				}
				else {
					tiles.push_back(1);
				}
			}

			//set player spawn
			for (int p = 0; p < tanks.size(); p++) {
				
				app.regularAmmo[p] = 5;
				app.regularCooldown[p].Reset();

				while (1) {
					double prob = getRand();
					double prob2 = getRand();
					int id = floor(prob * map.fullsize);

					if (std::find(map.blocks.begin(), map.blocks.end(), id) != map.blocks.end()) {
						continue;
					}

					int k = fmod(id, map.rowSize);
					int i = (id - k) / map.rowSize;
					int x = 8 + k * (map.l);
					int y = 15 + i * (map.l);

					float startAngle = prob2 * 2 * M_PI;
					tanks[p]->m_body->SetTransform(b2Vec2(x, y), startAngle);
					map.blocks.push_back(id);
					break;
				}
			}
		}

		world->Step(timeStep, velocityIterations, positionIterations);
		prepareScene();
		//draw tiles
		for (int i = 0; i < map.rowSize; i++) {
			for (int k = 0; k < map.rowSize; k++) {

				int cur_x = (8 + k * (map.l) - map.l / 2) * MTOPIXEL;
				int cur_y = (15 + i * (map.l) - map.l / 2) * MTOPIXEL;
				int tile_id = i * map.rowSize + k;

				if (tiles[tile_id] == 1) {
					light_grayTileDst.x = cur_x;
					light_grayTileDst.y = cur_y;
					SDL_RenderCopy(app.renderer, gameTexture, &light_grayTileSrc, &light_grayTileDst);
				}
				else if (tiles[tile_id] == 2) {
					grayTileDst.x = cur_x;
					grayTileDst.y = cur_y;
					SDL_RenderCopy(app.renderer, gameTexture, &grayTileSrc, &grayTileDst);
				}
			}
		}

		//draw walls
		for (mazeFix* it : walls) {
			it->draw(gameTexture);
		}

		doInput();

		for (int p = 0; p < tanks.size(); p++) {

			if (app.left[p] && app.right[p]) {
				tanks[p]->m_body->SetAngularVelocity(0);
			}
			else if (app.left[p]) {
				tanks[p]->m_body->SetAngularVelocity(-75 * DEGTORAD);
			}
			else if (app.right[p]) {
				tanks[p]->m_body->SetAngularVelocity(75 * DEGTORAD);
			}
			else {
				tanks[p]->m_body->SetAngularVelocity(0);
			}

			pos[p] = tanks[p]->m_body->GetPosition();
			angles[p] = tanks[p]->m_body->GetAngle();

			if (app.up[p] && app.down[p]) {
				tanks[p]->Vel.y = -1.1 * cosLookup(angles[p]);
				tanks[p]->Vel.x = 1.1 * sinLookup(angles[p]);
			}
			else if (app.up[p]) {
				tanks[p]->Vel.y = -4.2 * cosLookup(angles[p]);
				tanks[p]->Vel.x = 4.2 * sinLookup(angles[p]);
			}
			else if (app.down[p]) {
				tanks[p]->Vel.y = 3.2 * cosLookup(angles[p]);
				tanks[p]->Vel.x = -3.2 * sinLookup(angles[p]);
			}
			else {
				tanks[p]->Vel.x = 0;
				tanks[p]->Vel.y = 0;
			}

			tanks[p]->m_body->SetLinearVelocity(tanks[p]->Vel);

			//shoot
			//bullets can clip through wall if tanks are too close
			if (app.space[p]) {
				//best solution is to raytrace and look for instakills before shooting
				b2Vec2 pt = b2Vec2(pos[p].x + 3.5 * sin(angles[p]), pos[p].y - 3.5 * cos(angles[p]));
				bool insta = false;
				int insta_others = -1;
				
				for (b2Fixture* fixture = mazeBody->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
					if (fixture->TestPoint(pt)) {
						insta = true;
					}
				}

				//insta others
				for (int others = 0; others < tanks.size(); others++) {
					if (others != p) {
						for (b2Fixture* fixture = tanks[others]->m_body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
							if (fixture->TestPoint(pt)) {
								insta_others = others;
							}
						}
					}
				}

				if (insta_others != -1) {
					theEnd.is = true;
					theEnd.timer.Reset();
					tanks[insta_others]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);
					app.space[p] = 0;
					app.regularAmmo[p] += 1;

					bulletDst.x = pt.x * MTOPIXEL - 4.5;
					bulletDst.y = pt.y * MTOPIXEL - 4.5;
					SDL_RenderCopy(app.renderer, gameTexture, &bulletSrc, &bulletDst);

				}else if (insta) {
					theEnd.is = true;
					theEnd.timer.Reset();
					tanks[p]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);
					app.space[p] = 0;

					bulletDst.x = pt.x * MTOPIXEL - 4.5;
					bulletDst.y = pt.y * MTOPIXEL - 4.5;
					SDL_RenderCopy(app.renderer, gameTexture, &bulletSrc, &bulletDst);
				}
				else {
					app.space[p] = 0;
					b2Vec2 bulletVel = b2Vec2(bulletMaxVelocity * sin(angles[p]), -bulletMaxVelocity * cos(angles[p]));
					bullets.push_back(new Bullet(world, p, pt.x, pt.y, bulletVel));
				}

				tanks[p]->head_state = 9;
			}

			//draw tanks and handle deaths
			//draw tanks

			tanks[p]->body.posRect.x = (pos[p].x - tanks[p]->width * 0.5) * MTOPIXEL;
			tanks[p]->body.posRect.y = (pos[p].y - tanks[p]->height * 0.5) * MTOPIXEL;
			tanks[p]->body.angle = angles[p];
			blit(tanks[p]->bodycolor, tanks[p]->body.posRect, tanks[p]->body.angle, tanks[p]->body.center);
			blit(tanks[p]->body.texture, tanks[p]->body.posRect, tanks[p]->body.angle, tanks[p]->body.center);

			int thisHead = tanks[p]->head_state;
			float offy;

			if (thisHead > 0) {
				if (thisHead < 5 || thisHead > 8) {
					thisHead = 1;
					offy = 5;
				}
				else {
					thisHead = 2;
					offy = 0;
				}
				tanks[p]->head_state -= 1;
			}
			else {
				offy = 7;
			}

			tanks[p]->headPosRect[thisHead].x = (pos[p].x - tanks[p]->width * 0.5) * MTOPIXEL + 4;
			tanks[p]->headPosRect[thisHead].y = (pos[p].y - tanks[p]->height * 0.5) * MTOPIXEL - offy;
			blit(tanks[p]->headTextures[thisHead], tanks[p]->headPosRect[thisHead], tanks[p]->body.angle, tanks[p]->headCenter[thisHead]);


			//handle deaths
			//note: one bullet can kill 2 players which is not intended (but not worrying)
			for (b2ContactEdge* ce = tanks[p]->m_body->GetContactList(); ce; ce = ce->next) {
				if (ce->contact->IsTouching()) {
					for (Bullet* b : bullets) {
						if (ce->other == b->body) {
							theEnd.is = true;
							theEnd.timer.Reset();
							tanks[p]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);
							bulletScheduledForRemoval.insert(b);
							break;
						}
					}
				}
			}
		}

		//draw bullets and queue death
		for (Bullet* it: bullets) {
			float time = it->GetLifespan();
			if (time > 10000) {
				bulletScheduledForRemoval.insert(it);
			}
			b2Transform bulletTransform = it->GetTransform();
			//filledCircleRGBA(app.renderer, bulletTransform.p.x * MTOPIXEL, bulletTransform.p.y * MTOPIXEL, it->m_radius * MTOPIXEL, 255, 0, 0, 100);
			
			bulletDst.x = bulletTransform.p.x * MTOPIXEL -4.5;
			bulletDst.y = bulletTransform.p.y * MTOPIXEL -4.5;
			SDL_RenderCopy(app.renderer, gameTexture, &bulletSrc, &bulletDst);
		}

		//circleRGBA(app.renderer, pt.x * MTOPIXEL, pt.y * MTOPIXEL, 1, 0, 0, 255, 1000);
		presentScene();
		SDL_Delay(10);


		//handle end
		if (theEnd.is && theEnd.timer.GetMilliseconds() > 3000) {
			app.reset = 1;
			//remove bullets from map
			for (Bullet* it : bullets) {
				bulletScheduledForRemoval.insert(it);
			}
		}

		//process removal schedule
		std::set<Bullet*>::iterator it = bulletScheduledForRemoval.begin();
		std::set<Bullet*>::iterator end = bulletScheduledForRemoval.end();
		for (; it != end; ++it) {
			Bullet* dyingBall = *it;

			//delete ball... physics body is destroyed here
			delete dyingBall;

			//remove it from list of bullets
			std::vector<Bullet*>::iterator it = std::find(bullets.begin(), bullets.end(), dyingBall);
			if (it != bullets.end())
				bullets.erase(it);
		}
		//clear this list for next time
		bulletScheduledForRemoval.clear();


		//retrieve game information for api
		for (int i = 0; i < 20; i++) {
			api.bulletsPosXY[i] = 0;
			api.bulletsVelXY[i] = 0;
		}
		for (int i = 0; i < bullets.size(); i++) {

			b2Vec2 pb = bullets[i]->body->GetPosition();
			b2Vec2 vb = bullets[i]->body->GetLinearVelocity();

			api.bulletsPosXY[i] = pb.x;
			api.bulletsPosXY[i + 10] = pb.y;
			api.bulletsVelXY[i] = vb.x;
			api.bulletsVelXY[i + 10] = vb.y;

		}
		for (int i = 0; i < 2; i++) {
			b2Vec2 pt = tanks[i]->m_body->GetPosition();
			b2Vec2 vt = tanks[i]->m_body->GetLinearVelocity();
			
			api.tanksPosXY[i] = pt.x;
			api.tanksPosXY[i+2] = pt.y;
			api.tanksVelXY[i] = vt.x;
			api.tanksVelXY[i + 2] = vt.y;
		}
	}

	delete world;
	return 0;
}

*/