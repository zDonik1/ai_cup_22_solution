import random
import numpy as np
import matplotlib.pyplot as plt
from IPython.display import clear_output
from model import Game, Constants, Vec2, Item, Obstacle, Action, Unit


class ReplayBuffer:
    def __init__(self, capacity):
        self.capacity = capacity
        self.buffer = []
        self.position = 0

    def push(self, state, action, reward, next_state, done):
        if len(self.buffer) < self.capacity:
            self.buffer.append(None)
        self.buffer[self.position] = (state, action, reward, next_state, done)
        self.position = (self.position + 1) % self.capacity

    def sample(self, batch_size):
        batch = random.sample(self.buffer, batch_size)
        state, action, reward, next_state, done = [], [], [], [], []
        for entry in batch:
            s, a, r, ns, d = entry
            state.append(s)
            action.append(a)
            reward.append(r)
            next_state.append(ns)
            done.append(d)
        return state, action, reward, next_state, done

    def __len__(self):
        return len(self.buffer)


class ActionSpace:
    """
    parameters:
        low - a list of lower bounds for each dimension in action
        high - a list of upper bounds for each dimension in action
    """

    def __init__(self, low, high):
        self.low = np.array(low)
        self.high = np.array(high)
        self.shape = len(low)

    def normalized(self, action: np.ndarray):
        action = self.low + (action + 1.0) * 0.5 * (self.high - self.low)
        action = np.clip(action, self.low, self.high)
        return action

    def reverse_norm(self, action):
        action = 2 * (action - self.low) / (self.high - self.low) - 1
        action = np.clip(action, self.low, self.high)
        return action

    def sample(self):
        return [random.uniform(a, b) for a, b in zip(self.low, self.high)]


class ObservationSpace:
    def __init__(self, zone_attribs_size, zone_attribs, max_units, unit_attribs_size, unit_attribs, max_loot,
                 loot_attribs_size, loot_attribs, max_obstacles, obstacle_attribs_size, obstacle_attribs,
                 max_projectiles, projectile_attribs_size, projectile_attribs):
        self.zone_attribs = zone_attribs

        self.max_units = max_units
        self.unit_attribs_size = unit_attribs_size
        self.unit_attribs = unit_attribs

        self.max_loot = max_loot
        self.loot_attribs_size = loot_attribs_size
        self.loot_attribs = loot_attribs

        self.max_obstacles = max_obstacles
        self.obstacle_attribs_size = obstacle_attribs_size
        self.obstacle_attribs = obstacle_attribs

        self.max_projectiles = max_projectiles
        self.projectile_attribs_size = projectile_attribs_size
        self.projectile_attribs = projectile_attribs

        self.shape = zone_attribs_size + max_units * unit_attribs_size + max_loot * loot_attribs_size \
                     + max_obstacles * obstacle_attribs_size + max_projectiles * projectile_attribs_size

    def state(self, game: Game, constants: Constants):
        state = _make_state(game.zone, self.zone_attribs)

        enemy_states = []
        self_state = []
        self_unit = None
        for i in range(self.max_units):
            if i < len(game.units):
                if game.units[i].player_id == game.my_id:
                    self_state += _make_state(game.units[i], self.unit_attribs)
                    self_unit = game.units[i]
                else:
                    enemy_states += _make_state(game.units[i], self.unit_attribs)
            else:
                enemy_states += [0.0 for e in range(self.unit_attribs_size * (self.max_units - i))]
                break

        if self_unit is None:
            self_unit = Unit(0, 0, 0, 0, 0, Vec2(0, 0), 0, Vec2(0, 0), Vec2(0, 0), 0, None, 0, None, 0, [0, 0, 0], 0)

        state += self_state
        state += enemy_states

        state += _make_states(game.loot, self.max_loot, self.loot_attribs_size, self.loot_attribs)
        state += _make_states(list(filter(lambda obs: is_obstacle_close(self_unit.position, obs), constants.obstacles)),
                              self.max_obstacles, self.obstacle_attribs_size, self.obstacle_attribs)
        state += _make_states(game.projectiles, self.max_projectiles, self.projectile_attribs_size,
                              self.projectile_attribs)

        return state


def plot(frame_idx, rewards, n_episode):
    clear_output(True)
    plt.figure(figsize=(20, 5))
    plt.subplot(131)
    plt.title('frame %s. reward: %s' % (frame_idx, rewards[-1]))
    plt.plot(rewards)
    plt.savefig("reward_graph")
    plt.close()


def is_obstacle_close(position: Vec2, obstacle: Obstacle):
    x_bounds = (position.x - 25, position.x + 25)
    y_bounds = (position.y - 25, position.y + 25)
    return x_bounds[0] < obstacle.position.x < x_bounds[1] \
           and y_bounds[0] < obstacle.position.y < y_bounds[0]


def _to_floats(attrib):
    if type(attrib) is Vec2:
        return [attrib.x, attrib.y]
    elif type(attrib) is list:
        return [float(item) for item in attrib]
    elif isinstance(attrib, Item):
        return [float(attrib.TAG)]
    elif type(attrib) is Action:
        return [float(attrib.finish_tick), float(attrib.action_type)]
    elif attrib is None:
        return [0.0, 0.0]
    else:
        return [float(attrib)]


def _make_state(obj, attribs):
    state = []
    for attrib_name in attribs:
        state += _to_floats(getattr(obj, attrib_name))
    return state


def _make_states(objs, max_elem, attribs_size, obj_attribs):
    state = []
    for i in range(max_elem):
        if i < len(objs):
            state += _make_state(objs[i], obj_attribs)
        else:
            state += [0.0 for e in range(attribs_size * (max_elem - i))]
            break
    return state
