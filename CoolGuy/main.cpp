#include "pch.h"

#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <array>
#include <cstdio>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

std::mutex mtx;

using namespace std;
std::string msgBuffer;
std::string consoleBuffer;
std::string result;
SOCKET clientSocket;

void sendFromBackgroudn(std::string msg)
{
    msgBuffer = msg;
}

void executeCommand(std::string command)
{
    if (clientSocket)
    {
        send(clientSocket, command.c_str(), command.size(), 0);
    }
    else {
        
        cout << "invalid client socket" << endl;
    }
}

void parseMessage()
{
    if (msgBuffer.size())
    {
        executeCommand(msgBuffer);
        msgBuffer = "";
    }
}

void checkInput()
{
    std::string input;
    while (true) {
        mtx.lock();
        if (std::cin.peek() != EOF) {
            std::getline(std::cin, input);
            if (input.size()) {
                msgBuffer = input;
                cout << "user msg: " << msgBuffer << endl;
                parseMessage();
            }
        }

        if (consoleBuffer.size())
        {
            cout << "Nice: " << endl << consoleBuffer.c_str() << endl;
            consoleBuffer = "";
        }

        mtx.unlock();
    }
}
void socketLoop();

void listenThread()
{
    std::thread t(&socketLoop);
    t.detach();
}


void main()
{
    listenThread();
    checkInput();
}

void socketLoop()
{
    // Inicie o winsock
    WSADATA wd;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wd);
    if (wsOk != 0)
    {
        cerr << "Erro ao iniciar o winsock!" << endl;
        return;
    }

    // Crie um socket
    SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET)
    {
        cerr << "Erro ao criar o socket!" << endl;
        return;
    }

    // Vincule o endereço IP e a porta ao socket
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;
    bind(listener, (sockaddr*)&hint, sizeof(hint));

    // Diga o winsock que o socket é um socket de escuta
    listen(listener, SOMAXCONN);

    // Espere uma conexão
    sockaddr_in client;
    int clientSize = sizeof(client);
    clientSocket = accept(listener, (sockaddr*)&client, &clientSize);
    char host[NI_MAXHOST];
    char svc[NI_MAXSERV];

    ZeroMemory(host, NI_MAXHOST);
    ZeroMemory(svc, NI_MAXSERV);
    

    if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0) == 0)
    {
        cout << host << " connected on port " << svc << endl;
    }
    else
    {
        inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
        cout << host << " connected on port " << ntohs(client.sin_port) << endl;
    }

    // Feche o escutador
    closesocket(listener);
    

    // Loop de espera para receber e ecoar a mensagem
    char buf[4096];

    while (true)
    {

        if (!clientSocket)
        {
            return;
        }
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            cerr << "Erro na recv()" << endl;
            break;
        }
        if (bytesReceived == 0)
        {
            cout << "Cliente desconectou" << endl;
            break;
        }

        //send(clientSocket, "teste", 6, 0);

        consoleBuffer = buf;

        /*

        if (strcmp(buf, "padaria") == 0)
        {
            // Envia uma string
            send(clientSocket, "teste", 6, 0);
        }
        */
    }
    // Feche o socket
    closesocket(clientSocket);
    // Feche o winsock
    WSACleanup();
}


