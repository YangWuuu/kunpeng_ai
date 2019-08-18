import time
import json
import random
from abc import abstractmethod
from copy import deepcopy
from typing import List, Dict
from map_info import MapInfo, Point, DIRECTION, map_str_to_json

random.seed(2019)


class State:
    def __init__(self, _defence_agents, _attack_agents, _map_info: MapInfo):
        self.map_info = _map_info
        self.defence_agents = _defence_agents  # type: Agents
        self.attack_agents = _attack_agents  # type: Agents
        self.game_over = False  # type: bool

    def __deepcopy__(self, memo):
        if not memo:
            memo = None
        result = self.__class__(self.defence_agents, self.attack_agents, self.map_info)
        result.defence_agents = deepcopy(self.defence_agents)
        result.attack_agents = deepcopy(self.attack_agents)
        return result

    def show_map(self):
        map_str = [[" ." for _ in range(self.map_info.width)] for __ in range(self.map_info.height)]
        for i in range(self.map_info.width):
            for j in range(self.map_info.height):
                point = self.map_info.maps[i][j]
                if point.wall:
                    map_str[i][j] = " #"
                elif point.tunnel == DIRECTION.UP:
                    map_str[i][j] = " ^"
                elif point.tunnel == DIRECTION.DOWN:
                    map_str[i][j] = " v"
                elif point.tunnel == DIRECTION.LEFT:
                    map_str[i][j] = " <"
                elif point.tunnel == DIRECTION.RIGHT:
                    map_str[i][j] = " >"
                elif point.name != "":
                    map_str[i][j] = " " + point.name
        for idx in self.defence_agents.id_loc:
            point = self.defence_agents.id_loc[idx]
            map_str[point.x][point.y] = "d{}".format(idx)
        for idx in self.attack_agents.id_loc:
            point = self.attack_agents.id_loc[idx]
            map_str[point.x][point.y] = "a{}".format(idx)
        return "\n".join(
            ["".join([map_str[i][j] for i in range(self.map_info.width)]) for j in range(self.map_info.height)])

    def apply_actions(self, actions: Dict[int, DIRECTION]):
        for agent_id in actions:
            action = actions[agent_id]
            if agent_id in self.defence_agents.id_loc:
                self.defence_agents.id_loc[agent_id] = self.defence_agents.id_loc[agent_id].next[action]
            else:
                self.attack_agents.id_loc[agent_id] = self.attack_agents.id_loc[agent_id].next[action]
        dead_agent_id = list()
        for defence_id in self.defence_agents.id_loc:
            for attack_id in self.attack_agents.id_loc:
                if self.defence_agents.id_loc[defence_id] == self.attack_agents.id_loc[attack_id]:
                    if defence_id not in dead_agent_id:
                        dead_agent_id.append(defence_id)
        for idx in dead_agent_id:
            del self.defence_agents.id_loc[idx]
        if len(self.defence_agents.id_loc) == 0:
            self.game_over = True


class Agents:
    def __init__(self, _id_loc: Dict[int, Point]):
        self.id_loc = _id_loc  # type: Dict[int, Point]

    def __deepcopy__(self, memo):
        if not memo:
            memo = None
        result = self.__class__(self.id_loc)
        result.id_loc = self.id_loc.copy()
        return result

    @abstractmethod
    def get_actions(self, _state: State) -> Dict[int, DIRECTION]:
        ret = dict()
        for idx in self.id_loc:
            ret[idx] = random.choice([DIRECTION.UP, DIRECTION.DOWN, DIRECTION.LEFT, DIRECTION.RIGHT, DIRECTION.NONE])
        return ret


class DefenceAgents(Agents):
    def get_actions(self, _state: State):
        ret = dict()
        for idx in self.id_loc:
            choices = list()
            for direction in [DIRECTION.UP, DIRECTION.DOWN, DIRECTION.LEFT, DIRECTION.RIGHT, DIRECTION.NONE]:
                is_safe = True
                next_loc = self.id_loc[idx].next[direction]
                for attack_idx in _state.attack_agents.id_loc:
                    if _state.map_info.path.get_cost(next_loc, _state.attack_agents.id_loc[attack_idx]) <= 1 or \
                       _state.map_info.path.get_cost(_state.attack_agents.id_loc[attack_idx], next_loc) <= 1:
                        is_safe = False
                if is_safe:
                    choices.append(direction)
            if len(choices) == 0:
                for direction in [DIRECTION.UP, DIRECTION.DOWN, DIRECTION.LEFT, DIRECTION.RIGHT, DIRECTION.NONE]:
                    is_safe = True
                    next_loc = self.id_loc[idx].next[direction]
                    for attack_idx in _state.attack_agents.id_loc:
                        if _state.map_info.path.get_cost(next_loc, _state.attack_agents.id_loc[attack_idx]) <= 0 or \
                           _state.map_info.path.get_cost(_state.attack_agents.id_loc[attack_idx], next_loc) <= 0:
                            is_safe = False
                    if is_safe and direction not in choices:
                        choices.append(direction)
            ret[idx] = random.choice(choices)
        return ret


class AttackAgents(Agents):
    def get_actions(self, _state: State):
        ret = dict()
        for idx in self.id_loc:
            choices = list()
            min_cost_loc = None
            min_cost = 9999
            for defence_idx in _state.defence_agents.id_loc:
                cost = _state.map_info.path.get_cost(self.id_loc[idx], _state.defence_agents.id_loc[defence_idx])
                if cost < min_cost:
                    min_cost = cost
                    min_cost_loc = _state.defence_agents.id_loc[defence_idx]
            for direction in [DIRECTION.UP, DIRECTION.DOWN, DIRECTION.LEFT, DIRECTION.RIGHT, DIRECTION.NONE]:
                if _state.map_info.path.get_cost(self.id_loc[idx].next[direction], min_cost_loc) \
                        < _state.map_info.path.get_cost(self.id_loc[idx], min_cost_loc):
                    choices.append(direction)
            ret[idx] = random.choice(choices)
        return ret


class Game:
    def __init__(self, _state: State):
        self.state = _state  # type: State
        self.defence_agents = self.state.defence_agents  # type: Agents
        self.attack_agents = self.state.attack_agents  # type: Agents
        self.round_id = 0  # type: int

    def run(self):
        while True:
            print("\n\nround id: {}:".format(self.round_id))
            print(self.state.show_map())
            if self.round_id == 28:
                print("test")
            defence_actions = self.defence_agents.get_actions(self.state)
            attack_actions = self.attack_agents.get_actions(self.state)
            print("defence actions: {}".format(defence_actions))
            print("attack actions: {}".format(attack_actions))
            self.state.apply_actions(defence_actions)
            self.state.apply_actions(attack_actions)
            if self.state.game_over or self.round_id > 300:
                break
            self.defence_agents = deepcopy(self.state.defence_agents)
            self.attack_agents = deepcopy(self.state.attack_agents)
            self.round_id += 1


def example():
    scene = [
        {"id": 0, "x": 0, "y": 0, "is_attack": False},
        {"id": 1, "x": 15, "y": 5, "is_attack": True},
        {"id": 2, "x": 6, "y": 6, "is_attack": True},
        # {"id": 3, "x": 7, "y": 7, "is_attack": True},
        # {"id": 4, "x": 8, "y": 8, "is_attack": True},
    ]
    map_info = MapInfo(map_str_to_json("../../server/map_r2m1.txt"), calculate_path=True)
    defence_agents_loc = dict()
    attack_agents_loc = dict()
    for s in scene:
        if s["is_attack"]:
            attack_agents_loc[s["id"]] = map_info.maps[s["x"]][s["y"]]
        else:
            defence_agents_loc[s["id"]] = map_info.maps[s["x"]][s["y"]]
    defence_agents = DefenceAgents(defence_agents_loc)
    attack_agents = AttackAgents(attack_agents_loc)
    state = State(defence_agents, attack_agents, map_info)
    game = Game(state)
    game.run()


if __name__ == '__main__':
    example()
