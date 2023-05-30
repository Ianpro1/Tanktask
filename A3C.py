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
from collections import namedtuple, deque
from PR2L import utilities
from PR2L.rendering import pltprint
import torch.multiprocessing as mp
import time
Experience = namedtuple("Experience", ["state","action", "reward", "next_state"])

def makeEnv(render : bool = False):
    return Tankt.DualEnv(False, render, 1/24., 1)

ENV_NUM = 30
INPUT_SIZE = 111
OUTPUT_SIZE = 5
GAMMA = 0.99
LEARNING_RATE = 1e-4
BETA_ENTROPY = 1.0
BETA_POLICY = 0.5
PROCESS_COUNT = 3
device = "cuda" if torch.cuda.is_available() else "cpu"

class A3CAgent(ag.Agent):
    def __init__(self, net, device):
        self.net = net
        self.device = device

    @torch.no_grad()
    def __call__(self, x, internal_states):
        x = float32_preprocessing(x).to(self.device)
        act_v = self.net(x)[0]
        
        act_v = act_v.data.cpu().numpy()
        keys = []
        for row_v in act_v:
            
            rand = np.random.uniform(0., 1., size=len(row_v))
            k = row_v < rand
            keys.append(k)
        return keys, internal_states

class ExperienceSource:
    def __init__(self,render, num_envs, act_net1, act_net2, device1, device2):
        self.agent1 = A3CAgent(act_net1, device1)
        self.agent2 = A3CAgent(act_net2, device2)
        envs = [makeEnv() for _ in range(num_envs-1)]
        envs.append(makeEnv(render))
        self.envs = envs
    
    def __iter__(self):

        states1 = []
        states2 = []
        next_states1 = []
        next_states2 = []

        history1 = []
        history2 = []
        for e in self.envs:
            obs1, obs2 = e.reset()
            states1.append(obs1)
            states2.append(obs2)

        reset_ids = []
        reset_obs1 = []
        reset_obs2 = []

        while(1):
            acts1, _ = self.agent1(states1, None)
            acts2, _ = self.agent2(states2, None)

            keys = np.concatenate((acts1, acts2), axis=1)

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
                    history1.append(exp1)
                    history2.append(exp2)
                else:
                    exp1 = Experience(states1[i], acts1[i], tuple1[1], tuple1[0])
                    exp2 = Experience(states2[i], acts2[i],tuple2[1], tuple2[0])
                    history1.append(exp1)
                    history2.append(exp2)

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
            yield history1, history2
            history1.clear()
            history2.clear()

class ActorCritic(nn.Module):
    def __init__(self, input_shape, n_actions):
        super().__init__()
        self.lin = nn.Sequential(
            nn.BatchNorm1d(input_shape),
            nn.Linear(input_shape, 512),
            nn.ReLU(),
            nn.Linear(512, 512),
            nn.ReLU(),
            nn.Linear(512, 512),
            nn.ReLU(),
            nn.Linear(512, 512),
            nn.ReLU(),
        )
        
        self.value = nn.Sequential(
            nn.Linear(512, 512),
            nn.ReLU(),
            nn.Linear(512, 256),
            nn.ReLU(),
            nn.Linear(256, 1)
        )
        
        self.policy = nn.Sequential(
            nn.Linear(512, 512),
            nn.ReLU(),
            nn.Linear(512, 256),
            nn.ReLU(),
            nn.Linear(256, n_actions),
            nn.Sigmoid()
        )
  
    def forward(self, x):
        x = self.lin(x)
        act_v = self.policy(x)
        value = self.value(x)
        return act_v, value

def network_reset(layer):
    if isinstance(layer, (nn.Linear, nn.Conv2d, nn.BatchNorm2d, nn.BatchNorm1d)):
        layer.reset_parameters()
    
def play_env(render, env_number, net1, net2, queue, device1="cpu", device2="cpu"):
    exp_source = ExperienceSource(render, env_number, net1, net2, device1, device2)
    minibatch = []

    for his1, his2 in exp_source:
        
        minibatch.extend(his1)
        minibatch.extend(his2)
        queue.put(minibatch.copy())
        minibatch.clear()

class BatchGenerator:
    def __init__(self, exp_queue, PROCESS_COUNT):
        self.queue = exp_queue
        self.PROCESS_COUNT = PROCESS_COUNT
        self.rewards = []
        self.steps = []
    
    def __iter__(self):
        batch = []
        idx = 0
        while True:
            exp = self.queue.get()         
            idx +=1
            batch.extend(exp)

            if idx % self.PROCESS_COUNT == 0:
                yield batch
                batch.clear()


if __name__ == "__main__":

    exp_queue = mp.Queue(maxsize=PROCESS_COUNT)

    net = ActorCritic(INPUT_SIZE, OUTPUT_SIZE).to(device)
    tgt_net = ag.TargetNet(net)

    iter_source = iter(BatchGenerator(exp_queue, PROCESS_COUNT))

    process = []

    process.append(mp.Process(target=play_env, args=(True, ENV_NUM, net, tgt_net.target_model, exp_queue, device, device)))

    for _ in range(PROCESS_COUNT-1):
        process.append(mp.Process(target=play_env, args=(False, ENV_NUM, net, net, exp_queue, device, device)))
    for p in process:
        p.start()
    
    opt = torch.optim.Adam(net.parameters(), lr=LEARNING_RATE)

    net.apply(network_reset)
    tgt_net.sync()
    time.sleep(1)
    modelmanager = utilities.ModelBackupManager("A3C", "001", [net])
    idx = 0 
    plot = pltprint(color="red", alpha = 0.4)
    while(1):
        idx += 1
        if (idx % 100 == 0):
            print(idx)
        if (idx % 100000 == 0):
            modelmanager.save()

        history = next(iter_source)

        states = []
        rewards = []
        actions = []
        next_states = []
        not_dones = []
        for exp in history:
            states.append(exp.state)
            actions.append(exp.action)
            rewards.append(exp.reward)
            if (exp.next_state is None):
                not_dones.append(False)
            else:
                not_dones.append(True)
                next_states.append(exp.next_state)
        
        states = float32_preprocessing(states).to(device)
        #expect actions as bool not float!
        actions = float32_preprocessing(actions).to(device)
        rewards = float32_preprocessing(rewards).to(device)
        next_states = float32_preprocessing(next_states).to(device)
        not_dones = np.array(not_dones, copy=False)

        #loss in 3 parts : MSE, REINFORCE, entropy
        with torch.no_grad():
            if (len(next_states) > 0):
                next_refs_q_v = torch.zeros_like(rewards)
                q_v = net(next_states)[1].squeeze(-1)
                next_refs_q_v[not_dones] = q_v

                refs_q_v = rewards + next_refs_q_v * GAMMA
            else:
                refs_q_v = rewards

        refs_q_v = refs_q_v.unsqueeze(-1)
        opt.zero_grad()

        acts, values = net(states)

        if (idx % 10 == 0):    
            a = acts.data.cpu().numpy()
            plot.drawbuffer(a[0])

        values = values
        mse_loss = F.mse_loss(refs_q_v, values)
        
        #get the complementary probabilities
        p_a = actions - acts
        p_a = torch.absolute(p_a)
        logprob_a = torch.log10(p_a)

        adv = refs_q_v - values.detach()

        policy_loss = -(logprob_a * adv).sum(dim=1).mean()
        
        variance_loss = F.mse_loss(acts.mean(), torch.tensor(0.5).to(device))

        loss = BETA_ENTROPY * variance_loss + mse_loss + BETA_POLICY * policy_loss
        loss.backward()
        opt.step()

        if (idx % 1000 == 0):
            tgt_net.alpha_sync(1-1e-3)
        