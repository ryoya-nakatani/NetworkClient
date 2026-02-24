#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include "NetworkPacket.h"
#include "NetworkStats.h"
#include "DataBase.h"

class TCPManager
{
public:
    bool Connect(); // TCP接続用
    void CheckTcpEvents();
    bool SendSetMessage(float x, float y, float z, float rotY);

    uint32_t GetMyID() { return my_id; }

    std::vector<MessageData> syncBuffer;    // 同期用バッファ

private:
    SOCKET tcp_socket = INVALID_SOCKET;
    uint32_t my_id = 0;

    // 汎用の復元＋抽出テンプレート関数
    template <typename T, typename Func>
    bool DeserializeAndExtraction(EventType type, std::vector<T>& buffer, Func func)
    {
        // 固定長のヘッダー＋カウント分を先に読む
        int headerAndCountSize = sizeof(PacketHeader) + sizeof(uint32_t);
        std::vector<char> peekBuf(headerAndCountSize);
        int received = recv(tcp_socket, peekBuf.data(), headerAndCountSize, MSG_PEEK);
        if (received < headerAndCountSize) return false;

        // ヘッダーを飛ばして個数を取得し、パケット全体サイズを計算
        char* ptr_size = peekBuf.data() + sizeof(PacketHeader);
        uint32_t dataCount = *reinterpret_cast<uint32_t*>(ptr_size);
        int totalPacketSize = headerAndCountSize + (dataCount * sizeof(T));

        // 全データが届いているか確認
        std::vector<char> fullBuf(totalPacketSize);
        int checkFull = recv(tcp_socket, fullBuf.data(), totalPacketSize, MSG_PEEK);
        if (checkFull < totalPacketSize) return false;

        // 全データを受信
        int receivedBytes = recv(tcp_socket, fullBuf.data(), totalPacketSize, 0);

        // ログ出力
        NetworkStats::GetInstance().AddTraffic(0, receivedBytes);
        std::string detail = "Count: " + std::to_string(dataCount) + " / TotalSize: " + std::to_string(totalPacketSize);
        NetworkStats::GetInstance().AddPacketLog(type, detail);

        const char* ptr = fullBuf.data();

        // ヘッダーをとばす
        ptr += sizeof(PacketHeader);

        // 個数を取り出す
        uint32_t count = 0;
        memcpy(&count, ptr, sizeof(uint32_t));
        ptr += sizeof(uint32_t);

        // データを取り出す
        const T* payloadArray = reinterpret_cast<const T*>(ptr);

        // エンディアン変換を行ってからベクターに入れる
        for (uint32_t i = 0; i < count; ++i) {
            T data = payloadArray[i];
            buffer.push_back(func(data));
        }
        return true;
    };

};

