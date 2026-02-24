#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <map>

class ClientConfig {
public:
    static ClientConfig& GetInstance() {
        static ClientConfig instance;
        return instance;
    }

    ClientConfig(const ClientConfig&) = delete;
    ClientConfig& operator=(const ClientConfig&) = delete;

    // デフォルトの設定
    std::string ip_address = "";
    int port_number = 50000;
    int buffer_size = 1024;
    int select_timeout_us = 1000;
	int max_message = 10;

    // 外部ファイルから読み込む関数
    bool LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Configファイルが見つからなかったため、デフォルトの設定を使用します。" << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            // コメント行や空行をスキップする処理があってもいいかも
            if (line.empty() || line[0] == '#' || line[0] == '/') continue;

            auto delimiterPos = line.find('=');
            if (delimiterPos == std::string::npos) continue;

            std::string name = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);

            // 余分な空白を消す処理などはお好みで！
            try {
                if (name == "IP_ADDRESS") ip_address = value;
                else if (name == "PORT_NUMBER") port_number = std::stoi(value);
                else if (name == "BUFFER_SIZE") buffer_size = std::stoi(value);
                else if (name == "SELECT_TIMEOUT_US") select_timeout_us = std::stoi(value);
                else if (name == "MAX_MESSAGE")max_message = std::stoi(value);
            }
            catch (...) {
                std::cerr << "設定値の変換に失敗しました: " << name << std::endl;
            }
        }
        std::cout << filename << "の読み込みに成功。" << std::endl;
        return true;
    }

private:
    ClientConfig() = default;
};