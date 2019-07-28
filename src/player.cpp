#include "player.h"

#include "log.h"

void Player::message_leg_start(cJSON *msg) {
    cJSON *msg_data = cJSON_GetObjectItem(msg, "msg_data");
    if (msg_data == nullptr) {
        log_error("msg_data is nullptr");
    }
    cJSON *msg_maps = cJSON_GetObjectItem(msg_data, "map");
    if (msg_maps == nullptr) {
        log_error("msg_maps is nullptr");
    }
    cJSON *msg_width = cJSON_GetObjectItem(msg_maps, "width");
    int width = msg_width->valueint;
    cJSON *msg_height = cJSON_GetObjectItem(msg_maps, "height");
    int height = msg_height->valueint;
    cJSON *msg_vision = cJSON_GetObjectItem(msg_maps, "vision");
    int vision = msg_vision->valueint;
    leg.GenMap(width, height, vision);
    cJSON *msg_meteors = cJSON_GetObjectItem(msg_maps, "meteor");
    if (msg_meteors != nullptr){
        for (int i = 0; i < cJSON_GetArraySize(msg_meteors); i++) {
            cJSON *msg_meteor = cJSON_GetArrayItem(msg_meteors, i);
            cJSON *msg_x = cJSON_GetObjectItem(msg_meteor, "x");
            cJSON *msg_y = cJSON_GetObjectItem(msg_meteor, "y");
            int x = msg_x->valueint;
            int y = msg_y->valueint;
            leg.maps[x][y]->meteor = true;
        }
    }

    cJSON *msg_tunnels = cJSON_GetObjectItem(msg_maps, "tunnel");
    if (msg_tunnels != nullptr){
        for (int i = 0; i < cJSON_GetArraySize(msg_tunnels); i++) {
            cJSON *msg_tunnel = cJSON_GetArrayItem(msg_tunnels, i);
            cJSON *msg_x = cJSON_GetObjectItem(msg_tunnel, "x");
            cJSON *msg_y = cJSON_GetObjectItem(msg_tunnel, "y");
            cJSON *msg_direction = cJSON_GetObjectItem(msg_tunnel, "direction");
            int x = msg_x->valueint;
            int y = msg_y->valueint;
            string direction = string(msg_direction->valuestring);
            if (direction == "up") {
                leg.maps[x][y]->direction = Direction::UP;
            } else if (direction == "down") {
                leg.maps[x][y]->direction = Direction::DOWN;
            } else if (direction == "left") {
                leg.maps[x][y]->direction = Direction::LEFT;
            } else if (direction == "right") {
                leg.maps[x][y]->direction = Direction::RIGHT;
            } else {
                log_error("direction is wrong %s", direction.c_str());
            }
        }
    }

    //TODO wormhole

    cJSON *msg_teams = cJSON_GetObjectItem(msg_data, "teams");
    if (msg_teams == nullptr) {
        log_error("msg_teams is nullptr");
    }
    if (cJSON_GetArraySize(msg_teams) != 2) {
        log_error("teams length is not 2");
    }
    leg.my_team.units.clear();
    leg.enemy_team.units.clear();
    for (int i = 0; i < 2; i++) {
        cJSON *msg_team = cJSON_GetArrayItem(msg_teams, i);
        cJSON *msg_force = cJSON_GetObjectItem(msg_team, "force");
        cJSON *msg_id = cJSON_GetObjectItem(msg_team, "id");
        cJSON *msg_units = cJSON_GetObjectItem(msg_team, "players");
        if (msg_id->valueint == team_id) {
            leg.my_team.id = msg_id->valueint;
            leg.my_team.force = msg_force->valuestring;
            for (int j = 0; j < cJSON_GetArraySize(msg_units); j++) {
                leg.my_team.units.push_back(cJSON_GetArrayItem(msg_units, j)->valueint);
            }
        } else {
            leg.enemy_team.id = msg_id->valueint;
            leg.enemy_team.force = msg_force->valuestring;
            for (int j = 0; j < cJSON_GetArraySize(msg_units); j++) {
                leg.enemy_team.units.push_back(cJSON_GetArrayItem(msg_units, j)->valueint);
            }
        }
    }

}

void Player::message_leg_end(cJSON *msg) {

}

string Player::message_round(cJSON *msg) {
    cJSON *msg_data = cJSON_GetObjectItem(msg, "msg_data");
    if (msg_data == nullptr) {
        log_error("msg_data is nullptr");
    }
    cJSON *msg_mode = cJSON_GetObjectItem(msg_data, "mode");
    if (msg_data == nullptr) {
        log_error("msg_data is nullptr");
    }
    ri.mode = msg_mode->valuestring;
    cJSON *msg_round_id = cJSON_GetObjectItem(msg_data, "round_id");
    ri.round_id = msg_round_id->valueint;
    cJSON *msg_players = cJSON_GetObjectItem(msg_data, "players");
    ri.my_units.clear();
    ri.enemy_units.clear();
    if (msg_players != nullptr){
        for (int i = 0; i < cJSON_GetArraySize(msg_players); i++) {
            cJSON *msg_player = cJSON_GetArrayItem(msg_players, i);
            int id = cJSON_GetObjectItem(msg_player, "id")->valueint;
            int score = cJSON_GetObjectItem(msg_player, "score")->valueint;
            int sleep = cJSON_GetObjectItem(msg_player, "sleep")->valueint;
            int team = cJSON_GetObjectItem(msg_player, "team")->valueint;
            int x = cJSON_GetObjectItem(msg_player, "x")->valueint;
            int y = cJSON_GetObjectItem(msg_player, "y")->valueint;
            Unit::Ptr tmp = Unit::gen(id, score, sleep, team, leg.maps[x][y]);
            if (team == team_id) {
                ri.my_units[id] = move(tmp);
            } else {
                ri.enemy_units[id] = move(tmp);
            }
        }
    }
    cJSON *msg_powers = cJSON_GetObjectItem(msg_data, "power");
    ri.powers.clear();
    if (msg_powers != nullptr) {
        for (int i = 0; i < cJSON_GetArraySize(msg_powers); i++) {
            cJSON *msg_power = cJSON_GetArrayItem(msg_powers, i);
            int point = cJSON_GetObjectItem(msg_power, "point")->valueint;
            int x = cJSON_GetObjectItem(msg_power, "x")->valueint;
            int y = cJSON_GetObjectItem(msg_power, "y")->valueint;
            ri.powers.emplace_back(Power(point, leg.maps[x][y]));
        }
    }
    cJSON *msg_teams = cJSON_GetObjectItem(msg_data, "teams");
    if (cJSON_GetArraySize(msg_teams) != 2) {
        log_error("teams length is not 2");
    }
    for (int i = 0; i < 2; i++) {
        cJSON *msg_team = cJSON_GetArrayItem(msg_teams, i);
        cJSON *msg_id = cJSON_GetObjectItem(msg_team, "id");
        cJSON *msg_point = cJSON_GetObjectItem(msg_team, "point");
        cJSON *msg_remain_life = cJSON_GetObjectItem(msg_team, "remain_life");
        if (msg_id->valueint == team_id) {
            ri.my_point = msg_point->valueint;
            ri.my_remain_life = msg_remain_life->valueint;
        } else {
            ri.enemy_point = msg_point->valueint;
            ri.enemy_remain_life = msg_remain_life->valueint;
        }
    }

    for (auto &mu : ri.my_units) {
        blackboard->set("mu", mu.first);
        tree.root_node->executeTick();
//        mu.second->direction = (Direction) (uniform_int_distribution<int>(0, 4)(e));
    }

    return pack_msg();
}

string Player::pack_msg() {
    cJSON *root = cJSON_CreateObject();
    cJSON *msg_data = cJSON_CreateObject();
    cJSON *actions = cJSON_CreateArray();

    cJSON_AddItemToObject(root, "msg_name", cJSON_CreateString("action"));
    cJSON_AddItemToObject(root, "msg_data", msg_data);
    cJSON_AddNumberToObject(msg_data, "round_id", ri.round_id);
    cJSON_AddItemToObject(msg_data, "actions", actions);

    for (auto &u: ri.my_units) {
        cJSON *subAct = cJSON_CreateObject();
        cJSON *move = cJSON_CreateArray();
        cJSON_AddItemToArray(actions, subAct);
        cJSON_AddNumberToObject(subAct, "team", team_id);
        cJSON_AddNumberToObject(subAct, "player_id", u.first);
        cJSON_AddItemToObject(subAct, "move", move);
        switch (u.second->direction) {
            case Direction::UP:
                cJSON_AddItemToArray(move, cJSON_CreateString("up"));
                break;
            case Direction::DOWN:
                cJSON_AddItemToArray(move, cJSON_CreateString("down"));
                break;
            case Direction::LEFT:
                cJSON_AddItemToArray(move, cJSON_CreateString("left"));
                break;
            case Direction::RIGHT:
                cJSON_AddItemToArray(move, cJSON_CreateString("right"));
                break;
            default:
                break;
        }
    }
    char *msg = cJSON_PrintUnformatted(root);

    string s;
    char act_msg[9999];
    sprintf(act_msg, "%05d%s", (int) strlen(msg), msg);
    free(msg);
    return string(act_msg);
}