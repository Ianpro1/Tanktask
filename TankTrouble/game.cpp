#include <game.h>
//#include <SDL2_rotozoom.h>
#include <stdio.h>
#include <input.h>
#include <draw.h>
#include <defs.h>
#include <math.h>

using namespace std;

TankTrouble::TankTrouble(App* app, bool render, float userTimestep, float SDLDelay_ms, int seed) {
	first_time = true;
	renderDelay_ms = SDLDelay_ms;
	SDL_render = render;

	cosTable = new float[tableSize];
	cosTableInit();

	if (render) {
		initSDL(app);
		gameTexture = loadTexture(app, "assets/game.png");
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
	timeStep = userTimestep;      //the length of time passed to simulate (seconds)
	world->Step(timeStep, velocityIterations, positionIterations);
	tanks.push_back(new tank(app, world, 0, 0, 0, render));
	tanks.push_back(new tank(app, world, 0, 0, 0, render));
	pos = new b2Vec2[2];

	if (SDL_render) {
		SDL_SetTextureColorMod(tanks[0]->bodycolor, 245, 0, 0);
		SDL_SetTextureColorMod(tanks[1]->bodycolor, 0, 245, 0);
	}


	mazeDef.type = b2_staticBody;
	mazeDef.position.Set(0, 0);
	mazeBody = world->CreateBody(&mazeDef);
	m_seed = seed;
	if (seed == -1) {
		srand(time(0));
	}
	else {
		srand(seed);
	}
	map = new mazeSetup(app, mapSize);
	api = new Api(mapSize, 2);
}

void TankTrouble::cosTableInit() {

	for (int i = 0; i < tableSize; i++) {
		cosTable[i] = cos(M_PI * i / tableSize);
	}
}

float TankTrouble::cosLookup(float x) {

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

float TankTrouble::sinLookup(float x) {
	return cosLookup(x - M_PI / 2);
}

bool TankTrouble::step(App* app) {

	if (app->reset) {
		internal_time = 0;
		space_pressed[0].Reset(0);
		space_pressed[1].Reset(0);
		theEnd.timer.Reset(0);
		theEnd.is = false;
		app->reset = 0;

		if (m_seed == -1 || first_time == true) {
			world->DestroyBody(mazeBody);
			mazeBody = world->CreateBody(&mazeDef);
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
		}

		if (first_time != true && m_seed != -1) {
			map->blocks.pop_back();
			map->blocks.pop_back();
		}
		first_time = false;

		//set player spawn
		//angle
		double prob2 = getRand();
		float startAngle = prob2 * 2 * M_PI;
		//position
		for (int p = 0; p < tanks.size(); p++) {

			tanks[p]->regularAmmo = 5;
			selfkill[p] = false;

			while (1) {
				double prob = getRand();
				int id = floor(prob * map->fullsize);

				if (std::find(map->blocks.begin(), map->blocks.end(), id) != map->blocks.end()) {
					continue;
				}

				int k = fmod(id, map->rowSize);
				int i = (id - k) / map->rowSize;
				int x = 8 + k * (map->l);
				int y = 15 + i * (map->l);

				tanks[p]->m_body->SetTransform(b2Vec2(x, y), startAngle + (p * M_PI));
				map->blocks.push_back(id);
				break;
			}
		}
	}

	world->Step(timeStep, velocityIterations, positionIterations);
	internal_time += 1;

	if (SDL_render) {
		prepareScene(app);
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
		if (app->space[p] && tanks[p]->regularAmmo > 0 && space_pressed[p].GetTimeSteps(internal_time) > 5) {
			space_pressed[p].Reset(internal_time);
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

			app->space[p] = 0;

			if (insta_others != -1) {
				theEnd.is = true;
				theEnd.timer.Reset(internal_time);
				tanks[insta_others]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);

				if (SDL_render) {
					bulletDst.x = pt.x * MTOPIXEL - 4.5;
					bulletDst.y = pt.y * MTOPIXEL - 4.5;
					SDL_RenderCopy(app->renderer, gameTexture, &bulletSrc, &bulletDst);
				}
			}
			else if (insta) {
				theEnd.is = true;
				theEnd.timer.Reset(internal_time);
				tanks[p]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);

				if (SDL_render) {
					bulletDst.x = pt.x * MTOPIXEL - 4.5;
					bulletDst.y = pt.y * MTOPIXEL - 4.5;
					SDL_RenderCopy(app->renderer, gameTexture, &bulletSrc, &bulletDst);
				}
			}
			else {
				b2Vec2 bulletVel = b2Vec2(bulletMaxVelocity * sin(angles[p]), -bulletMaxVelocity * cos(angles[p]));
				bullets.push_back(new Bullet(world, p, pt.x, pt.y,internal_time, bulletVel));
				tanks[p]->regularAmmo -= 1;
			}

			tanks[p]->head_state = 9;
		}

		//draw tanks and handle deaths
		//draw tanks
		if (SDL_render) {

			tanks[p]->body.posRect.x = (pos[p].x - tanks[p]->width * 0.5) * MTOPIXEL;
			tanks[p]->body.posRect.y = (pos[p].y - tanks[p]->height * 0.5) * MTOPIXEL;
			tanks[p]->body.angle = angles[p];
			blit(app, tanks[p]->bodycolor, tanks[p]->body.posRect, tanks[p]->body.angle, tanks[p]->body.center);
			blit(app, tanks[p]->body.texture, tanks[p]->body.posRect, tanks[p]->body.angle, tanks[p]->body.center);

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
			blit(app, tanks[p]->headTextures[thisHead], tanks[p]->headPosRect[thisHead], tanks[p]->body.angle, tanks[p]->headCenter[thisHead]);
		}

		//handle deaths
		//note: one bullet can kill 2 players which is not intended (but not worrying)
		for (b2ContactEdge* ce = tanks[p]->m_body->GetContactList(); ce; ce = ce->next) {
			if (ce->contact->IsTouching()) {
				for (Bullet* b : bullets) {
					if (ce->other == b->body) {
						theEnd.is = true;
						theEnd.timer.Reset(internal_time);
						tanks[p]->m_body->SetTransform(b2Vec2(-1000, -1000), 0);
						bulletScheduledForRemoval.insert(b);
						if (b->id == p) {
							selfkill[p] = true;
						}
						break;
					}
				}
			}
		}
	}

	//draw bullets and queue death
	for (Bullet* it : bullets) {
		float time = it->GetLifespan(internal_time, timeStep);
		if (time > 40) {
			bulletScheduledForRemoval.insert(it);
		}
	}

	if (SDL_render) {
		//draw bullets
		for (Bullet* it : bullets) {
			b2Transform bulletTransform = it->GetTransform();
			//filledCircleRGBA(app->renderer, bulletTransform.p.x * MTOPIXEL, bulletTransform.p.y * MTOPIXEL, it->m_radius * MTOPIXEL, 255, 0, 0, 100);
			bulletDst.x = bulletTransform.p.x * MTOPIXEL - 4.5;
			bulletDst.y = bulletTransform.p.y * MTOPIXEL - 4.5;
			SDL_RenderCopy(app->renderer, gameTexture, &bulletSrc, &bulletDst);
		}

		//circleRGBA(app->renderer, pt.x * MTOPIXEL, pt.y * MTOPIXEL, 1, 0, 0, 255, 1000);
		presentScene(app);
		SDL_Delay(renderDelay_ms);
	}

	//handle end
	if (theEnd.is && theEnd.timer.GetSeconds(internal_time, timeStep) > 10) {
		app->reset = 1;
	}

	if (app->reset)
	{
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

		tanks[dyingBall->id]->regularAmmo += 1;
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

	app->doInput();

	if (app->reset) {
		return 1;
	}
	else {
		return 0;
	}
}

std::array<float, 110> TankTrouble::retrieveData() {
	std::array<float, 110> data;

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
	//tank1 pos X at 40, tank2 posX at 41

	//maze
	for (int i = 0; i < 30; i++) {
		data[i + (int64_t)48] = api->horizontalWalls[i];
		data[i + (int64_t)78] = api->verticalWalls[i];
	}
	data[108] = tanks[0]->m_body->GetAngle();
	data[109] = tanks[1]->m_body->GetAngle();

	return data;
}

TankTrouble::~TankTrouble() {
	//from constructor
	delete map;
	delete api;
	delete pos;
	delete world;
	delete cosTable;
}
