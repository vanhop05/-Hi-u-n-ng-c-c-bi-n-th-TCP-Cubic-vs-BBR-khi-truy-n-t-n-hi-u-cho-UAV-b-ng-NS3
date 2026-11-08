
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main(int argc, char* argv[]) {
    // Kiểm tra cú pháp dòng lệnh: serverUDP.exe <port_number>
    if (argc != 2) {
        cout << "Cu phap: serverUDP.exe <port_number>" << endl;
        return 1;
    }
    
    int port = atoi(argv[1]);

    // Khởi tạo thư viện Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Loi khoi tao Winsock!" << endl;
        return 1;
    }

    // Tạo socket UDP
    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Loi tao socket!" << endl;
        WSACleanup();
        return 1;
    }

    // Cấu hình bind
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Loi bind!" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server UDP dang lang nghe tren cong " << port << "..." << endl;

    char buffer[1024];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        // Nhận tên miền từ client
        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&clientAddr, &clientAddrLen);
        
        if (bytesReceived > 0) {
            string domainName(buffer);
            cout << "\nNhan yeu cau phan giai ten mien: " << domainName << endl;

            // Tiến hành chuyển sang tên dạng địa chỉ IPv4
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET; // Chỉ lấy IPv4 gồm 4 số

            string ipResult = "Khong tim thay hoac loi phan giai";
            if (getaddrinfo(domainName.c_str(), NULL, &hints, &res) == 0) {
                struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
                char ipStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(ipv4->sin_addr), ipStr, sizeof(ipStr));
                ipResult = string(ipStr);
                freeaddrinfo(res);
            }

            // Hiển thị và trả kết quả về cho client
            cout << "Ket qua: " << ipResult << endl;
            sendto(serverSocket, ipResult.c_str(), ipResult.length(), 0, (SOCKADDR*)&clientAddr, clientAddrLen);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}