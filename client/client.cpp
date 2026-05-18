#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

void receiveMessages(SOCKET sock) {
    char buffer[2048];
    while (true) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            cout << "\n[Message]: " << buffer << "\n> ";
        }
        else {
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    thread recvThread(receiveMessages, sock);
    recvThread.detach();

    string msg;
    cout << "> ";
    while (getline(cin, msg)) {
        if (!msg.empty()) {
            send(sock, msg.c_str(), msg.length(), 0);
            cout << "> ";
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}