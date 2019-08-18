import unittest
import json
from map_info import MapInfo, DIRECTION, map_str_to_json


class TestFunc(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.map_info = MapInfo(map_str_to_json("../../server/map_r2m1.txt"))

    def move_one_step(self, x, y, direction: DIRECTION):
        next_p = self.map_info.maps[x][y].next[direction]
        return next_p.x, next_p.y

    def test_pos(self):
        print("test pos")
        cases = [
            [0, 0, DIRECTION.RIGHT, 1, 0],
            [0, 19, DIRECTION.RIGHT, 1, 19],
            [0, 19, DIRECTION.UP, 0, 19],
            [1, 19, DIRECTION.LEFT, 13, 6],
            [5, 4, DIRECTION.DOWN, 5, 4],
            [6, 4, DIRECTION.DOWN, 15, 5],
            [9, 4, DIRECTION.DOWN, 15, 5],
            [10, 4, DIRECTION.DOWN, 15, 5],
            [6, 12, DIRECTION.DOWN, 19, 0],
            [4, 14, DIRECTION.RIGHT, 4, 14],
            [4, 13, DIRECTION.RIGHT, 5, 4],
            [6, 4, DIRECTION.RIGHT, 6, 4],
        ]
        for case in cases:
            self.assertTupleEqual((case[3], case[4]), self.move_one_step(case[0], case[1], case[2]))


if __name__ == '__main__':
    print(unittest.main())

