import math
from typing import Optional
import timeit
import numpy as np
from model import Game, Obstacle
from model.order import Order
from model.unit_order import UnitOrder
from model.action_order import *
from model.constants import Constants
from model.vec2 import Vec2
from debug_interface import DebugInterface
from sacnet.train import Trainer
from sacnet.utils import plot, is_obstacle_close

max_games = 40000
batch_size = 128
n_discrete_actions = 4  # none, shoot, pick up loot, drink potion


class MyStrategy:
    def __init__(self):
        self.constants = None
        self.time_point = 0
        self.n_episode = 1

        self.episode_reward = 0
        self.trainer = Trainer(batch_size, n_discrete_actions)
        self.frame_idx = 0
        self.rewards = []
        self.last_state = None
        self.last_unit_state = None
        self.unit = None
        self.last_damage = 0
        self.last_score = 0

    def reset(self, constants: Constants):
        self.constants = constants
        self.trainer.reset(constants)

    def get_order(self, game: Game, debug_interface: Optional[DebugInterface]) -> Order:
        for unit in game.units:
            if unit.player_id == game.my_id:
                self.last_unit_state = self.unit
                self.unit = unit

        order = self._train(game, False)
        self.last_state = game

        # current_time = timeit.timeit()
        # print("{} {}".format(1 / current_time - self.time_point, order))
        # self.time_point = current_time

        return Order({self.unit.id: order})

    def debug_update(self, debug_interface: DebugInterface):
        pass

    def finish(self):
        self._train(self.last_state, True)
        print("frame {} reward {}".format(self.frame_idx, self.episode_reward))
        self.rewards.append(self.episode_reward)
        plot(self.frame_idx, self.rewards, self.n_episode)
        self.episode_reward = 0
        self.n_episode += 1

    def _train(self, game: Game, done):
        reward = self._calc_reward(game)
        self.episode_reward += reward
        self.frame_idx += 1

        action_vec = self.trainer.tick(self.frame_idx, self.last_state, game, reward, done, self.n_episode)

        # converting action continuous value to discrete actions
        action = None
        action_idx = int(action_vec[4] * n_discrete_actions)
        if action_idx == 1:
            action = Aim(True)
        elif action_idx == 2:
            closest = None
            min_dist = float('inf')
            for loot in game.loot:
                dist = _sqr_dist(self.unit.position, loot.position)
                if dist < min_dist:
                    min_dist = dist
                    closest = loot.id
            if closest is not None:
                action = Pickup(closest)
        elif action_idx == 3:
            action = UseShieldPotion()

        move = np.array([action_vec[0], action_vec[1]])
        move = move / np.linalg.norm(move) * self.constants.max_unit_forward_speed
        return UnitOrder(Vec2(move[0], move[1]), Vec2(action_vec[2], action_vec[3]), action)

    def _calc_reward(self, game: Game):
        if self.last_unit_state is None:
            return 0

        self_player = None
        for player in game.players:
            if player.id == game.my_id:
                self_player = player

        reward = 0

        # doing damage
        if self_player.damage > self.last_damage:
            reward += 15
            self.last_damage = self_player.damage

        # looting shield
        if self.unit.shield_potions > self.last_unit_state.shield_potions:
            reward += 20

        # regenerating
        if self.unit.health > self.last_unit_state.health or self.unit.shield > self.last_unit_state.shield:
            reward += 10

        # looting ammo
        if self.unit.ammo[self.unit.weapon.type_index] > self.last_unit_state.ammo[self.unit.weapon.type_index]:
            reward += 5

        # hitting obstacles
        obstacles = list(filter(lambda obs: is_obstacle_close(self.unit.position, obs),
                                self.constants.obstacles))
        if len(obstacles) > 0:
            closest_obs = min(obstacles, key=lambda obs: _sqr_dist(self.unit.position, obs.position))
            dist = math.sqrt(_sqr_dist(closest_obs.position, self.unit.position))
            if dist - closest_obs.radius - self.constants.unit_radius < 0.2:
                reward -= 7

        # taking damage
        if self.unit.health < self.last_unit_state.health or self.unit.shield < self.last_unit_state.shield:
            reward -= 25

        return reward


def _sqr_dist(v1: Vec2, v2: Vec2):
    vec = [v1.x - v2.x, v1.y - v2.y]
    return vec[0] * vec[0] + vec[1] * vec[1]
