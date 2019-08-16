from typing import List
import json
import time
from map_info import MapInfo, Point


class State:
    def __init__(self):
        pass

    def step(self, agents):
        pass


class Agents:
    def __init__(self, _id_list: List[int], _loc_list: List[Point], _is_attack: bool):
        self.id_list = _id_list
        self.loc_list = _loc_list
        self.is_attack = _is_attack

    def get_action(self, state: State):
        pass


class Game:
    def __init__(self, _attack_agents: Agents, _defence_agents: Agents, _map_json):
        self.map_info = MapInfo(_map_json)
        self.game_over = False  # type: bool

    def run(self):
        while not self.game_over:
            pass


if __name__ == '__main__':
    pass

