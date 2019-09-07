#include "make_decision.h"

#include "player.h"
#include "log.h"

constexpr const char *whitespaces = "     ";
constexpr const size_t ws_count = 5;

BT::NodeStatus MakeDecision::tick() {
    auto info = config().blackboard->get<Player *>("info");

    int score_num = info->task_score->score_num;
    vector<double> all_good_score = vector<double>(score_num, 0.0);
    
    for (auto &gc : info->task_score->score) {
        for (int i = 0; i < score_num; i++) {
            all_good_score[i] += gc.second[i];
        }
    }
    double max_good_score = *max_element(all_good_score.begin(), all_good_score.end());
    vector<int> good_score_id;
    for (int i = 0; i < score_num; i++) {
        if (equal_double(all_good_score[i], max_good_score))
            good_score_id.push_back(i);
    }
    int id = -1;
    if (!good_score_id.empty()) {
        id = good_score_id[uniform_int_distribution<int>(0, (int)good_score_id.size() - 1)(e)];
        for (auto& md : info->task_score->map_direction[id]) {
            auto& mu = info->round_info->my_units[md.first];
            if (mu) {
                mu->direction = md.second;
            }
        }
    }

    string out = "\n";
    for (auto &gc : info->task_score->score) {
        out += "round_id: " + to_string(info->game->round_id) + string(" TASK_NAME: ") + TASK_NAME_STRING[gc.first] + "\n";
        vector<int> idx = sort_indexes(gc.second);
        reverse(idx.begin(), idx.end());
        for (int i = 0; i < min({10, (int)idx.size()}); i++) {
            out += to_string(i) + ":" + &whitespaces[std::min(ws_count, to_string(i).size())];
            for (auto &md : info->task_score->map_direction[idx[i]]) {
                auto &mu = info->round_info->my_units[md.first];
                if (mu) {
                    out += to_string(mu->id) + ": " + DIRECTION_STRING[md.second] + "  ";
                }
            }
            out += to_string(gc.second[idx[i]]) + string("\n");
        }
        for (int i = max({(int)idx.size() - 10, 10}); i < (int)idx.size(); i++) {
            out += to_string(i) + ":" + &whitespaces[std::min(ws_count, to_string(i).size())];
            for (auto &md : info->task_score->map_direction[idx[i]]) {
                auto &mu = info->round_info->my_units[md.first];
                if (mu) {
                    out += to_string(mu->id) + ": " + DIRECTION_STRING[md.second] + "  ";
                }
            }
            out += to_string(gc.second[idx[i]]) + string("\n");
        }
    }
    if (id != -1) {
        out += "\nround_id: " + to_string(info->game->round_id) + " DECISION:\n";
        for (auto &gc : info->task_score->score) {
            out += TASK_NAME_STRING[gc.first] + "\t";
            for (auto &md : info->task_score->map_direction[id]) {
                auto &mu = info->round_info->my_units[md.first];
                if (mu) {
                    out += to_string(mu->id) + ": " + DIRECTION_STRING[md.second] + "  ";
                }
            }
            out += to_string(gc.second[id]) + string("\n");
        }
    }
    log_info("%s", out.c_str());

    return BT::NodeStatus::SUCCESS;
}
