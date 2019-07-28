#include <cstdio>
#include <string>
#include <cstring>
#include <ctime>
#include <chrono>
#include <thread>

#include "player.h"
#include "OSSocket.h"
#include "behaviortree_cpp/behavior_tree.h"


int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s [player_id] [serverIp] [serverPort]\n", argv[0]);
        return -1;
    }

#ifdef OS_WINDOWS
    // windows下，需要进行初始化操作
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed\n");
        return false;
    }
#endif

    OS_SOCKET hSocket;

    while (OSCreateSocket(argv[2], (unsigned) std::stoul(argv[3]), &hSocket) != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    printf("connect server success\n");

    int myTeamId = std::stoi(argv[1]);

    Player player(myTeamId, "ai_yang");
    /* 向server注册 */
    char regMsg[200] = {'\0'};
    sprintf(regMsg, R"({"msg_name":"registration","msg_data":{"team_id":%d,"team_name":"ai_yang"}})", myTeamId);
    char regMsgWithLength[200] = {'\0'};
    sprintf(regMsgWithLength, "%05d%s", (int) strlen(regMsg), regMsg);
    send(hSocket, regMsgWithLength, (int) strlen(regMsgWithLength), 0);
    printf("register my info to server success\n");

    /* 进入游戏 */
    while (true) {
        char buffer[99999] = {'\0'};
        if (recv(hSocket, buffer, sizeof(buffer) - 1, 0)) {
            cJSON *msgBuf = cJSON_Parse(buffer + 5);
            printf("%s\n", buffer + 5);
            if (nullptr == msgBuf)
                continue;
            cJSON *msgNamePtr = cJSON_GetObjectItem(msgBuf, "msg_name");
            if (nullptr == msgNamePtr)
                continue;
            char *msgName = msgNamePtr->valuestring;
            if (0 == strcmp(msgName, "round")) {
                string ret = player.message_round(msgBuf);
                printf("\nSendActMsg:%s\n", ret.c_str());
                send(hSocket, ret.c_str(), ret.size(), 0);
            } else if (0 == strcmp(msgName, "leg_start")) {
                player.message_leg_start(msgBuf);
            } else if (0 == strcmp(msgName, "leg_end")) {
                player.message_leg_end(msgBuf);
            } else if (0 == strcmp(msgName, "game_over")) {
                break;
            }
        }
    }

    OSCloseSocket(hSocket);

    return 0;
}



