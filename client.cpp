#include <iostream>
#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 1. Khởi tạo Socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // 2. Kết nối tới Server (Localhost, Cổng 8080)
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Khong the ket noi toi Server!\n";
        WSACleanup();
        return 1;
    }
    
    std::cout << "Ket noi thanh cong! Nhap chuoi de gui den server.\n";

    char buffer[1024];
    std::string input;

    // 3. Vòng lặp Gửi/Nhận
    while (true) {
        std::cout << "-> Ban: ";
        std::getline(std::cin, input);

        if (input.empty()) continue;

        // Gửi chuỗi
        send(clientSocket, input.c_str(), input.length(), 0);

        // Chờ nhận phản hồi
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0) {
            std::string response(buffer);
            std::cout << "<- Server: " << response << "\n\n";

            // Nếu Server trả về chữ "good bye", client cũng tự động thoát
            if (response.find("good bye") != std::string::npos) {
                break;
            }
        } else {
            std::cout << "Server da dong ket noi.\n";
            break;
        }
    }

    // 4. Dọn dẹp
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}