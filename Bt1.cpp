#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h> 
int main() {
    // ==========================================
    
    // ==========================================
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Khoi tao Winsock that bai.\n");
        return 1;
    }

    // 1. Khai báo mã sinh viên 
    unsigned long ma_sv = 106230013; 
    
    // 2. Tính toán giá trị x = ([mã SV] modulo 255) + 1
    int x = (ma_sv % 255) + 1;
    
    // 3. Tạo chuỗi địa chỉ IP "127.0.0.x"
    char ip_str[20];
    sprintf(ip_str, "127.0.0.%d", x);
    printf("Dia chi IP can chuyen doi: %s\n", ip_str);
    printf("----------------------------------------\n");

    // ==========================================
    // CÁCH 1: Dùng hàm inet_addr
    // ==========================================
    unsigned long ip_bin_1 = (unsigned long)inet_addr(ip_str);
    
    if (ip_bin_1 == INADDR_NONE) {
        printf("Cach 1 (inet_addr): Loi chuyen doi dia chi IP!\n");
    } else {
        printf("Cach 1 (inet_addr): Gia tri so (unsigned long) la: %lu\n", ip_bin_1);
    }

    // ==========================================
    // CÁCH 2: Dùng hàm inet_pton
    // ==========================================
    struct in_addr ip_bin_2; 
    
    if (inet_pton(AF_INET, ip_str, &ip_bin_2) == 1) {
        unsigned long result_2 = (unsigned long)ip_bin_2.s_addr;
        printf("Cach 2 (inet_pton): Gia tri so (unsigned long) la: %lu\n", result_2);
    } else {
        printf("Cach 2 (inet_pton): Loi chuyen doi dia chi IP!\n");
    }

    // Dọn dẹp tài nguyên Winsock trước khi kết thúc
    WSACleanup();

    return 0;
}