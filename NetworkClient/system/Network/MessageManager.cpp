#include "MessageManager.h"
#include "../../Config/ClientConfig.h"
#include "DataBase.h"
#include <iostream>
#include <winsock2.h>

// メッセージの追加
void MessageManager::AddMessage(MessageData msg) {
    if (DataBase::GetInstance().GetMessages().size() >= ClientConfig::GetInstance().max_message) DataBase::GetInstance().GetMessages().erase(DataBase::GetInstance().GetMessages().begin());
    DataBase::GetInstance().GetMessages().push_back(msg);
}
void MessageManager::AddMessage(std::vector<MessageData> msgs) {
    for (const auto& msg : msgs) {
        if (DataBase::GetInstance().GetMessages().size() >= ClientConfig::GetInstance().max_message) DataBase::GetInstance().GetMessages().erase(DataBase::GetInstance().GetMessages().begin());
        DataBase::GetInstance().GetMessages().push_back(msg);
    }
}

void MessageManager::ReplaceAllMessages(const std::vector<MessageData>& newList) {
    DataBase::GetInstance().GetMessages() = newList;

#ifdef _DEBUG
    std::cout << "メッセージの同期が完了しました。: " << DataBase::GetInstance().GetMessages().size() << std::endl;
#endif
}

bool MessageManager::ExistsAt(float x, float y, float z) const {
    // 判定の許容範囲（この距離内なら「同じ場所」とみなす）
    const float THRESHOLD = 5.0f;

    for (const auto& msg : DataBase::GetInstance().GetMessages()) {
        float dx = msg.x - x;
        float dy = msg.y - y;
        float dz = msg.z - z;
        float distSq = dx * dx + dy * dy + dz * dz;

        // 許容範囲内であればセーフ
        if (distSq < (THRESHOLD * THRESHOLD)) {
            return true;
        }
    }
    return false;
}