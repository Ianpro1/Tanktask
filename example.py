import Tankt
import Tanktask
#after you've installed the library you can run this script to play the game

#DualEnv is an environment that returns 2 tuples of (observation, reward, done_flag) when calling step() or 2 (observation) on reset()
dualEnv = Tankt.DualEnv(False, False, 1/24., 1, True)

#The following class documentation can be found in source => TankTrouble/binder.cpp

env = Tanktask.game(True, True, 1/24., 10, -1)

while (1):
    env.silentStep()
