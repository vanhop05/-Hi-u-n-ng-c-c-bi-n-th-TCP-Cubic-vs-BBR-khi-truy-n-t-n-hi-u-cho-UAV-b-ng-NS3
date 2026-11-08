#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // 1. Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Khoi tao Winsock that bai." << std::endl;
        return 1;
    }

    // 2. Tạo TCP Socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Tao socket that bai." << std::endl;
        WSACleanup();
        return 1;
    }

    // 3. Cấu hình địa chỉ Server để kết nối
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 4. Kết nối tới Server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Khong the ket noi den Server! Hay dam bao TCPserver.exe dang chay." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "=== DA KET NOI THANH CONG TOI SERVER ===" << std::endl;

    // 5. Vòng lặp nhập chuỗi liên tục từ bàn phím
    std::string inputStr;
    while (true) {
        std::cout << "\nNhap chuoi ky tu  ";
        std::getline(std::cin, inputStr);

        // Nếu người dùng bấm Enter trống hoặc gõ "exit" thì thoát
        if (inputStr.empty() || inputStr == "exit") {
            std::cout << "Dang dong ket noi..." << std::endl;
            break;
        }

        // Gửi chuỗi tới Server
        send(clientSocket, inputStr.c_str(), inputStr.length(), 0);

        // Chờ và nhận kết quả từ Server
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Ket qua tu Server: " << buffer << std::endl;
        } else {
            std::cout << "Mat ket noi voi Server." << std::endl;
            break;
        }
    }

    // 6. Đóng kết nối và dọn dẹp
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}