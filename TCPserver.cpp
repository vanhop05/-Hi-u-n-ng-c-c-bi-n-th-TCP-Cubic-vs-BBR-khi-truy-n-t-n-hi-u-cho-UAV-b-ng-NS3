#include <iostream>
#include <string>
#include <cctype>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define BUFFER_SIZE 1024

// Hàm viết hoa chữ cái đầu tiên của mỗi từ trong chuỗi
void capitalizeWords(std::string &str) {
    bool newWord = true;
    for (size_t i = 0; i < str.length(); i++) {
        if (std::isspace(static_cast<unsigned char>(str[i]))) {
            newWord = true;
        } else if (newWord) {
            str[i] = std::toupper(static_cast<unsigned char>(str[i]));
            newWord = false;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];

    // 1. Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Khoi tao Winsock that bai. Ma loi: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // 2. Tạo TCP Socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Tao socket that bai. Ma loi: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 3. Cấu hình địa chỉ Server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Lắng nghe trên mọi giao diện mạng
    serverAddr.sin_port = htons(PORT);

    // 4. Gắn (Bind) socket với địa chỉ và cổng
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind that bai. Ma loi: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // 5. Lắng nghe kết nối
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen that bai. Ma loi: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "=== TCP SERVER DANG CHAY (Port " << PORT << ") ===" << std::endl;
    std::cout << "Dang cho ket noi tu Client...\n" << std::endl;

    // 6. Vòng lặp chấp nhận kết nối và xử lý yêu cầu liên tục
    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept that bai. Ma loi: " << WSAGetLastError() << std::endl;
            continue;
        }

        // Lấy thông tin IP và Port của Client
        char* clientIP = inet_ntoa(clientAddr.sin_addr);
        int clientPort = ntohs(clientAddr.sin_port);
        std::cout << "[+] Nhan yeu cau tu Client - IP: " << clientIP << ", Port: " << clientPort << std::endl;

        // Vòng lặp nhận dữ liệu liên tục từ Client hiện tại
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                std::string data(buffer);
                std::cout << "    Du lieu goc tu Client (" << clientIP << ":" << clientPort << "): \"" << data << "\"" << std::endl;

                // Đổi chữ cái đầu tiên thành chữ in hoa
                capitalizeWords(data);
                std::cout << "    Du lieu sau khi xu ly: \"" << data << "\"" << std::endl;

                // Gửi trả kết quả về cho Client
                send(clientSocket, data.c_str(), data.length(), 0);
            } else {
                // Khi Client ngắt kết nối (thoát chương trình)
                std::cout << "[-] Client (" << clientIP << ":" << clientPort << ") da ngat ket noi." << std::endl;
                break;
            }
        }

        // Đóng kết nối với Client hiện tại và chờ Client tiếp theo
        closesocket(clientSocket);
        std::cout << "------------------------------------------" << std::endl;
    }

    // 7. Dọn dẹp
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}