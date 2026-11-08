#include <iostream>
#include <winsock2.h>
#include <string>
#include <cctype> 

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 1. Thiết lập Socket & Lắng nghe
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);
    std::cout << "[SERVER] Dang lang nghe tai cong 8080...\n";

    // 2. Chấp nhận kết nối từ Client
    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    std::cout << "[SERVER] Client da ket noi thanh cong!\n";

    char buffer[1024];

    // 3. Vòng lặp nhận và xử lý dữ liệu
    while (true) {
        memset(buffer, 0, sizeof(buffer)); // Xóa bộ đệm
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if (bytesReceived <= 0) {
            std::cout << "[SERVER] Client da ngat ket noi.\n";
            break;
        }

        std::string input(buffer);
        std::string upperOnly = "";
        std::string alphaLowerOnly = "";

        // Xử lý chuỗi theo yêu cầu đề bài
        for (char c : input) {
            //  Lấy các chữ cái in hoa
            if (isupper(c)) {
                upperOnly += c;
            }
            //  Loại bỏ chữ số/ký tự đặc biệt, đưa về chữ thường để kiểm tra
            if (isalpha(c)) {
                alphaLowerOnly += tolower(c);
            }
        }

        // Tạo nội dung phản hồi
        std::string response = upperOnly;
        if (response.empty()) response = "(Khong co ky tu in hoa nao)";

        bool shouldExit = (alphaLowerOnly == "exit" || alphaLowerOnly == "quit");

        if (shouldExit) {
            response += "\ngood bye"; 
        }

        // Gửi phản hồi lại cho Client
        send(clientSocket, response.c_str(), response.length(), 0);

        // Ngắt kết nối khi nhận được lệnh thoát
        if (shouldExit) {
            std::cout << "[SERVER] Nhan duoc lenh thoat. Dang dong Server...\n";
            break;
        }
    }

    // 4. Dọn dẹp
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}