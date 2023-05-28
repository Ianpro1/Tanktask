import Tanktask as tt
import numpy as np

__all__ = ['DualEnv', 'playEnv']


class DualEnv:
    """
    A more user-friendly interface than the tt.game class with standard functions such as step and reset
    as well as catInputs : to concatenate both player's input numpy arrays.
    """
    def __init__(self, enableUserInputs : bool, render : bool, timeStep : float, SDL_render : float):
        self.game = tt.game(enableUserInputs, render, timeStep, SDL_render)
    
    def step(self, x):
        out = self.game.step(x)
        rew1 = out[1]
        rew2 = out[2]
        done = out[3]
        obs = out[0]
        return (obs, rew1, done, False, "id: player1"), (obs, rew2, done, False, "id: player2")
    
    def catInputs(self, x1, x2):
        return np.concatenate([x1, x2], axis=0)
    
    def reset(self):
        out = self.game.silentStep()[0]
        return (out, "id: player1"), (out, "id: player2")


class playEnv:
    """
    Environment for single player bot : ids are {0, 1} for player 1 and player 2.
    This environment is meant to allow users to play against a bot or AI.
    """
    def __init__(self, timeStep : float, SDL_render : float, bot_id : int):
        self.game = tt.game(True, True, timeStep, SDL_render)
        self.id = bot_id
    
    def step(self, x):
        self.game.setInput(self.id, x)
        out = self.game.silentStep()
        return (out[0], out[self.id + 1], out[3], False, "play environment")
    
    def reset(self):
        out = self.game.silentStep()[0]
        return (out, "play environment")

