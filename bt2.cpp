#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    //  Khởi tạo WinSock 
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Khong the khoi tao WinSock!\n";
        return 1;
    }
    // KHỞI TẠO SOCKET CHO SERVER
    
    // Tạo socket sử dụng IPv4 (AF_INET) và giao thức TCP (SOCK_STREAM)
    SOCKET serverSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    
    if (serverSocket == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        std::cout << "Creating socket failed with code " << errCode << "\n";
        WSACleanup();
        return 1; // Thoát nếu tạo socket lỗi
    } else {
        std::cout << "Creating socket completed successfully.\n";
    }
 //  HÀM BIND 
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET; // Sử dụng IPv4             
    serverAddr.sin_port = htons(8080);              
    serverAddr.sin_addr.s_addr = INADDR_ANY;         

    // Thực hiện hàm bind
    int bindResult = bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    
    if (bindResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        std::cout << "Bind API failed with code " << errCode << "\n";
        closesocket(serverSocket); // Đóng socket trước khi thoát
        WSACleanup();
        return 1;
    } else {
        std::cout << "Bind API completed successfully.\n";
    }

 //  HÀM LISTEN 
    int listenResult = listen(serverSocket, SOMAXCONN);
    
    if (listenResult == SOCKET_ERROR) {
        int errCode = WSAGetLastError();
        std::cout << "Listen failed with code " << errCode << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    } else {
        std::cout << "Successfully !!! Server is listening the requests ...\n";
    }

    std::cout << "Dang cho client ket noi ...\n";
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);

    if (clientSocket == INVALID_SOCKET) {
        int errCode = WSAGetLastError();
        std::cout << "Accept failed with code " << errCode << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client da ket noi. Dang cho client phan hoi ...\n";
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Client phan hoi: " << buffer << "\n";
    } else if (bytesReceived == 0) {
        std::cout << "Client da dong ket noi.\n";
    } else {
        int errCode = WSAGetLastError();
        std::cout << "Receive failed with code " << errCode << "\n";
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}