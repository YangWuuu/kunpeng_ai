#include "player.h"

#include <algorithm>

#include "log.h"

void Player::message_leg_start(cJSON *msg) {
    parse_message_leg_start(msg);
    leg.construct_map();
}

void Player::message_leg_end(cJSON *msg) {

}

string Player::message_round(cJSON *msg) {
    parse_message_round(msg);

    //show_map();
    for (auto &mu : ri.my_units) {
        blackboard->set("mu", mu.first);
        tree.root_node->executeTick();
//        mu.second->direction = (Direction) (uniform_int_distribution<int>(0, 4)(e));
    }
//    if (ri.my_units.count(leg.my_team.units[0]) != 0){
//        char input;
//        scanf("%c%*c", &input);
//        auto my_first_unit = ri.my_units[leg.my_team.units[0]];
//        switch(input){
//            case 'w':
//                my_first_unit->direction = Direction::UP;
//                break;
//            case 's':
//                my_first_unit->direction = Direction::DOWN;
//                break;
//            case 'a':
//                my_first_unit->direction = Direction::LEFT;
//                break;
//            case 'd':
//                my_first_unit->direction = Direction::RIGHT;
//                break;
//            default:
//                my_first_unit->direction = Direction::NONE;
//                break;
//        }
//    }

    return pack_msg();
}
