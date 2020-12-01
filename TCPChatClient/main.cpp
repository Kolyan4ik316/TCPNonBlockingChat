#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <string>

#pragma comment (lib, "ws2_32.lib")



int main()
{
	// Startup winsock
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data))
	{
		std::cout << "WSAStartup failed " << WSAGetLastError() << std::endl;
		return -1;
	}
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(53000);
	int sizeSH = sizeof(serverAddr);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	SOCKET connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(connection, (SOCKADDR*)&serverAddr, sizeSH) == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		std::cout << "SOCKET_ERROR" << WSAGetLastError() << std::endl;
		return -1;
	}
	unsigned long enabled = 1;
	ioctlsocket(connection, FIONBIO, &enabled);
	int error = WSAGetLastError();
	if (error == SOCKET_ERROR && error != WSAEWOULDBLOCK)
	{
		std::cout << "Client socket error " << WSAGetLastError() << std::endl;
		return -1;
	}
	bool isRunning = true;
	std::thread receiver([&]()
		{
			while (isRunning)
			{
				char msg[256];
				ZeroMemory(msg, sizeof(msg));
				if (recv(connection, msg, sizeof(msg), 0) > 0)
				{
					std::cout << "Message from server : " << msg << std::endl;
				}
			}
		});
	while (isRunning)
	{	
		char msgS[256];
		std::cin >> msgS;
		std::string msgString = msgS;
		
		if (send(connection, msgS, sizeof(msgS), 0) == SOCKET_ERROR)
		{
			std::cout << "Client send error " << WSAGetLastError() << std::endl;
		}
		if (msgString.compare("out") == 0)
		{
			isRunning = false;
		}
	}
	receiver.detach();
	system("pause");
	return 0;
}