#include "player.h"

#include "log.h"

void Player::parse_message_leg_start(cJSON *msg) {
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
    leg_info->GenMap(width, height, vision);
    cJSON *msg_meteors = cJSON_GetObjectItem(msg_maps, "meteor");
    if (msg_meteors != nullptr) {
        for (int i = 0; i < cJSON_GetArraySize(msg_meteors); i++) {
            cJSON *msg_meteor = cJSON_GetArrayItem(msg_meteors, i);
            cJSON *msg_x = cJSON_GetObjectItem(msg_meteor, "x");
            cJSON *msg_y = cJSON_GetObjectItem(msg_meteor, "y");
            int x = msg_x->valueint;
            int y = msg_y->valueint;
            leg_info->maps[x][y]->wall = true;
        }
    }

    cJSON *msg_clouds = cJSON_GetObjectItem(msg_maps, "cloud");
    if (msg_clouds != nullptr) {
        for (int i = 0; i < cJSON_GetArraySize(msg_clouds); i++) {
            cJSON *msg_cloud = cJSON_GetArrayItem(msg_clouds, i);
            cJSON *msg_x = cJSON_GetObjectItem(msg_cloud, "x");
            cJSON *msg_y = cJSON_GetObjectItem(msg_cloud, "y");
            int x = msg_x->valueint;
            int y = msg_y->valueint;
            leg_info->maps[x][y]->cloud = true;
        }
    }

    cJSON *msg_tunnels = cJSON_GetObjectItem(msg_maps, "tunnel");
    if (msg_tunnels != nullptr) {
        for (int i = 0; i < cJSON_GetArraySize(msg_tunnels); i++) {
            cJSON *msg_tunnel = cJSON_GetArrayItem(msg_tunnels, i);
            cJSON *msg_x = cJSON_GetObjectItem(msg_tunnel, "x");
            cJSON *msg_y = cJSON_GetObjectItem(msg_tunnel, "y");
            cJSON *msg_direction = cJSON_GetObjectItem(msg_tunnel, "direction");
            int x = msg_x->valueint;
            int y = msg_y->valueint;
            string tunnel = string(msg_direction->valuestring);
            if (tunnel == "up") {
                leg_info->maps[x][y]->tunnel = DIRECTION::UP;
            } else if (tunnel == "down") {
                leg_info->maps[x][y]->tunnel = DIRECTION::DOWN;
            } else if (tunnel == "left") {
                leg_info->maps[x][y]->tunnel = DIRECTION::LEFT;
            } else if (tunnel == "right") {
                leg_info->maps[x][y]->tunnel = DIRECTION::RIGHT;
            } else {
                log_error("direction is wrong %s", tunnel.c_str());
            }
        }
    }

    cJSON *msg_wormholes = cJSON_GetObjectItem(msg_maps, "wormhole");
    if (msg_wormholes != nullptr) {
        map<string, vector<Point::Ptr>> wormholes;
        for (int i = 0; i < cJSON_GetArraySize(msg_wormholes); i++) {
            cJSON *msg_wormhole = cJSON_GetArrayItem(msg_wormholes, i);
            cJSON *msg_name = cJSON_GetObjectItem(msg_wormhole, "name");
            cJSON *msg_x = cJSON_GetObjectItem(msg_wormhole, "x");
            cJSON *msg_y = cJSON_GetObjectItem(msg_wormhole, "y");
            string name = msg_name->valuestring;
            int x = msg_x->valueint;
            int y = msg_y->valueint;
            transform(name.begin(), name.end(), name.begin(), ::tolower);
            if (wormholes.count(name) == 0) {
                wormholes[name] = vector<Point::Ptr>();
            }
            leg_info->maps[x][y]->name = name;
            wormholes[name].emplace_back(leg_info->maps[x][y]);
        }
        for (auto &w : wormholes) {
            if (w.second.size() != 2) {
                log_error("wormholes error %s", w.first.c_str());
            } else {
                w.second[0]->wormhole = w.second[1];
                w.second[1]->wormhole = w.second[0];
            }
        }
    }

    cJSON *msg_teams = cJSON_GetObjectItem(msg_data, "teams");
    if (msg_teams == nullptr) {
        log_error("msg_teams is nullptr");
    }
    if (cJSON_GetArraySize(msg_teams) != 2) {
        log_error("teams length is not 2");
    }
    leg_info->my_team.units.clear();
    leg_info->enemy_team.units.clear();
    for (int i = 0; i < 2; i++) {
        cJSON *msg_team = cJSON_GetArrayItem(msg_teams, i);
        cJSON *msg_force = cJSON_GetObjectItem(msg_team, "force");
        cJSON *msg_id = cJSON_GetObjectItem(msg_team, "id");
        cJSON *msg_units = cJSON_GetObjectItem(msg_team, "players");
        if (msg_id->valueint == team_id) {
            leg_info->my_team.id = msg_id->valueint;
            leg_info->my_team.force = msg_force->valuestring;
            for (int j = 0; j < cJSON_GetArraySize(msg_units); j++) {
                leg_info->my_team.units.push_back(cJSON_GetArrayItem(msg_units, j)->valueint);
            }
        } else {
            leg_info->enemy_team.id = msg_id->valueint;
            leg_info->enemy_team.force = msg_force->valuestring;
            for (int j = 0; j < cJSON_GetArraySize(msg_units); j++) {
                leg_info->enemy_team.units.push_back(cJSON_GetArrayItem(msg_units, j)->valueint);
            }
        }
    }
}

void Player::parse_message_round(cJSON *msg) {
    cJSON *msg_data = cJSON_GetObjectItem(msg, "msg_data");
    if (msg_data == nullptr) {
        log_error("msg_data is nullptr");
    }
    cJSON *msg_mode = cJSON_GetObjectItem(msg_data, "mode");
    if (msg_data == nullptr) {
        log_error("msg_data is nullptr");
    }
    round_info->mode = msg_mode->valuestring;
    cJSON *msg_round_id = cJSON_GetObjectItem(msg_data, "round_id");
    round_info->round_id = msg_round_id->valueint;
    cJSON *msg_players = cJSON_GetObjectItem(msg_data, "players");
    round_info->my_units.clear();
    round_info->enemy_units.clear();
    if (msg_players != nullptr) {
        for (int i = 0; i < cJSON_GetArraySize(msg_players); i++) {
            cJSON *msg_player = cJSON_GetArrayItem(msg_players, i);
            int id = cJSON_GetObjectItem(msg_player, "id")->valueint;
            int score = cJSON_GetObjectItem(msg_player, "score")->valueint;
            int sleep = cJSON_GetObjectItem(msg_player, "sleep")->valueint;
            int team = cJSON_GetObjectItem(msg_player, "team")->valueint;
            int x = cJSON_GetObjectItem(msg_player, "x")->valueint;
            int y = cJSON_GetObjectItem(msg_player, "y")->valueint;
            Unit::Ptr tmp = Unit::gen(id, score, sleep, team, leg_info->maps[x][y]);
            if (team == team_id) {
                round_info->my_units[id] = move(tmp);
            } else {
                round_info->enemy_units[id] = move(tmp);
            }
        }
    }
    cJSON *msg_powers = cJSON_GetObjectItem(msg_data, "power");
    round_info->powers.clear();
    if (msg_powers != nullptr) {
        for (int i = 0; i < cJSON_GetArraySize(msg_powers); i++) {
            cJSON *msg_power = cJSON_GetArrayItem(msg_powers, i);
            int point = cJSON_GetObjectItem(msg_power, "point")->valueint;
            int x = cJSON_GetObjectItem(msg_power, "x")->valueint;
            int y = cJSON_GetObjectItem(msg_power, "y")->valueint;
            round_info->powers.emplace_back(Power(point, leg_info->maps[x][y]));
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
            round_info->my_point = msg_point->valueint;
            round_info->my_remain_life = msg_remain_life->valueint;
        } else {
            round_info->enemy_point = msg_point->valueint;
            round_info->enemy_remain_life = msg_remain_life->valueint;
        }
    }
}


string Player::pack_msg() {
    cJSON *root = cJSON_CreateObject();
    cJSON *msg_data = cJSON_CreateObject();
    cJSON *actions = cJSON_CreateArray();

    cJSON_AddItemToObject(root, "msg_name", cJSON_CreateString("action"));
    cJSON_AddItemToObject(root, "msg_data", msg_data);
    cJSON_AddNumberToObject(msg_data, "round_id", round_info->round_id);
    cJSON_AddItemToObject(msg_data, "actions", actions);

    for (auto &u: round_info->my_units) {
        if (u.second == nullptr)
            continue;
        cJSON *subAct = cJSON_CreateObject();
        cJSON *move = cJSON_CreateArray();
        cJSON_AddItemToArray(actions, subAct);
        cJSON_AddNumberToObject(subAct, "team", team_id);
        cJSON_AddNumberToObject(subAct, "player_id", u.first);
        cJSON_AddItemToObject(subAct, "move", move);
        switch (u.second->direction) {
            case DIRECTION::UP:
                cJSON_AddItemToArray(move, cJSON_CreateString("up"));
                break;
            case DIRECTION::DOWN:
                cJSON_AddItemToArray(move, cJSON_CreateString("down"));
                break;
            case DIRECTION::LEFT:
                cJSON_AddItemToArray(move, cJSON_CreateString("left"));
                break;
            case DIRECTION::RIGHT:
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

void Player::show_map() {
//    0: black
//    1: red
//    2: green
//    3: yellow
//    4: blue
//    5: purple
//    6: dark green
//    7: white
//    4: back color  3: front color
    bool debug = false;
    if (debug) {
        printf("\033[47m\033[2J"); // clear console
    }
    auto format = [](const string &fmt, const string &s) -> string {
        char tmp[1024];
        sprintf(tmp, fmt.c_str(), s.c_str());
        return string(tmp);
    };
    string fmt_map = "\x1b[47;30m %s\x1b[0m";
    string fmt_power = "\x1b[45;30m$%s\x1b[0m";
    string fmt_enemy = "\x1b[41;37me%s\x1b[0m";
    string fmt_my = "\x1b[42;37mm%s\x1b[0m";
    string fmt_wall = "\x1b[40;30m  \x1b[0m";
    string fmt_tunnel = "\x1b[46;35m %s\x1b[0m";
    string fmt_wormhole = "\x1b[44;35m %s\x1b[0m";
    printf("\x1b[47;30m\n\nround_id: %d  mode: %s\n\n\x1b[0m", round_info->round_id, round_info->mode.c_str());
    auto print_team_info = [&](const Team &team, const string &fmt, int point, int remain_life) {
        printf("\x1b[%sid: %d  point: %d  remain_life: %d force: %s\x1b[0m", fmt.c_str(), team.id, point, remain_life,
               team.force.c_str());
        printf("\x1b[0;0m\n\x1b[0m");
        auto map_units = &round_info->my_units;
        if (team.id != team_id)
            map_units = &round_info->enemy_units;
        for (auto &mu : *map_units) {
            auto &u = mu.second;
            printf("\x1b[%s    %d  score: %d  sleep: %d  x: %2d y:%2d\x1b[0m", fmt.c_str(), u->id, u->score, u->sleep,
                   u->loc->x, u->loc->y);
            printf("\x1b[0;0m\n\x1b[0m");
        }
    };
    print_team_info(leg_info->my_team, "42;37m", round_info->my_point, round_info->my_remain_life);
    print_team_info(leg_info->enemy_team, "41;37m", round_info->enemy_point, round_info->enemy_remain_life);

    vector<vector<string>> map_string(leg_info->height, vector<string>(leg_info->width, format(fmt_map, ".")));
    for (auto &power : round_info->powers) {
        map_string[power.loc->x][power.loc->y] = format(fmt_power, to_string(power.point));
    }
    for (int h = 0; h < leg_info->height; h++) {
        for (int w = 0; w < leg_info->width; w++) {
            if (leg_info->maps[w][h]->wall) {
                map_string[w][h] = fmt_wall;
            } else if (leg_info->maps[w][h]->tunnel == DIRECTION::UP) {
                map_string[w][h] = format(fmt_tunnel, "^");
            } else if (leg_info->maps[w][h]->tunnel == DIRECTION::DOWN) {
                map_string[w][h] = format(fmt_tunnel, "v");
            } else if (leg_info->maps[w][h]->tunnel == DIRECTION::LEFT) {
                map_string[w][h] = format(fmt_tunnel, "<");
            } else if (leg_info->maps[w][h]->tunnel == DIRECTION::RIGHT) {
                map_string[w][h] = format(fmt_tunnel, ">");
            } else if (leg_info->maps[w][h]->wormhole) {
                map_string[w][h] = format(fmt_wormhole, leg_info->maps[w][h]->name);
            }
        }
    }
    for (auto &u : round_info->enemy_units) {
        map_string[u.second->loc->x][u.second->loc->y] = format(fmt_enemy, to_string(u.first));
    }
    for (auto &u : round_info->my_units) {
        map_string[u.second->loc->x][u.second->loc->y] = format(fmt_my, to_string(u.first));
    }
    if (debug) {
        for (int h = 0; h < leg_info->height; h++) {
            for (int w = 0; w < leg_info->width; w++) {
                printf("%s", map_string[w][h].c_str());
            }
            printf("\x1b[0;0m\n\x1b[0m");
        }
    }
}
