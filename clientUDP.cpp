
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main(int argc, char* argv[]) {
    // Kiểm tra cú pháp dòng lệnh: clientUDP.exe <server_IP> <port_number> <domain_name>[cite: 2]
    if (argc != 4) {
        cout << "Cu phap: clientUDP.exe <server_IP> <port_number> <domain_name>" << endl;
        return 1;
    }

   
    string serverIP = argv[1];
    int port = atoi(argv[2]);
    string domainName = argv[3];

    // Khởi tạo thư viện Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Loi khoi tao Winsock!" << endl;
        return 1;
    }

    // Tạo socket UDP
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Loi tao socket!" << endl;
        WSACleanup();
        return 1;
    }

    // Cấu hình địa chỉ server để gửi đến
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    // Gửi tên miền cần xác định địa chỉ IP đến server
    sendto(clientSocket, domainName.c_str(), domainName.length(), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    cout << "Da gui ten mien '" << domainName << "' toi server " << serverIP << ":" << port << endl;

    // Chờ kết quả từ server và hiển thị
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int serverAddrLen = sizeof(serverAddr);
    
    int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&serverAddr, &serverAddrLen);
    
    if (bytesReceived > 0) {
        cout << "Ket qua tu server: " << domainName << " -> " << buffer << endl;
    } else {
        cout << "Khong nhan duoc phan hoi tu server." << endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}