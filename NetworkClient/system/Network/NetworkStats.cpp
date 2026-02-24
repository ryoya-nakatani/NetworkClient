#include "NetworkStats.h"
#include "DataBase.h"
#include <Windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>

// ヘルパー関数: 現在時刻を "HH:MM:SS.ms" 形式の文字列にする
std::string GetCurrentTimeStamp() {
	// 現在時刻を取得
	auto now = std::chrono::system_clock::now();
	auto now_time_t = std::chrono::system_clock::to_time_t(now);

	// ミリ秒部分を計算
	auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
		now.time_since_epoch()) % 1000;

	// ローカル時間に変換 (Windowsでは localtime_s が安全)
	struct tm tm_info;
	localtime_s(&tm_info, &now_time_t);

	// 文字列フォーマット作成
	std::stringstream ss;
	ss << std::put_time(&tm_info, "%H:%M:%S") // 時:分:秒
		<< "." << std::setfill('0') << std::setw(3) << now_ms.count(); // .ミリ秒

	return ss.str();
}

void NetworkStats::Update(float deltaTime) {
	// 0.1秒ごとにグラフを更新
	graphTimer += deltaTime;
	if (graphTimer >= 0.1f) {
		// 履歴の末尾に追加
		tcpSendHistory.push_back((float)currentFrameSent);
		tcpRecvHistory.push_back((float)currentFrameRecv);

		// 古いデータを削除してサイズを一定に保つ
		if (tcpSendHistory.size() > 60) tcpSendHistory.erase(tcpSendHistory.begin());
		if (tcpRecvHistory.size() > 60) tcpRecvHistory.erase(tcpRecvHistory.begin());

		// カウンタをリセット
		currentFrameSent = 0;
		currentFrameRecv = 0;
		graphTimer = 0.0f;
	}
}

void NetworkStats::AddTraffic(uint16_t sent, uint16_t recv) {
	currentFrameSent += sent;
	currentFrameRecv += recv;
}

void NetworkStats::AddPacketLog(EventType type, const std::string& detail) {
	PacketLogEntry entry;
	// Enumから文字列への変換（switchとかで分岐させてもOK）
	switch (type) {
	case EventType::Welcome: entry.type = "Welcome"; break;
	case EventType::SetMessage: entry.type = "SetMessage"; break;
	case EventType::SyncBatch: entry.type = "SyncBatch"; break;
	case EventType::SyncClientInfo: entry.type = "SyncClientInfo"; break;
	default: entry.type = "Unknown"; break;
	}
	entry.detail = detail;
	entry.timeStr = GetCurrentTimeStamp();

	packetLogs.push_back(entry);
	if (packetLogs.size() > MAX_LOG_LINES) {
		packetLogs.pop_front();
	}
}

// ImGuiのインクルードを忘れずに！
#include "../imgui/imgui.h" 

void NetworkStats::DrawDebugUI() {
	ImGui::Begin("Network Monitor"); // ウィンドウ開始

	// ---------------------------------------------------------
	// 1. サーバー情報 & 自分ステータス
	// ---------------------------------------------------------
	if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 接続状態を色付きで表示
		if (state == Connecting) {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "State: Connected");
		}
		else {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "State: Disconnected");
		}

		ImGui::Text("My ID: %d", (int)myID);

		ImGui::Separator();
		ImGui::Text("--- Server Info ---");
		for (auto const& [key, val] : serverInfo) {
			ImGui::Text("%d: %s", key, val.c_str());
		}
	}

	// ---------------------------------------------------------
	// 2. クライアントリスト
	// ---------------------------------------------------------
	if (ImGui::CollapsingHeader("Clients List", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Active Clients: %d", DataBase::GetInstance().GetClients().size());

		if (ImGui::BeginTable("clients_table", 2, ImGuiTableFlags_Borders)) {
			ImGui::TableSetupColumn("ID");
			ImGui::TableSetupColumn("IP Address");
			ImGui::TableHeadersRow();

			for (const auto& client : DataBase::GetInstance().GetClients()) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", (int)client.id);
				ImGui::TableSetColumnIndex(1);
				char ipStr[INET_ADDRSTRLEN] = { 0 };
				if (inet_ntop(AF_INET, &client.IPaddress, ipStr, sizeof(ipStr))) {
					ImGui::TextUnformatted(ipStr);
				}
				else {
					ImGui::Text("---.---.---.---");
				}
			}
			ImGui::EndTable();
		}
	}

	// ---------------------------------------------------------
	// 3. 通信量グラフ (TCP)
	// ---------------------------------------------------------
	if (ImGui::CollapsingHeader("Traffic (TCP)", ImGuiTreeNodeFlags_DefaultOpen)) {
		// 送信 (緑)
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Send");
		float maxSend = *std::max_element(tcpSendHistory.begin(), tcpSendHistory.end());
		ImGui::PlotLines("##Send", tcpSendHistory.data(), (int)tcpSendHistory.size(),
			0, NULL, 0.0f, maxSend + 100.0f, ImVec2(0, 50));

		// 受信 (黄色)
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Recv");
		float maxRecv = *std::max_element(tcpRecvHistory.begin(), tcpRecvHistory.end());
		ImGui::PlotLines("##Recv", tcpRecvHistory.data(), (int)tcpRecvHistory.size(),
			0, NULL, 0.0f, maxRecv + 100.0f, ImVec2(0, 50));
	}

	// ---------------------------------------------------------
	// 4. パケットログ
	// ---------------------------------------------------------
	if (ImGui::CollapsingHeader("Packet Logs", ImGuiTreeNodeFlags_DefaultOpen)) {
		// ログ用のスクロール領域を作成
		ImGui::BeginChild("LogScroll", ImVec2(0, 150), true);

		for (const auto& log : packetLogs) {
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1), "[%s]", log.timeStr.c_str());
			ImGui::SameLine();

			// タイプによって色を変えるとおしゃれ！
			ImVec4 typeColor = ImVec4(1, 1, 1, 1);
			if (log.type == "SyncBatch") typeColor = ImVec4(0.4f, 1.0f, 1.0f, 1); // シアン
			else if (log.type == "SetMessage") typeColor = ImVec4(1.0f, 0.6f, 0.0f, 1); // オレンジ
			else if(log.type == "SyncClientInfo") typeColor = ImVec4(0.7f, 1.0f, 0.2f, 1.0f); // 黄緑

			ImGui::TextColored(typeColor, "%s", log.type.c_str());
			ImGui::SameLine();
			ImGui::Text("- %s", log.detail.c_str());
		}

		// オートスクロール（常に一番下を表示）
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::EndChild();
	}

	ImGui::End();
}

void NetworkStats::DrawMessageListUI() {
	ImGui::Begin("MessageList"); // ウィンドウ開始

// ---------------------------------------------------------
// 5. メッセージリスト (現在保持している全データ)
// ---------------------------------------------------------
	if (ImGui::CollapsingHeader("Message List (Snapshot)", ImGuiTreeNodeFlags_DefaultOpen)) {

		// メッセージリストを取得
		const auto& messages = DataBase::GetInstance().GetMessages();
		ImGui::Text("Total Messages: %d", (int)messages.size());

		// 縦に長くなりすぎないようにスクロール領域を作る (高さ200px)
		ImGui::BeginChild("MsgListScroll", ImVec2(0, 400), true);

		// 4列のテーブルを作成 (OwnerID, X, Y, Z)
		// ImGuiTableFlags_RowBg: 縞模様にして見やすく
		// ImGuiTableFlags_ScrollY: ヘッダー固定でスクロール可能に
		if (ImGui::BeginTable("MsgTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

			// --- ヘッダー設定 ---
			ImGui::TableSetupColumn("Owner ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
			ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("rotY", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupScrollFreeze(0, 1); // ヘッダーを常に上部に固定
			ImGui::TableHeadersRow();

			// --- データ表示ループ ---
			for (const auto& msg : messages) {
				ImGui::TableNextRow();

				// ★自分のIDのデータなら「緑色」にする演出
				bool isMine = (msg.ownerId == this->myID);
				if (isMine) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
				}

				// カラムごとに値をセット
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", msg.ownerId);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.2f", msg.x);

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%.2f", msg.y);

				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%.2f", msg.z);

				ImGui::TableSetColumnIndex(4);
				ImGui::Text("%.2f", msg.rotY);

				// 色を戻す
				if (isMine) {
					ImGui::PopStyleColor();
				}
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}

	//ImGui::Separator(); // 区切り線

	//// --- 【追加】プレイヤーのリアルタイム座標情報 (UDPデータ) ---
	//if (ImGui::CollapsingHeader("Players Realtime Info (UDP)", ImGuiTreeNodeFlags_DefaultOpen)) {
	//	const auto& playersInfo = DataBase::GetInstance().GetPlayersInfo();
	//	ImGui::Text("Active Data in Map: %zu", playersInfo.size());

	//	ImGui::BeginChild("PlayerInfoScroll", ImVec2(0, 200), true);
	//	if (ImGui::BeginTable("PlayerTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY)) {

	//		ImGui::TableSetupColumn("Client ID", ImGuiTableColumnFlags_WidthFixed, 70.0f);
	//		ImGui::TableSetupColumn("Pos X", ImGuiTableColumnFlags_WidthStretch);
	//		ImGui::TableSetupColumn("Pos Y", ImGuiTableColumnFlags_WidthStretch);
	//		ImGui::TableSetupColumn("Pos Z", ImGuiTableColumnFlags_WidthStretch);
	//		ImGui::TableSetupScrollFreeze(0, 1);
	//		ImGui::TableHeadersRow();

	//		// map のイテレータで回すよ
	//		for (auto const& [id, info] : playersInfo) {
	//			ImGui::TableNextRow();

	//			// 自分のデータなら色を変える
	//			bool isMine = (id == this->myID);
	//			if (isMine) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f)); // 自分はオレンジっぽく

	//			ImGui::TableSetColumnIndex(0);
	//			ImGui::Text("%d", id);

	//			ImGui::TableSetColumnIndex(1);
	//			ImGui::Text("%.2f", info.x);

	//			ImGui::TableSetColumnIndex(2);
	//			ImGui::Text("%.2f", info.y);

	//			ImGui::TableSetColumnIndex(3);
	//			ImGui::Text("%.2f", info.z);

	//			if (isMine) ImGui::PopStyleColor();
	//		}
	//		ImGui::EndTable();
	//	}
	//	ImGui::EndChild();
	//}

	ImGui::End();
}