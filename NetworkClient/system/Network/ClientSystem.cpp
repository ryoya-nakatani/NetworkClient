#include "ClientSystem.h"
#include "MessageManager.h"
#include "../../Config/ClientConfig.h"
#include <stdint.h> 
#include <string> 
#include <fstream>

bool ClientSystem::Initialize() {
	// configファイルのロード
	ClientConfig::GetInstance().LoadFromFile("Config/Config.txt");

	// Winsockの初期化
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup に 失敗しました。." << std::endl;
		return false;
	}

	//// UDP
	//udp_handler = std::make_unique<UDPHandler>();
	//if (!udp_handler->Initialize()) {
	//	WSACleanup();
	//	return false;
	//}
	// TCP
	tcp_manager = std::make_unique<TCPManager>();
	if (!tcp_manager->Connect()) {
		return false;
	}

	return true;
}

void ClientSystem::Cleanup() {
	// Winsockのクリーンアップ
	WSACleanup();
}

