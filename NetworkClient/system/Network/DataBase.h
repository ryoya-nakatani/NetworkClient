#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <map>
#include <string>
#include "NetworkPacket.h"

class DataBase
{
public:
    static DataBase& GetInstance() {
        static DataBase instance;
        return instance;
    }

    void SetClientListChanged(bool changed) { m_clientListChanged = changed; }
    bool GetClientListChanged() const { return m_clientListChanged; }
    void SetMessages(std::vector<MessageData> msgs) { m_messages = msgs; }
    auto& GetMessages() { return m_messages; }
    void SetClients(std::vector<ClientInfo> clientList) { m_clientList = clientList; m_clientListChanged = true; }
    auto& GetClients() { return m_clientList; }
	void SetPlayersInfo(std::map<uint32_t, PlayerInfo> playersInfo) { m_playersInfo = playersInfo; }
    auto& GetPlayersInfo() { return m_playersInfo; }

    // インディアン変換関数
    inline float float_swap_to_network(float f) {
        uint32_t temp;
        memcpy(&temp, &f, sizeof(float));
        uint32_t netTemp = htonl(temp); // ホスト順 -> ネットワーク順
        float result;
        memcpy(&result, &netTemp, sizeof(float));
        return result;
    }

private:
    DataBase() = default;

    // クライアント数の変動フラグ
    bool m_clientListChanged = false;

    // DBに保存する情報
    std::vector<MessageData> m_messages;
    std::vector<ClientInfo> m_clientList;
    std::map<uint32_t, PlayerInfo> m_playersInfo;
};

