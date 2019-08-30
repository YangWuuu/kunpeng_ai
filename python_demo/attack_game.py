import time
import json
import random
from abc import abstractmethod
from copy import deepcopy
import functools
import itertools
from typing import List, Dict, Set
from map_info import MapInfo, Point, DIRECTION, map_str_to_json

random.seed(2019)


def get_input_action():
    ss = input("Please input action: ").lower()
    ret = DIRECTION.NONE
    if ss == "w":
        ret = DIRECTION.UP
    elif ss == "s":
        ret = DIRECTION.DOWN
    elif ss == "a":
        ret = DIRECTION.LEFT
    elif ss == "d":
        ret = DIRECTION.RIGHT
    return ret


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
            ["".join([map_str[i][j] for i in range(self.map_info.width)]) for j in range(self.map_info.height)]) + \
               "\n{}\n{}".format(self.defence_agents.id_loc, self.attack_agents.id_loc)

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
            # ret[idx] = get_input_action()
        return ret


def get_all_can_go_points(_state: State, attack_point_int: int, defence_point_int: int) -> Set[int]:
    ret = set()
    if attack_point_int != defence_point_int:
        for i in range(_state.map_info.path.node_num):
            p = _state.map_info.path.to_point(i)
            if p.tunnel != DIRECTION.NONE or p.wall:
                continue
            if _state.map_info.path.get_cost_index(attack_point_int, i) == 0:
                continue
            if _state.map_info.path.get_cost_index(attack_point_int, i) > \
                    _state.map_info.path.get_cost_index(defence_point_int, i):
                ret.add(i)
    return ret


def get_all_can_go(_state: State, attack_locs: List[Point], defence_loc: Point):
    tmp_list = list()
    defence_loc_int = _state.map_info.path.to_index(defence_loc)
    for attack_loc in attack_locs:
        tmp_list.append(get_all_can_go_points(_state, _state.map_info.path.to_index(attack_loc), defence_loc_int))
    return tmp_list


class AttackAgents(Agents):
    def get_actions(self, _state: State):
        ret = dict()
        # for idx in self.id_loc:
        #     choices = list()
        #     min_cost_loc = None
        #     min_cost = 9999
        #     for defence_idx in _state.defence_agents.id_loc:
        #         cost = _state.map_info.path.get_cost(self.id_loc[idx], _state.defence_agents.id_loc[defence_idx])
        #         if cost < min_cost:
        #             min_cost = cost
        #             min_cost_loc = _state.defence_agents.id_loc[defence_idx]
        #     for direction in [DIRECTION.UP, DIRECTION.DOWN, DIRECTION.LEFT, DIRECTION.RIGHT, DIRECTION.NONE]:
        #         if _state.map_info.path.get_cost(self.id_loc[idx].next[direction], min_cost_loc) \
        #                 < _state.map_info.path.get_cost(self.id_loc[idx], min_cost_loc):
        #             choices.append(direction)
        #     ret[idx] = random.choice(choices)
        directions = [DIRECTION.UP, DIRECTION.DOWN, DIRECTION.LEFT, DIRECTION.RIGHT, DIRECTION.NONE]
        results = list()
        for actions in itertools.product(directions, repeat=len(self.id_loc)):
            for defence_idx in sorted(_state.defence_agents.id_loc.keys()):
                i = 0
                attack_locs = list()
                for idx in sorted(self.id_loc.keys()):
                    attack_locs.append(self.id_loc[idx].next[actions[i]])
                    i += 1
                tmp_results = list()
                for defence_action in directions:
                    defence_loc = _state.defence_agents.id_loc[defence_idx].next[defence_action]
                    survival = 1
                    for idx in sorted(self.id_loc.keys()):
                        if defence_loc == self.id_loc[idx]:
                            survival = 0
                            break
                    dis = [_state.map_info.path.get_cost(_, defence_loc) for _ in attack_locs]
                    points = get_all_can_go(_state, attack_locs, defence_loc)
                    intersection_points = functools.reduce(lambda x, y: x.intersection(y), points)
                    tmp_results.append({"intersection_num": len(intersection_points), "dis_sum": sum(dis), "survival": survival,
                                        "intersect_points": intersection_points, "points": points, "actions": actions,
                                        "defence": defence_idx, "dis": dis, "defence_action": defence_action})
                tmp_results = sorted(tmp_results, key=lambda x: (-x["survival"], -x["intersection_num"], -x["dis_sum"]))
                results.append(tmp_results[0])
        results = sorted(results, key=lambda x: (x["survival"], x["intersection_num"]))
        print(results[0]["intersection_num"], results[0]["dis_sum"], results[0]["defence_action"])
        i = 0
        for idx in sorted(self.id_loc.keys()):
            ret[idx] = results[0]["actions"][i]
            i += 1
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
            attack_actions = self.attack_agents.get_actions(self.state)
            print("attack actions: {}".format(attack_actions))
            defence_actions = self.defence_agents.get_actions(self.state)
            print("defence actions: {}".format(defence_actions))
            self.state.apply_actions(defence_actions)
            self.state.apply_actions(attack_actions)
            if self.state.game_over or self.round_id > 300:
                break
            self.defence_agents = deepcopy(self.state.defence_agents)
            self.attack_agents = deepcopy(self.state.attack_agents)
            self.round_id += 1


def example():
    scene = [
        {"id": 0, "x": 1, "y": 7, "is_attack": False},
        {"id": 8, "x": 1, "y": 7, "is_attack": False},
        {"id": 7, "x": 1, "y": 7, "is_attack": False},
        {"id": 9, "x": 10, "y": 0, "is_attack": False},
        {"id": 1, "x": 15, "y": 5, "is_attack": True},
        {"id": 2, "x": 6, "y": 6, "is_attack": True},
        {"id": 3, "x": 7, "y": 7, "is_attack": True},
        {"id": 4, "x": 7, "y": 7, "is_attack": True},
        # {"id": 4, "x": 8, "y": 8, "is_attack": True},
    ]
    map_info = MapInfo(map_str_to_json("../../server/map_r2m6.txt"), calculate_path=True)
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
