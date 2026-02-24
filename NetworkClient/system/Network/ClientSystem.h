#pragma once
#define WIN32_LEAN_AND_MEAN
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/timeb.h>
#include <map>
#include "NetworkPacket.h"
#include "TCPManager.h"
#include "UDPHandler.h"

// ws2_32.lib をリンクする
#pragma comment(lib, "ws2_32.lib")

class ClientSystem
{
public:
    // シングルトン取得
    static ClientSystem& GetInstance() {
        static ClientSystem instance;
        return instance;
    }

    // クライアント処理
    bool Initialize();
    void Cleanup();

    auto& GetTCPManager() { return tcp_manager; }
	auto& GetUDPHandler() { return udp_handler; }

private:
    ClientSystem() = default;
    ClientSystem(const ClientSystem&) = delete;
    ClientSystem& operator=(const ClientSystem&) = delete;

    // サーバー情報
    WSADATA wsaData = {};
    SOCKET client_socket = INVALID_SOCKET;
    sockaddr_in server_addr = {};

    std::unique_ptr<TCPManager> tcp_manager;
    std::unique_ptr<UDPHandler> udp_handler;
};