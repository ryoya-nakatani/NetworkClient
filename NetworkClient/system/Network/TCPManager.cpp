#include "TCPManager.h"
#include "DataBase.h"
#include "../../Config/ClientConfig.h"

// エンディアン変換のヘルパー関数群
auto clientSwap = [](const ClientInfo& client) {
	ClientInfo networkOrder;
	networkOrder.id = htonl(client.id);
	networkOrder.IPaddress = client.IPaddress;
	networkOrder.tcp_port = htonl(client.tcp_port);
	networkOrder.udp_port = htonl(client.udp_port);
	return networkOrder;
	};
auto messageSwap = [](const MessageData& message) {
	MessageData networkOrder;
	networkOrder.ownerId = htonl(message.ownerId);
	networkOrder.x = DataBase::GetInstance().float_swap_to_network(message.x);
	networkOrder.y = DataBase::GetInstance().float_swap_to_network(message.y);
	networkOrder.z = DataBase::GetInstance().float_swap_to_network(message.z);
	networkOrder.rotY = DataBase::GetInstance().float_swap_to_network(message.rotY);
	return networkOrder;
	};

// TCPの接続
bool TCPManager::Connect() {
	tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcp_socket == INVALID_SOCKET) return false;

	sockaddr_in tcp_addr = {};
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(ClientConfig::GetInstance().port_number);
	inet_pton(AF_INET, ClientConfig::GetInstance().ip_address.c_str(), &tcp_addr.sin_addr);

	if (connect(tcp_socket, (sockaddr*)&tcp_addr, sizeof(tcp_addr)) == SOCKET_ERROR) {
		std::cerr << "TCP接続 に 失敗しました。" << std::endl;
		closesocket(tcp_socket);
		return false;
	}

	// ノンブロッキング
	u_long mode = 1;
	ioctlsocket(tcp_socket, FIONBIO, &mode);

	return true;
}

// メッセージを設置
bool TCPManager::SendSetMessage(float x, float y, float z, float rotY) {
	if (tcp_socket == INVALID_SOCKET) return false;

	//	メッセージが既にあるか確認
	if (MessageManager::GetInstance().ExistsAt(x, y, z)) {
		std::cout << "[Client] その場所には既にメッセージがあります。" << std::endl;
		return false;
	}

	// パケットを組み立てる
	SetMessagePacket packet;
	packet.header.type = EventType::SetMessage;
	packet.header.senderId = my_id;						// Connect()で貰った自分のID
	packet.x = x;
	packet.y = y;
	packet.z = z;
	packet.rotY = rotY;

	int result = send(tcp_socket, reinterpret_cast<const char*>(&packet), sizeof(packet), 0);

	if (result == SOCKET_ERROR) {
		std::cerr << "[TCP] データの送信 に 失敗しました。 Error: " << WSAGetLastError() << std::endl;
		return false;
	}
	else {
		// 送信量を記録
		NetworkStats::GetInstance().AddTraffic(result, 0);
		std::string detail = "Pos(" + std::to_string(x) + "," + std::to_string(z) + ")";
		NetworkStats::GetInstance().AddPacketLog(EventType::SetMessage, detail);
	}

	std::cout << "[TCP] データの送信に成功しました。 EventType: SetMessage" << std::endl;
	return true;
}

// TCPの受信処理（update内で毎フレーム呼ぶ）
void TCPManager::CheckTcpEvents() {
	if (tcp_socket == INVALID_SOCKET) return;

	// ヘッダーだけを読み込むためのバッファ
	PacketHeader header;

	// データがある限りループする（1フレームに複数パケット来る可能性があるため）
	while (true) {
		// ヘッダーのみPeekで読む
		int peekResult = recv(tcp_socket, (char*)&header, sizeof(PacketHeader), MSG_PEEK);

		if (peekResult <= 0) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				// エラーならログ出し
			}
			break;
		}

		// データサイズがヘッダー未満なら、まだ処理できないので次フレームへ
		if (peekResult < sizeof(PacketHeader)) break;

		// パケットタイプによって処理を分ける
		switch (header.type) {
		case EventType::SyncBatch:
		{
			std::vector<MessageData> messageList;
			if (DeserializeAndExtraction(EventType::SyncBatch, messageList, messageSwap)) {
				MessageManager::GetInstance().AddMessage(messageList);
			}
			break;
		}

		case EventType::SyncClientInfo:
		{
			std::vector<ClientInfo> clientList;
			if (DeserializeAndExtraction(EventType::SyncClientInfo, clientList, clientSwap)) {
				DataBase::GetInstance().SetClients(clientList);
			}
			break;
		}
		case EventType::Welcome:
		{
			// IDをサーバーから受け取る
			WelcomePacket welcomePkt;
			int bytes = recv(tcp_socket, (char*)&welcomePkt, sizeof(WelcomePacket), 0);
			if (bytes > 0) {
				my_id = welcomePkt.header.senderId;
				std::cout << "TCP接続 完了。 ID: " << my_id << std::endl;
			}

			// ネットワーク統計にIDをセット
			NetworkStats::GetInstance().SetMyID(my_id);
			NetworkStats::GetInstance().SetConnectState(Connecting);
			NetworkStats::GetInstance().SetServerInfo(welcomePkt.serverPort, ClientConfig::GetInstance().ip_address);
			NetworkStats::GetInstance().AddPacketLog(EventType::Welcome, "Assigned ID: " + std::to_string(my_id));
		
			break;
		}
		// それ以外のパケット
		default:
		{
			// メッセージボックスを出す（エラー）
			MessageBox(NULL, "Error Message Type", "", MB_OK);
			break;
		}
		}
	}
}