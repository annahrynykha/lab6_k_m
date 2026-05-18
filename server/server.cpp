#include <iostream>
#include <vector>
#include <winsock2.h>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    vector<SOCKET> sockets;
    vector<WSAEVENT> events;

    WSAEVENT listenEvent = WSACreateEvent();
    WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE);

    sockets.push_back(listenSocket);
    events.push_back(listenEvent);

    while (true) {
        bool modified = false;

        for (size_t i = 0; i < events.size(); i += WSA_MAXIMUM_WAIT_EVENTS) {
            DWORD count = min((DWORD)WSA_MAXIMUM_WAIT_EVENTS, (DWORD)(events.size() - i));
            DWORD ret = WSAWaitForMultipleEvents(count, &events[i], FALSE, 1, FALSE);

            if (ret >= WSA_WAIT_EVENT_0 && ret < WSA_WAIT_EVENT_0 + count) {
                int index = i + (ret - WSA_WAIT_EVENT_0);

                WSANETWORKEVENTS networkEvents;
                WSAEnumNetworkEvents(sockets[index], events[index], &networkEvents);

                if (networkEvents.lNetworkEvents & FD_ACCEPT) {
                    SOCKET clientSocket = accept(sockets[index], NULL, NULL);
                    if (clientSocket != INVALID_SOCKET) {
                        WSAEVENT clientEvent = WSACreateEvent();
                        WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_CLOSE);

                        sockets.push_back(clientSocket);
                        events.push_back(clientEvent);
                        modified = true;

                        cout << "A new client has joined! Total clients: "
                            << sockets.size() - 1 << std::endl; 
                    }
                }

                if (networkEvents.lNetworkEvents & FD_READ) {
                    char buffer[2048];
                    int bytes = recv(sockets[index], buffer, sizeof(buffer) - 1, 0);
                    if (bytes > 0) {
                        buffer[bytes] = '\0';
                        for (size_t j = 1; j < sockets.size(); ++j) {
                            if (j != index) {
                                send(sockets[j], buffer, bytes, 0);
                            }
                        }
                    }
                }

                if (networkEvents.lNetworkEvents & FD_CLOSE) {
                    closesocket(sockets[index]);
                    WSACloseEvent(events[index]);
                    sockets.erase(sockets.begin() + index);
                    events.erase(events.begin() + index);
                    modified = true;
                }
            }
            if (modified) break;
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}