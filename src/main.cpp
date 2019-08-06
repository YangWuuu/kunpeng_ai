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
    bool debug = argc > 4 && string(argv[4]) == "debug";
    FILE *fp = fopen("/var/log/battle.log", "w");
    log_set_fp(fp);
    if (!debug) {
        log_set_quiet(true);
    }

#ifdef OS_WINDOWS
    // windows init
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        log_error("WSAStartup failed\n");
        return false;
    }
#endif

    OS_SOCKET hSocket;

    while (OSCreateSocket(argv[2], (unsigned) std::stoul(argv[3]), &hSocket) != 0) {
        log_error("wait 10ms to restart connect");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    log_info("connect server success\n");

    int myTeamId = std::stoi(argv[1]);
    Player player(myTeamId, "ai_yang", debug);
    /* Ïòserver×¢²á */
    char regMsg[200] = {'\0'};
    sprintf(regMsg, R"({"msg_name":"registration","msg_data":{"team_id":%d,"team_name":"ai_yang"}})", myTeamId);
    char regMsgWithLength[200] = {'\0'};
    sprintf(regMsgWithLength, "%05d%s", (int) strlen(regMsg), regMsg);
    OSSend(hSocket, regMsgWithLength, (int) strlen(regMsgWithLength));
    log_info("register my info to server success");

    vector<int> time_vec;
    while (true) {
        char buffer[99999] = {'\0'};
        if (recv(hSocket, buffer, sizeof(buffer) - 1, 0)) {
            auto start_time = chrono::system_clock::now();
            cJSON *msgBuf = cJSON_Parse(buffer + 5);
            log_info("%s", buffer);

            if (nullptr == msgBuf)
                continue;
            cJSON *msgNamePtr = cJSON_GetObjectItem(msgBuf, "msg_name");
            if (nullptr == msgNamePtr)
                continue;
            char *msgName = msgNamePtr->valuestring;
            if (0 == strcmp(msgName, "round")) {
                string ret = player.message_round(msgBuf);
                send(hSocket, ret.c_str(), ret.size(), 0);
                log_info("round_id: %d  SendActMsg: %s", player.ri.round_id, ret.c_str());
            } else if (0 == strcmp(msgName, "leg_start")) {
                player.message_leg_start(msgBuf);
                log_info("leg_start");
            } else if (0 == strcmp(msgName, "leg_end")) {
                player.message_leg_end(msgBuf);
                log_info("leg_end");
            } else if (0 == strcmp(msgName, "game_over")) {
                log_info("game_over");
                break;
            } else {
                log_error("others");
            }
            auto end_time = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
            time_vec.push_back((int) duration.count());
            log_info("time cost: %ld ms\n\n", duration.count());
        }
    }
    log_info("max single time is %d", *max_element(time_vec.begin(), time_vec.end()));
    OSCloseSocket(hSocket);
    if (fp) {
        fclose(fp);
    }
    return 0;
}
