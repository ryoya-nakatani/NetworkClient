//-------------------------------------------------------------
// MessageManager.h
// 説明：メッセージ関連のデータを管理するクラス
//-------------------------------------------------------------

#pragma once
#include <vector>
#include <string>
#include <fstream>
#include "NetworkPacket.h"

class MessageManager {
public:
    // シングルトンにしておくとどこからでもアクセスできて便利
    static MessageManager& GetInstance() {
        static MessageManager instance;
        return instance;
    }

    void AddMessage(MessageData msg);
    void AddMessage(std::vector<MessageData> msg);
    void ReplaceAllMessages(const std::vector<MessageData>& newList);
    bool ExistsAt(float x, float y, float z) const;

private:
    MessageManager() = default;
};