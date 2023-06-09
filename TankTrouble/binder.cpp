#include <pybind11/pybind11.h>
#include <game.h>
#include <structs.h>
#include <iostream>
#include <array>
#include <tuple>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <draw.h>
#include <SDL_image.h>

#undef main
namespace py = pybind11;


class Tanktask {
public:
	TankTrouble* game;
	App* app;

	Tanktask(bool enableUserInputs, bool enableRender, float userTimestep, float SDLDelay_ms, int seed) {
		app = new App(enableUserInputs);
		game = new TankTrouble(app, enableRender, userTimestep, SDLDelay_ms, seed);
	}

	std::tuple <std::array<float, 111>, float, std::array<float, 111>, float, bool> getTuple(std::array<float, 110> obs, bool done) {

		std::array<float, 111> obs1;
		std::array<float, 111> obs2;

		for (int i = 0; i < obs.size(); i++) {
			obs1[i] = obs[i];
			obs2[i] = obs[i];
		}
		//add player id
		obs1[110] = 0;
		obs2[110] = 1;

		float p1_reward = 0;
		float p2_reward = 0;
		if (done) {
			if (obs[40] < -700) {
				p1_reward = -1.0f;
			}
			else {
				p1_reward = 1.0f;
			}

			if (obs[41] < -700) {
				p2_reward = -1.0f;
			}
			else {
				p2_reward = 1.0f;
			}

			if (p2_reward == -1 && p1_reward == -1) {
				p2_reward = 0;
				p1_reward = 0;
			}

			if (game->selfkill[0])
				p1_reward -= 1;

			if (game->selfkill[1])
				p1_reward -= 1;
		}

		float punishment = game->internal_time * 0.00001;
		p1_reward -= punishment;
		p2_reward -= punishment;
		return std::make_tuple(obs1, p1_reward, obs2, p2_reward, done);
	}

	std::tuple <std::array<float, 111>, float, std::array<float, 111>, float, bool> step(std::array<bool, 10> inputs) {
		
		app->up[0] = inputs[0];
		app->down[0] = inputs[1];
		app->left[0] = inputs[2];
		app->right[0] = inputs[3];
		app->space[0] = inputs[4];
		app->up[1] = inputs[5];
		app->down[1] = inputs[6];
		app->left[1] = inputs[7];
		app->right[1] = inputs[8];
		app->space[1] = inputs[9];

		int done = game->step(app);

		std::array<float, 110> obs = game->retrieveData();

		return getTuple(obs, done);
	}

	std::tuple <std::array<float, 111>, float, std::array<float, 111>, float, bool> silentStep() {
		int done = game->step(app);
		std::array<float, 110> obs = game->retrieveData();

		return getTuple(obs, done);
	}

	void setInput(int player, std::array<bool, 5> inputs) {
		app->up[player] = inputs[0];
		app->down[player] = inputs[1];
		app->left[player] = inputs[2];
		app->right[player] = inputs[3];
		app->space[player] = inputs[4];
	}

	~Tanktask() {
		delete game;
		delete app;
	}

};

int main() {

	Tanktask game(true, true, 1 / 24., 10, 102294801);

	while (1) {
		game.silentStep();
	}

	return 0;
}


struct testWindow {

	App* app;

	testWindow() {
		app = new App(true);
		initSDL(app);
	}

	void drawBlack() {
		app->doInput();
		SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
		SDL_RenderClear(app->renderer);
		SDL_Delay(40);
		SDL_RenderPresent(app->renderer);
	}

	void drawPurple() {
		app->doInput();
		SDL_SetRenderDrawColor(app->renderer, 255, 0, 255, 255);
		SDL_RenderClear(app->renderer);
		SDL_Delay(40);
		SDL_RenderPresent(app->renderer);
	}

	//static path works but not relative
	void BlitImage() {
		SDL_Texture* texture = loadTexture(app, "assets/tankbody.png");
		SDL_Rect dst;
		dst.x = 100;
		dst.y = 100;
		dst.w = 200;
		dst.h = 500;
		SDL_Point pt;
		pt.x = 0;
		pt.y = 0;
		app->doInput();
		SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
		SDL_RenderClear(app->renderer);
		blit(app, texture, dst, 0, pt);
		SDL_RenderPresent(app->renderer);
		SDL_Delay(40);
	}

	int getError() {
		int imgFlags = IMG_INIT_PNG;
		if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
			std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
			SDL_Quit();
			return 1;
		}
		return 0;
	}

	~testWindow() {
		delete app;
	}
};

//current issue: loadTexture doesn't work relative to running script within interpreter, therefore we get the static path of images within interpreter


//Notes: the user reset button doesn't delete the bullets from map however it does if called from binded step function.
PYBIND11_MODULE(Tanktask, tt) {

	py::class_<Tanktask>(tt, "game")
		.def(py::init<bool, bool, float, float, int>(), py::arg("enableUserInputs"), py::arg("enableRender"), py::arg("timeStep"), py::arg("SDL_delay"), py::arg("seed"), " Tanktask unwrapped environment. 4 args-> enableUserInputs, enableRender, timeStep and SDL_delay, seed => -1 is for stochastic maze")
		/*.def("step", [](Tanktask& tt, py::array_t<bool, py::array::c_style | py::array::forcecast> python_input) {
		py::buffer_info buffer_info = python_input.request();
		if (buffer_info.size != 10) {
			throw std::runtime_error("Input must be an array of shape (10,)");
		}

		bool* action = static_cast<bool*>(buffer_info.ptr);
		return t.step(action); });*/
		.def("step", &Tanktask::step, py::arg("inputs"), "returns tuple(observation, player1_reward, player2_reward, done_flag).")
		.def("silentStep", &Tanktask::silentStep, "same behavior as step() but doesn't overwrites inputs (they are still handled and can be overwritten prior).")
		.def("setInput", &Tanktask::setInput, py::arg("player"), py::arg("inputs"), "Overwrites the inputs. 2 Args-> player id : {0,1} and inputs : array<bool, 5>.");

	py::class_<testWindow>(tt, "test")
		.def(py::init<>())
		.def("drawPurple", &testWindow::drawPurple)
		.def("drawBlack", &testWindow::drawBlack)
		.def("blit", &testWindow::BlitImage)
		.def("getError", &testWindow::getError);
}

