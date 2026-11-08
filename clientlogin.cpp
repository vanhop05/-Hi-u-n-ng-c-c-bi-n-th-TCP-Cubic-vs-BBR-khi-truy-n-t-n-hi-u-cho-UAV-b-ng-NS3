#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    string serverIP = "127.0.0.1";
    int port = 8888;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    cout << "===== CLIENT LOGIN/LOGOUT =====" << endl;
    cout << "Lenh ho tro:" << endl;
    cout << "- LOGIN <id> <password>" << endl;
    cout << "- LOGOUT" << endl;
    cout << "- EXIT (De thoat chuong trinh)" << endl;
    cout << "===============================" << endl;

    char buffer[1024];
    int serverAddrLen = sizeof(serverAddr);

    while (true) {
        cout << "\nNhap lenh: ";
        string command;
        getline(cin, command);

        if (command == "EXIT") {
            break;
        }
        if (command.empty()) {
            continue;
        }

        // Gửi yêu cầu lên Server
        sendto(clientSocket, command.c_str(), command.length(), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

        // Nhận phản hồi
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&serverAddr, &serverAddrLen);
        
        if (bytesReceived > 0) {
            cout << "Server: " << buffer << endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}