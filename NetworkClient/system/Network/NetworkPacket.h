#pragma once
#include <stdint.h>
#include <bitset>

enum class EventType : uint8_t {
    // TCP
    Welcome = 0,        // 接続した際のID割り当て 
    SetMessage = 1,    // メッセージ配置
    SyncServerInfo = 2,  // サーバー情報同期
    SyncClientInfo = 3,  // クライアント情報同期
    SyncBatch = 4,      // メッセージデータ送信

    // UDP
    SendPlayerInfo = 10, // プレーヤーの座標等の送信
	SendInput = 11      // プレーヤーの入力情報送信
};

enum class InputKey : uint32_t {
    W, A, S, D,
    UP, DOWN, LEFT, RIGHT,

    MAX // ビット数定義用
};

// キャストのヘルパー
constexpr size_t ToIdx(InputKey key) {
    return static_cast<size_t>(key);
}

// MAX_INPUT_KEY_BITS を定数として定義
constexpr size_t MAX_INPUT_KEY_BITS = static_cast<size_t>(InputKey::MAX);

#pragma pack(push, 1)   // メモリの隙間を埋めてパケットサイズを正確にする

// @brief パケットのヘッダー
// @param type パケットの種類
// @param senderId パケット送信元のクライアントID
struct PacketHeader {
    EventType type;
    uint32_t senderId;
};

// @brief 接続した際のID割り当て用パケット
// @param type パケットの種類（welcom固定）
// @param assignedId パケット送信元のクライアントID（サーバーなので0固定）
// @param serverPort サーバーのポート番号
// @param serverIP サーバーのIPアドレス
struct WelcomePacket {  
    PacketHeader header;
    uint16_t serverPort;
    char serverIP[16];
};

// @brief メッセージの情報を格納する構造体
// @param ownerId メッセージを設置したクライアントのID
// @param x,y,z メッセージの座標
// @param rotY メッセージの向き
struct MessageData {
    uint32_t ownerId;
    float x, y, z, rotY;
};

// @brief クライアント用のメッセージ設置パケット
// @param header パケットのヘッダー
// @param x,y,z メッセージの座標
// @param rotY メッセージの向き
struct SetMessagePacket {
    PacketHeader header;
    float x, y, z, rotY;
};

// @brief クライアントの情報
// @param id サーバーから割り当てられたクライアントID
// @param IPaddress クライアントのIPアドレス
// @param tcp_por, udp_portt クライアントのポート番号
struct ClientInfo {
    uint32_t id;
    uint32_t IPaddress;
    uint16_t tcp_port;
    uint16_t udp_port;
};

// @brief UDPで送るPlayerの座標（バッチ）
// @param x,y,z Playerの座標
struct PlayerInfo {
    std::bitset<MAX_INPUT_KEY_BITS> flags;
    float x, y, z;
};

#pragma pack(pop)