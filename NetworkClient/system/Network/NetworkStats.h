#pragma once
#include <string>
#include <map>
#include <vector>
#include <deque>
#include "NetworkPacket.h"
#include "MessageManager.h"

enum ConnectState{
	Connecting,
	Disconnection,
};

struct PacketLogEntry {
	std::string type;   // "SyncBatch" とか
	std::string detail; // 中身の詳細テキスト
	std::string timeStr;         // 記録した時間
};

class NetworkStats
{
public:
	static NetworkStats& GetInstance() {
		static NetworkStats instance;
		return instance;
	}
	// コピーコンストラクタと代入演算子を削除
	NetworkStats(const NetworkStats&) = delete;
	NetworkStats& operator=(const NetworkStats&) = delete;

	// 毎フレーム呼ぶ（グラフの更新用）
	void Update(float deltaTime);

	// パケット受信・送信時に呼ぶ
	void AddTraffic(uint16_t sent, uint16_t recv);
	void AddPacketLog(EventType type, const std::string& detail);

	// 状態セット系
	void SetMyID(size_t id) { myID = id; }
	void SetConnectState(ConnectState s) { state = s; }
	void SetServerInfo(const size_t & port, const std::string& value) { serverInfo[port] = value; }

	ConnectState GetConnectState() const { return state; }

	void DrawDebugUI();
	void DrawMessageListUI();

private:
	NetworkStats() {
		// グラフ配列の初期化（これやっとかないとエラーになるよ！）
		tcpSendHistory.resize(60, 0.0f);
		tcpRecvHistory.resize(60, 0.0f);
	}

	// 基礎ステータス
	ConnectState state = Disconnection;	// 接続状況
	std::map<size_t, std::string> serverInfo;	// サーバーの情報（ポート番号、IPアドレス）
	size_t myID = -1;	// 自分のID

	// 通信量計測用
	uint16_t currentFrameSent = 0; // そのフレーム(または計測期間)の送信量
	uint16_t currentFrameRecv = 0;
	std::vector<float> tcpSendHistory;
	std::vector<float> tcpRecvHistory;
	float graphTimer = 0.0f; // 更新タイミング管理

	// パケットログ (dequeで履歴管理)
	std::deque<PacketLogEntry> packetLogs;
	const size_t MAX_LOG_LINES = 100; // ログの保存最大数
};