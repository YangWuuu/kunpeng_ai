#include "player.h"

void Player::message_leg_start(cJSON *msg) {
    if (!leg_info) {
        leg_info = make_shared<LegStartInfo>();
    } else {
        prev_leg_info = leg_info;
    }
    if (!round_info) {
        round_info = make_shared<RoundInfo>();
    }
    parse_message_leg_start(msg);
    leg_info->construct_map();
    game = make_shared<Game>(leg_info);
    task_score = make_shared<TaskScore>();
}

string Player::message_leg_end(cJSON *msg) {
    cJSON *msg_data = cJSON_GetObjectItem(msg, "msg_data");
    cJSON *msg_teams = cJSON_GetObjectItem(msg_data, "teams");
    return cJSON_PrintUnformatted(msg_teams);
}

string Player::message_round(cJSON *msg) {
    parse_message_round(msg);
    //show_map();
    game->update_round_info(round_info);
    task_score->init_every_round(round_info->my_units);
    tree.root_node->executeTick();
    return pack_msg();
}
