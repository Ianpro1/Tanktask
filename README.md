# Tanktask

I made a copy of Tanktrouble (without the special weapons) and made python bindings as well in order to train a reinforcement learning agent.
For those who wants to make their own Tanktrouble agent (AI) and download the python module, follow the instructions and look at example.py

Also, the bindings can be found in TankTrouble/binder.cpp, so you can change the reward system from there. And to change the observations you'll have to modify the last lines in step() of game.cpp in the api section.
Finally, there's no cmake to build a visual studio project, but linking is pretty straight foward. Just look through CMakeLists.

<img src="https://github.com/Ianpro1/Tanktask/blob/master/Tanktask.gif" width="500">

******

INSTALLATION (FOR WINDOWS ONLY):

```
git clone --recursive https://github.com/Ianpro1/Tanktask.git
pip install ./Tanktask
```
