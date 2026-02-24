#include "UDPHandler.h"
#include "DataBase.h"
#include "../../Config/ClientConfig.h"
#include <iostream>

UDPHandler::~UDPHandler() {
    if (udp_socket != INVALID_SOCKET) {
        closesocket(udp_socket);
    }
}

bool UDPHandler::Initialize() {
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udp_socket == INVALID_SOCKET) {
        std::cerr << "[UDP] Socket Creation Failed." << std::endl;
        return false;
    }

    // サーバーアドレスの設定
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ClientConfig::GetInstance().port_number);
    inet_pton(AF_INET, ClientConfig::GetInstance().ip_address.c_str(), &server_addr.sin_addr);

    // ノンブロッキングモード設定
    u_long mode = 1;
    if (ioctlsocket(udp_socket, FIONBIO, &mode) != 0) {
        std::cerr << "[UDP] Non-blocking Mode Failed." << std::endl;
        closesocket(udp_socket);
        return false;
    }

    std::cout << "[UDP] Initialized Successfully." << std::endl;
    return true;
}

void UDPHandler::SendPlayerInfo(uint32_t my_id, float x, float y, float z) {
    if (udp_socket == INVALID_SOCKET) return;

    std::vector<char> packet(sizeof(PacketHeader)+sizeof(PlayerInfo));
    char* ptr = packet.data();

    // パケット組み立て
	PacketHeader header;
	PlayerInfo info;
    header.type = EventType::SendPlayerInfo;
    header.senderId = my_id;

    // エンディアン変換
    info.x = DataBase::GetInstance().float_swap_to_network(x);
    info.y = DataBase::GetInstance().float_swap_to_network(y);
    info.z = DataBase::GetInstance().float_swap_to_network(z);

    // パケットに詰める
    std::memcpy(ptr, &header, sizeof(PacketHeader));
    ptr += sizeof(PacketHeader);
    std::memcpy(ptr, &info, sizeof(PlayerInfo));
	ptr += sizeof(PlayerInfo);

    // 送信
    sendto(udp_socket, packet.data(), sizeof(PacketHeader) + sizeof(PlayerInfo), 0,
        (sockaddr*)&server_addr, sizeof(server_addr));
}

void UDPHandler::SendInput(uint32_t my_id, bool w, bool a, bool s, bool d, bool up, bool down, bool left, bool right) {
    if (udp_socket == INVALID_SOCKET) return;

    std::vector<char> packet(sizeof(PacketHeader) + sizeof(uint32_t));
    char* ptr = packet.data();

    // パケット組み立て
    PacketHeader header;
    header.type = EventType::SendInput;
    header.senderId = my_id;

    // 入力をビットフラグに変換
    std::bitset<ToIdx(InputKey::MAX)> flags;
    if (w)     flags.set(ToIdx(InputKey::W));
    if (a)     flags.set(ToIdx(InputKey::A));
    if (s)     flags.set(ToIdx(InputKey::S));
    if (d)     flags.set(ToIdx(InputKey::D));
    if (up)    flags.set(ToIdx(InputKey::UP));
    if (down)  flags.set(ToIdx(InputKey::DOWN));
    if (left)  flags.set(ToIdx(InputKey::LEFT));
    if (right) flags.set(ToIdx(InputKey::RIGHT));

    uint32_t rawFlags = static_cast<uint32_t>(flags.to_ulong());
    rawFlags = htonl(rawFlags);

        // パケットに詰める
    std::memcpy(ptr, &header, sizeof(PacketHeader));
    ptr += sizeof(PacketHeader);
    std::memcpy(ptr, &rawFlags, sizeof(uint32_t));

    // 送信
    sendto(udp_socket, packet.data(), sizeof(PacketHeader) + sizeof(uint32_t), 0,
        (sockaddr*)&server_addr, sizeof(server_addr));
}

void UDPHandler::CheckUdpEvents() {
    if (udp_socket == INVALID_SOCKET) return;

    std::vector<char> buf(ClientConfig::GetInstance().buffer_size);
    sockaddr_in from_addr;
    int addr_len = sizeof(from_addr);

    // データがある限り全て読み切る
    while (true) {
        int bytes = recvfrom(udp_socket, buf.data(), (int)buf.size(), 0, (sockaddr*)&from_addr, &addr_len);

        if (bytes > 0) {
            ProcessPacket(buf.data(), bytes);
        }
        else {
            // WSAEWOULDBLOCK（データ空っぽ）なら正常終了
            if (bytes == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {}
            break;
        }
    }
}

void UDPHandler::ProcessPacket(const char* buf, int size) {
    if (size < sizeof(PacketHeader)) return;

    const char* ptr = buf;
    PacketHeader header;
    std::memcpy(&header, ptr, sizeof(PacketHeader));
    ptr += sizeof(PacketHeader);

    switch (header.type)
    {
    case EventType::SendPlayerInfo:
    {
        // パケットの種類に応じた処理
        if (size >= sizeof(PacketHeader) + sizeof(PlayerInfo)) {
			PlayerInfo info;
			std::memcpy(&info, ptr, sizeof(PlayerInfo));

            // データ格納
            PlayerInfo& p = DataBase::GetInstance().GetPlayersInfo()[header.senderId];
            p.x = DataBase::GetInstance().float_swap_to_network(info.x);
            p.y = DataBase::GetInstance().float_swap_to_network(info.y);
            p.z = DataBase::GetInstance().float_swap_to_network(info.z);
        }
        break;
    }
    case EventType::SendInput:
    {
        if (size >= sizeof(PacketHeader) + sizeof(uint32_t)) {
            uint32_t rawFlags;
            std::memcpy(&rawFlags, ptr, sizeof(uint32_t));

            rawFlags = ntohl(rawFlags);
            std::bitset<ToIdx(InputKey::MAX)> flags(rawFlags);

            // データ格納
            DataBase::GetInstance().GetPlayersInfo()[header.senderId].flags = flags;
        }
        break;
	}
    default:
        break;
    }
}