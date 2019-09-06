#include <cstdio>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>
#include <thread>

#include "player.h"
#include "log.h"
#include "OSSocket.h"
#include "behaviortree_cpp/behavior_tree.h"


int main(int argc, char *argv[]) {
    if (argc < 4) {
        log_error("Usage: %s [player_id] [serverIp] [serverPort]\n", argv[0]);
        return -1;
    }

    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d_%02d_%02d_%02d_%02d_%02d",
            (int) ptm->tm_year + 1900, (int) ptm->tm_mon + 1, (int) ptm->tm_mday,
            (int) ptm->tm_hour, (int) ptm->tm_min, (int) ptm->tm_sec);

    string log_path = "/var/log/battle_yangwuuu_" + string(date) + ".log";
#ifdef OS_WINDOWS
    // windows init
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        log_error("WSAStartup failed\n");
        return false;
    }
    log_path = "./battle_yangwuuu.log";
#endif

    bool debug = argc > 4 && string(argv[4]) == "debug";
    FILE *fp = fopen(log_path.c_str(), "w");
    log_set_fp(fp);
    if (!debug) {
        log_set_quiet(true);
    }

    string arg_string;
    for (int i = 0; i < argc; i++) {
        arg_string += string(argv[i]) + " ";
    }
    log_info("argv is %s", arg_string.c_str());

    OS_SOCKET hSocket;

    while (OSCreateSocket(argv[2], (unsigned) std::stoul(argv[3]), &hSocket) != 0) {
        log_error("wait 10ms to restart connect");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    log_info("connect server success\n");

    int myTeamId = std::stoi(argv[1]);
    Player player(myTeamId);
    /* Ïòserver×¢²á */
    char regMsg[200] = {'\0'};
    sprintf(regMsg, R"({"msg_name":"registration","msg_data":{"team_id":%d,"team_name":"enemy"}})", myTeamId);
    char regMsgWithLength[200] = {'\0'};
    sprintf(regMsgWithLength, "%05d%s", (int) strlen(regMsg), regMsg);
    send(hSocket, regMsgWithLength, (int) strlen(regMsgWithLength), 0);
    log_info("register my info to server success");

    vector<int> time_vec_0;
    vector<int> time_vec_1;
    int time_round = 0;
    int time_all = 0;
    int avg_time_round_count = 0;
    int avg_time_leg_start_count = 0;
    double avg_time_round = 0.0;
    double avg_time_leg_start = 0.0;
    vector<string> leg_end_msgs;
    int recv_err_count = 0;
    auto char_to_int = [](const char *buffer, int len) -> int {
        int ret = 0;
        for (int i = 0; i < len; i++) {
            int d = *(buffer + i) - '0';
            if (d >= 0 && d <= 9) {
                ret = ret * 10 + d;
            } else {
                return 0;
            }
        }
        return ret;
    };
    string full_msg;
    int full_msg_len = 0;
    while (true) {
        if (recv_err_count > 20) {
            log_error("recv_err_count is too large");
            break;
        }
        char buffer[99999] = {'\0'};
        auto start_time_all = chrono::system_clock::now();
        if (recv(hSocket, buffer, sizeof(buffer) - 1, 0)) {
            auto start_time_round = chrono::system_clock::now();
            int len = char_to_int(buffer, 5);
            if (len > 0) {
                full_msg = string(buffer);
                full_msg_len = len;
            } else {
                full_msg += string(buffer);
            }
            log_info("full_msg_len: %d full_msg.size: %d %s", full_msg_len, full_msg.size(), buffer);
            if ((int) full_msg.size() < full_msg_len + 5) {
                log_error("bag is splited");
                recv_err_count++;
                continue;
            }
            cJSON *msgBuf = cJSON_Parse(full_msg.c_str() + 5);
            full_msg_len = 0;
            full_msg.clear();

            if (nullptr == msgBuf) {
                recv_err_count++;
                continue;
            }
            cJSON *msgNamePtr = cJSON_GetObjectItem(msgBuf, "msg_name");
            if (nullptr == msgNamePtr) {
                recv_err_count++;
                continue;
            }
            recv_err_count = 0;
            char *msgName = msgNamePtr->valuestring;
            if (0 == strcmp(msgName, "round")) {
                string ret = player.message_round(msgBuf);
                send(hSocket, ret.c_str(), (int) ret.size(), 0);
                log_info("round_id: %d  SendActMsg: %s", player.round_info->round_id, ret.c_str());
                auto end_time = chrono::system_clock::now();
                auto duration_round = chrono::duration_cast<chrono::milliseconds>(end_time - start_time_round).count();
                avg_time_round += (duration_round - avg_time_round) / (avg_time_round_count + 1);
                avg_time_round_count++;
            } else if (0 == strcmp(msgName, "leg_start")) {
                player.message_leg_start(msgBuf);
                log_info("leg_start");
                auto end_time = chrono::system_clock::now();
                auto duration_round = chrono::duration_cast<chrono::milliseconds>(end_time - start_time_round).count();
                avg_time_leg_start += (duration_round - avg_time_leg_start) / (avg_time_leg_start_count + 1);
                avg_time_leg_start_count++;
            } else if (0 == strcmp(msgName, "leg_end")) {
                leg_end_msgs.emplace_back(player.message_leg_end(msgBuf));
                log_info("leg_end");
            } else if (0 == strcmp(msgName, "game_over")) {
                log_info("game_over");
                break;
            } else {
                recv_err_count++;
                log_error("others");
            }
            auto end_time = chrono::system_clock::now();
            auto duration_all = chrono::duration_cast<chrono::milliseconds>(end_time - start_time_all);
            auto duration_round = chrono::duration_cast<chrono::milliseconds>(end_time - start_time_round);
            time_vec_0.push_back((int) duration_all.count());
            time_vec_1.push_back((int) duration_round.count());
            time_round += (int) duration_round.count();
            time_all += (int) duration_all.count();
            log_info("round time cost: %ld/%ld ms\n\n", duration_round.count(), duration_all.count());
        }
    }
    log_info("max single time is %d/%d ms round/all: %d/%dms\n", *max_element(time_vec_1.begin(), time_vec_1.end()),
             *max_element(time_vec_0.begin(), time_vec_0.end()), time_round, time_all);
    log_info("leg_start: %dcount  %fms", avg_time_leg_start_count, avg_time_leg_start);
    log_info("round: %dcount  %fms", avg_time_round_count, avg_time_round);
    for (string &s : leg_end_msgs) {
        log_info(s.c_str());
    }
    OSCloseSocket(hSocket);
    if (fp) {
        fclose(fp);
    }
    return 0;
}
