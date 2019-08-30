from enum import Enum
from typing import Dict, Union, List
import json
import numpy as np
from numba import jit


class DIRECTION(Enum):
    UP = 0
    DOWN = 1
    LEFT = 2
    RIGHT = 3
    NONE = 4


class Point:
    def __init__(self, _x: int, _y: int):
        self.x = _x  # type: int
        self.y = _y  # type: int
        self.next = dict()  # type: Dict[DIRECTION, Union[None, Point]]
        self.wall = False  # type: bool
        self.tunnel = DIRECTION.NONE  # type: DIRECTION
        self.wormhole = None  # type: Union[None, Point]
        self.name = ""

    def __repr__(self):
        ret = dict()
        ret["x"] = self.x
        ret["y"] = self.y
        return json.dumps(ret)


class MapInfo:
    def __init__(self, _map_json, calculate_path=False):
        self.map_json = _map_json
        self.width = _map_json["width"]  # type: int
        self.height = _map_json["height"]  # type: int
        self.vision = _map_json["vision"]  # type: int
        self.maps = dict()  # type: Dict[int, Dict[int, Point]]
        for x in range(self.width):
            if x not in self.maps:
                self.maps[x] = dict()
            for y in range(self.height):
                self.maps[x][y] = Point(x, y)
        self.load_map()
        self.construct_map()
        if calculate_path:
            self.path = Path(self.maps, self.width, self.height)

    def set_meteor(self, x: int, y: int):
        self.maps[x][y].wall = True

    def set_tunnel(self, x: int, y: int, direction: str):
        if direction == "up":
            self.maps[x][y].tunnel = DIRECTION.UP
        elif direction == "down":
            self.maps[x][y].tunnel = DIRECTION.DOWN
        elif direction == "left":
            self.maps[x][y].tunnel = DIRECTION.LEFT
        elif direction == "right":
            self.maps[x][y].tunnel = DIRECTION.RIGHT
        else:
            self.maps[x][y].tunnel = DIRECTION.NONE

    def set_wormhole(self, x: int, y: int, next_x: int, next_y: int, name: str):
        self.maps[x][y].wormhole = self.maps[next_x][next_y]
        self.maps[x][y].name = name

    def move_one_step(self, p: Point, direction: DIRECTION):
        next_x = p.x
        next_y = p.y
        if direction == DIRECTION.UP:
            next_y -= 1
        elif direction == DIRECTION.DOWN:
            next_y += 1
        elif direction == DIRECTION.LEFT:
            next_x -= 1
        elif direction == DIRECTION.RIGHT:
            next_x += 1
        if 0 <= next_x < self.width and 0 <= next_y < self.height and not self.maps[next_x][next_y].wall:
            return self.maps[next_x][next_y]
        else:
            return p

    def move_none(self, p: Point):
        if DIRECTION.NONE in p.next:
            return
        if p.tunnel == DIRECTION.NONE:
            p.next[DIRECTION.NONE] = p
            return
        list_p = [p]
        next_p = self.move_one_step(p, p.tunnel)
        while next_p.tunnel != DIRECTION.NONE and DIRECTION.NONE not in next_p.next:
            list_p.append(next_p)
            next_p = self.move_one_step(next_p, next_p.tunnel)
        if DIRECTION.NONE in next_p.next:
            next_p = next_p.next[DIRECTION.NONE]
        for pp in list_p:
            pp.next[DIRECTION.NONE] = next_p

    def load_map(self):
        if "meteor" in self.map_json:
            for meteor in self.map_json["meteor"]:
                self.set_meteor(meteor["x"], meteor["y"])
        if "tunnel" in self.map_json:
            for tunnel in self.map_json["tunnel"]:
                self.set_tunnel(tunnel["x"], tunnel["y"], tunnel["direction"])
        if "wormhole" in self.map_json:
            wormholes = dict()  # type: Dict[str, List[int]]
            for wormhole in self.map_json["wormhole"]:
                name = wormhole["name"].lower()
                if name not in wormholes:
                    wormholes[name] = list()
                wormholes[name].append(wormhole["x"])
                wormholes[name].append(wormhole["y"])
            for name in wormholes:
                w = wormholes[name]
                assert (len(w) == 4)
                self.set_wormhole(w[0], w[1], w[2], w[3], name)
                self.set_wormhole(w[2], w[3], w[0], w[1], name)

    def construct_map(self):
        for x in self.maps:
            for y in self.maps[x]:
                self.move_none(self.maps[x][y])
        for x in self.maps:
            for y in self.maps[x]:
                for d in [DIRECTION.UP, DIRECTION.DOWN, DIRECTION.LEFT, DIRECTION.RIGHT]:
                    p = self.maps[x][y]
                    next_p = self.move_one_step(p, d)
                    p.next[d] = next_p.next[DIRECTION.NONE]
                    if p != next_p and next_p.wormhole:
                        p.next[d] = next_p.wormhole


class Path:
    def __init__(self, _maps: Dict[int, Dict[int, Point]], _width: int, _height: int):
        self.maps = _maps  # type: Dict[int, Dict[int, Point]]
        self.width = _width  # type: int
        self.height = _height  # type: int
        self.node_num = self.width * self.height  # type: int
        self.inf = 9999
        self.dist = [[self.inf for _ in range(self.node_num)] for __ in range(self.node_num)]  # type: List[List[int]]
        self.path = [[0 for _ in range(self.node_num)] for __ in range(self.node_num)]  # type: List[List[int]]
        for i in range(self.node_num):
            for j in range(self.node_num):
                self.path[i][j] = j
        for i in range(self.node_num):
            for d in self.to_point(i).next:
                j = self.to_index(self.to_point(i).next[d])
                if i != j:
                    self.dist[i][j] = 1
                else:
                    self.dist[i][j] = 0
        self.dist = floyd(self.node_num, np.array(self.dist), np.array(self.path)).tolist()

    def get_cost(self, start: Point, end: Point):
        return self.dist[self.to_index(start)][self.to_index(end)]

    def get_cost_index(self, start: int, end: int):
        return self.dist[start][end]

    def to_point(self, index: int) -> Point:
        return self.maps[index % self.width][index // self.width]

    def to_index(self, point: Point) -> int:
        return point.x + point.y * self.width

    def floyd(self):
        for k in range(self.node_num):
            for i in range(self.node_num):
                for j in range(self.node_num):
                    if self.dist[i][k] + self.dist[k][j] < self.dist[i][j] and self.dist[i][k] != self.inf \
                            and self.dist[k][j] != self.inf and i != j:
                        self.dist[i][j] = self.dist[i][k] + self.dist[k][j]
                        self.path[i][j] = self.path[i][k]


@jit(nopython=True)
def floyd(node_num, dist, path, inf=9999):
    for k in range(node_num):
        for i in range(node_num):
            for j in range(node_num):
                if dist[i][k] + dist[k][j] < dist[i][j] and dist[i][k] != inf and dist[k][j] != inf and i != j:
                    dist[i][j] = dist[i][k] + dist[k][j]
                    path[i][j] = path[i][k]
    return dist


def map_str_to_json(file):
    map_json = dict()
    map_txt_json = json.loads(open(file, 'r').read().replace("\n", "").replace("\t", "").replace(" ", ""))
    map_json["vision"] = map_txt_json["game"]["vision"]
    width = map_txt_json["map"]["width"]
    height = map_txt_json["map"]["height"]
    map_json["width"] = width
    map_json["height"] = height
    meteor_list = list()
    tunnel_list = list()
    wormhole_list = list()
    map_str = map_txt_json["map"]["map_str"].strip()
    assert len(map_str) == width * height
    for i, s in enumerate(map_str):
        x = i % width
        y = i // width
        p = dict()
        p["x"] = x
        p["y"] = y
        if s == "." or s == "X" or s == "O" or s.isdigit():
            continue
        if s == '#':
            meteor_list.append(p)
        elif s == "^":
            p["direction"] = "up"
            tunnel_list.append(p)
        elif s == "v":
            p["direction"] = "down"
            tunnel_list.append(p)
        elif s == "<":
            p["direction"] = "left"
            tunnel_list.append(p)
        elif s == ">":
            p["direction"] = "right"
            tunnel_list.append(p)
        else:
            p["name"] = s
            wormhole_list.append(p)
    map_json["meteor"] = meteor_list
    map_json["tunnel"] = tunnel_list
    map_json["wormhole"] = wormhole_list
    return map_json
