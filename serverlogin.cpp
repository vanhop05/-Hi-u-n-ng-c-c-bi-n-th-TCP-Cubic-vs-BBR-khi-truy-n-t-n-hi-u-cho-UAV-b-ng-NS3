#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Co so du lieu tai khoan
map<string, string> users_db = {
    {"admin", "12345"},
    {"user", "abc"}
};

// Quan ly trang thai dang nhap
map<string, bool> active_sessions;

// Ham tao khoa dinh danh client tu IP va Port
string getClientKey(sockaddr_in clientAddr) {
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, sizeof(ipStr));
    return string(ipStr) + ":" + to_string(ntohs(clientAddr.sin_port));
}

// Ham phan tich chuoi thanh cac tu
vector<string> splitMessage(const string& msg) {
    vector<string> words;
    stringstream ss(msg);
    string word;
    while (ss >> word) {
        words.push_back(word);
    }
    return words;
}

int main() {
    int port = 8888;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    cout << "Server dang chay tai port " << port << "..." << endl;

    char buffer[1024];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (SOCKADDR*)&clientAddr, &clientAddrLen);
        
        if (bytesReceived > 0) {
            string msg(buffer);
            string clientKey = getClientKey(clientAddr);
            vector<string> tokens = splitMessage(msg);
            string response = "";

            if (tokens.empty()) continue;

            // XU LY LENH LOGIN
            if (tokens[0] == "LOGIN") {
                if (tokens.size() != 3) {
                    response = "Sai dinh dang thong diep"; 
                } else {
                    string id = tokens[1];
                    string pass = tokens[2];

                    if (users_db.find(id) == users_db.end()) {
                        response = "ID khong ton tai"; 
                    } else if (users_db[id] != pass) {
                        response = "Password khong dung"; 
                    } else {
                        active_sessions[clientKey] = true;
                        response = "Dang nhap thanh cong !!!"; 
                    }
                }
            }
            // XU LY LENH LOGOUT
            else if (tokens[0] == "LOGOUT") {
                if (tokens.size() != 1) {
                    response = "Sai dinh dang thong diep"; 
                } else if (!active_sessions[clientKey]) {
                    response = "Chua dang nhap"; 
                } else {
                    active_sessions[clientKey] = false;
                    response = "Dang xuat thanh cong !!!"; 
                }
            }
            // SAI CU PHAP CHUNG
            else {
                response = "Sai dinh dang thong diep"; 
            }

            // Gui phan hoi ve cho Client
            cout << "[" << clientKey << "] Yeu cau: " << msg << " -> Phan hoi: " << response << endl;
            sendto(serverSocket, response.c_str(), response.length(), 0, (SOCKADDR*)&clientAddr, clientAddrLen);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}