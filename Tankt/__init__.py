import Tanktask as tt

__all__ = ['DualEnv']


class DualEnv:
    """
    A more user-friendly interface than the tt.game class with standard functions such as step and reset
    as well as catInputs : to concatenate both player's input numpy arrays.
    """
    def __init__(self, enableUserInputs : bool, render : bool, timeStep : float, SDL_render : float, maze_randomized : bool, seed : int = 31):
        
        if (maze_randomized):
            self.game = tt.game(enableUserInputs, render, timeStep, SDL_render, -1)
        else:
            self.game = tt.game(enableUserInputs, render, timeStep, SDL_render, seed)
    
    def step(self, x) -> tuple[tuple[tuple, float, bool], tuple[tuple, float, bool]]:
        """
        returns 2 tuples of (observation, reward, termination flag).
        """
        out = self.game.step(x)
        obs1 = out[0]
        rew1 = out[1]
        obs2 = out[2]
        rew2 = out[3]
        done = out[4]
        return (obs1, rew1, done), (obs2, rew2, done)
    
    def reset(self)-> tuple[tuple, tuple]:
        """
        returns 2 observation tuples without requiring inputs.
        """
        out = self.game.silentStep()
        obs1 = out[0]
        obs2 = out[2]
        return obs1, obs2


#class playEnv:
   # """
   # Environment for single player bot : ids are {0, 1} for player 1 and player 2.
   # This environment is meant to allow users to play against a bot or AI.
   # """
 #   def __init__(self, timeStep : float, SDL_render : float, bot_id : int):
  #      self.game = tt.game(True, True, timeStep, SDL_render)
   #     self.id = bot_id
    
    #def step(self, x):
     #   self.game.setInput(self.id, x)
      #  out = self.game.silentStep()
       # return (out[0], out[self.id + 1], out[3], False, "play environment")
    
    #def reset(self):
     #   out = self.game.silentStep()[0]
      #  return (out, "play environment")

