#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <map>
#include "NetworkPacket.h"

class UDPHandler {
public:
    UDPHandler() = default;
    ~UDPHandler();

    bool Initialize();

    // 自分の座標をサーバーへ送信
    void SendPlayerInfo(uint32_t my_id, float x, float y, float z);
	// キー入力状態をサーバーへ送信
    void SendInput(uint32_t my_id, bool w, bool a, bool s, bool d, bool up, bool down,bool left,bool right);

    // UDPの受信チェックとデータ更新
    void CheckUdpEvents();

private:
    SOCKET udp_socket = INVALID_SOCKET;
    sockaddr_in server_addr = {};

    // 汎用のデータ抽出
    void ProcessPacket(const char* buf, int size);
};