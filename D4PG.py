import Tankt
import Tanktask
import torch
from torch import nn
import numpy as np
import torch.nn.functional as F
from PR2L import agent as ag
from PR2L.training import distr_projection
from PR2L.experience import SimpleReplayBuffer
from PR2L.agent import float32_preprocessing
from collections import namedtuple

Experience = namedtuple("Experience", ["state","action", "reward", "next_state"])

class Actor(nn.Module):
    def __init__(self, inp, out):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(inp, 516),
            nn.ReLU(),
            nn.Linear(516, 516),
            nn.ReLU(),
            nn.Linear(516, out),
            nn.Sigmoid(),
        )

    def forward(self, x):
        return self.net(x)

class Critic(nn.Module):
    def __init__(self, inp, out):
        super().__init__()
        self.max = 1.0
        self.min = -1.0
        self.delta = 1.0
        self.register_buffer("atoms_weight", torch.arange(-1.0, 2.0, 1.0))
        print(self.atoms_weight)
        self.obs_net = nn.Sequential(
            nn.Linear(inp, 516),
            nn.ReLU(),
            nn.Linear(516, 516),
            nn.ReLU(),
            nn.Linear(516, 516),
            nn.ReLU(),
        )

        self.v_net = nn.Sequential(
            nn.Linear(516 + out, 516),
            nn.LeakyReLU(),
            nn.Linear(516, 256),
            nn.LeakyReLU(),
            nn.Linear(256, 3),
        )

    def forward(self, obs, act):
        hidden = self.obs_net(obs)
        cat = torch.cat((hidden, act), dim=1)
        values = self.v_net(cat)
        return values

    def getExpected(self, logits):
        e =  F.softmax(logits, dim=1) * self.atoms_weight
        e = e.sum(dim=1)
        return e

class AgentD4PG(ag.Agent):
    def __init__(self, act_net, device="cpu", epsilon=0.3):
        super().__init__()
        self.act_net = act_net
        self.device = device
        self.epsilon = epsilon
    
    @torch.no_grad()
    def __call__(self, obs, internal_states):
        states = float32_preprocessing(obs).to(self.device)
        mu_v = self.act_net(states)
        actions = mu_v.data.cpu().numpy()
        actions += self.epsilon * np.random.normal(size=actions.shape)
        return actions, internal_states

def makeEnv(render : bool = False):
    return Tankt.DualEnv(False, render, 1/24., 1)

BATCH_SIZE = 32
REPLAY_SIZE = 20000
ENV_NUM = 40
INPUT_SIZE = 111
OUTPUT_SIZE = 5
GAMMA = 0.99
LEARNING_RATE = 1e-4
device = "cuda" if torch.cuda.is_available() else "cpu"

class ExperienceSource:
    def __init__(self,render,  num_envs, act_net, device):
        self.agent = AgentD4PG(act_net, device)
        envs = [makeEnv() for _ in range(num_envs-1)]
        envs.append(makeEnv(render))
        self.envs = envs
    
    def __iter__(self):

        states1 = []
        states2 = []
        next_states1 = []
        next_states2 = []

        history = []
        
        for e in self.envs:
            obs1, obs2 = e.reset()
            states1.append(obs1)
            states2.append(obs2)

        reset_ids = []
        reset_obs1 = []
        reset_obs2 = []

        while(1):
            acts1, _ = self.agent(states1, None)
            acts2, _ = self.agent(states2, None)

            keys1 = acts1 > 0.5
            keys2 = acts2 > 0.5

            keys = np.concatenate((keys1, keys2), axis=1)

            for i, env in enumerate(self.envs):
                tuple1, tuple2 = env.step(keys[i])
                next_states1.append(tuple1[0])
                next_states2.append(tuple2[0])
                if (tuple1[2]):
                    exp1 = Experience(states1[i], acts1[i], tuple1[1], None)
                    exp2 = Experience(states2[i], acts2[i], tuple2[1], None)
                    reset_ids.append(i)
                    obs1, obs2 = env.reset()
                    reset_obs1.append(obs1)
                    reset_obs2.append(obs2)
                    history.extend([exp1, exp2])
                else:
                    exp1 = Experience(states1[i], acts1[i], tuple1[1], tuple1[0])
                    exp2 = Experience(states2[i], acts2[i],tuple2[1], tuple2[0])
                    history.extend([exp1, exp2])

            states1 = next_states1.copy()
            states2 = next_states2.copy()
            next_states1.clear()
            next_states2.clear()
            for i, id in enumerate(reset_ids):
                states1[id] = reset_obs1[i]
                states2[id] = reset_obs2[i]
            reset_obs1.clear()
            reset_obs2.clear()
            reset_ids.clear()
            yield history
            history.clear()
            
#this code is for N_STEPS 1
if __name__ == "__main__":

    actor = Actor(INPUT_SIZE, OUTPUT_SIZE).to(device)
    critic = Critic(INPUT_SIZE, OUTPUT_SIZE).to(device)
    
    tgt_actor = ag.TargetNet(actor)
    tgt_critic = ag.TargetNet(critic)

    crt_opt = torch.optim.Adam(critic.parameters(), lr =LEARNING_RATE)
    act_opt = torch.optim.Adam(actor.parameters(), lr =LEARNING_RATE)

    exp_source = ExperienceSource(False, ENV_NUM, actor, device)

    iter_exp = iter(exp_source)
    render_source = iter(ExperienceSource(True, 1, actor, device))
    history = []    
    idx = 0

    buffer = SimpleReplayBuffer(None, REPLAY_SIZE)

    while(1):
        next(render_source)
        if (idx % ENV_NUM == 0):
            his = next(iter_exp)
            for exp in his:
                buffer._add(exp)
        if (len(buffer) < 10000):
            continue
            
        states = []
        rewards = []
        actions = []
        next_states = []
        not_dones = []

        history = buffer.sample(BATCH_SIZE)

        for exp in history:
            state, action, reward, next_state = exp
            states.append(state)
            actions.append(action)
            rewards.append(reward)
            if (next_state is None):
                not_dones.append(False)
            else:
                not_dones.append(True)
                next_states.append(next_state)
        
        states = float32_preprocessing(states).to(device)
        actions = float32_preprocessing(actions).to(device)
        next_states = float32_preprocessing(next_states).to(device)
        rewards = float32_preprocessing(rewards).to(device)
        not_dones = np.array(not_dones, copy=False)
        #critic loss: log(q(x))p(x)
        crt_opt.zero_grad()
        q_v = critic(states, actions)
        logprob_q_v = F.log_softmax(q_v, dim=1)
        
        if len(next_states) > 0:
            next_acts = tgt_actor.target_model(next_states)
            next_q_v = tgt_critic.target_model(next_states, next_acts)

            next_dist_q_v = F.softmax(next_q_v, dim=1)
        else:
            next_dist_q_v = torch.Tensor([])

        refs_dist_q_v = distr_projection(next_dist_q_v, rewards, not_dones, GAMMA, 3, 1.0, -1.0, 1.0, device)

        crt_loss = -(refs_dist_q_v * logprob_q_v)
        crt_loss = crt_loss.sum(dim=1).mean()
        crt_loss.backward()

        #perform step
        crt_opt.step()

        #actor loss: -E(x)

        act_opt.zero_grad()

        acts = actor(states)
        q_v = critic(states, acts)
        e_v = critic.getExpected(q_v)
        act_loss = -e_v.mean()
        act_loss.backward()
        act_opt.step()

        tgt_actor.alpha_sync(alpha=1-1e-3)
        tgt_critic.alpha_sync(alpha=1-1e-3)
        idx += 1
        if (idx % 100 == 0):
            print(idx)