#include <iostream>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    vector<SOCKET> clients;
    int targetClients = 1100;

    cout << "Starting connecting " << targetClients << " clients...\n";

    for (int i = 0; i < targetClients; ++i) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) != SOCKET_ERROR) {
            u_long mode = 1;
            ioctlsocket(sock, FIONBIO, &mode);

            clients.push_back(sock);
            if (i % 100 == 0) cout << "Connected " << i << " clients...\n";
        }
        else {
            cout << "Client connection error " << i << ". Code: " << WSAGetLastError() << "\n";
            break;
        }
    }

    cout << "\nSuccessfully connected " << clients.size() << " clients\n";

    if (clients.size() > 1) {
        cout << "\nWaiting 1 second for the server to register all FD_ACCEPT events...\n";
        Sleep(1000);

        string testMsg = "STRESS TEST MESSAGE";
        cout << "Sending a test message from client [0]...\n";
        send(clients[0], testMsg.c_str(), testMsg.length(), 0);

        cout << "Waiting 2 seconds for the server to send the message to all others...\n";
        Sleep(2000);

        int receivedCount = 0;
        char buffer[1024];

        for (size_t i = 1; i < clients.size(); ++i) {
            int bytes = recv(clients[i], buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                receivedCount++;
            }
        }

        cout << "========================================\n";
        cout << "TEST RESULT:\n";
        cout << "Clients that successfully received the message: "
            << receivedCount << " of " << clients.size() - 1 << "\n";
        cout << "========================================\n";
    }

    cout << "Press Enter to disconnect all connections and log out...";
    cin.get();

    for (SOCKET sock : clients) {
        closesocket(sock);
    }
    WSACleanup();
    return 0;
}