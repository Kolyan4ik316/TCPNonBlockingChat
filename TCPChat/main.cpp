#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <algorithm>
#include <string>
#pragma comment (lib, "ws2_32.lib")

struct PSocket
{
	SOCKET sock;
	char msg[256];
};

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
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY;
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(53000);
	int sizeSH = sizeof(serverHint);

	SOCKET in = socket(AF_INET, SOCK_STREAM, 0);
	unsigned long enabled = 1;
	ioctlsocket(in, FIONBIO, &enabled);
	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		std::cout << "Can't bind socket! " << WSAGetLastError() << std::endl;
		return -1;
	}
	listen(in, 0xFFFFFFFF);
	SOCKET out;
	
	std::vector<PSocket> outSockets;
	std::cout << "Yee" << std::endl;
	
	char MsgS[256];
	ZeroMemory(MsgS, sizeof(MsgS));
	while (true)
	{		
		out = accept(in, (sockaddr*)&serverHint, &sizeSH);
		int error = WSAGetLastError();
		if (out == INVALID_SOCKET && error != WSAEWOULDBLOCK)
		{
			std::cout << "Socket error! " << WSAGetLastError();
		}
		else
		{
			if (out != INVALID_SOCKET)
			{
				if (std::none_of(outSockets.cbegin(), outSockets.cend(), [&](const PSocket& sock)
					{
						return sock.sock == out;
					}))
				{
					outSockets.push_back({ out });
					std::cout << "New client has connected! " << std::endl;
					ZeroMemory(outSockets.back().msg, sizeof(outSockets.back().msg));
				}
			}
		}

		for (size_t i = 0; i < outSockets.size(); i++)
		{
			if (recv(outSockets.at(i).sock, outSockets.at(i).msg, sizeof(outSockets.at(i).msg), 0) > 0)
			{
				std::cout << "Client sends: " << outSockets.at(i).msg << std::endl;
				for (size_t j = 0; j < outSockets.size(); j++)
				{
					if (j == i)
					{
						std::string msgString = outSockets.at(i).msg;
						if (msgString.compare("out") == 0)
						{
							std::swap(outSockets.at(i), outSockets.back());
							outSockets.pop_back();
						}
						continue;
					}
					else
					{
						memcpy(outSockets.at(j).msg, outSockets.at(i).msg, 256);
						if (outSockets.at(j).msg != MsgS)
						{
							if (send(outSockets.at(j).sock, outSockets.at(j).msg, sizeof(outSockets.at(j).msg), 0) == SOCKET_ERROR)
							{
								std::cout << "Client sends error: " << WSAGetLastError() << std::endl;
							}
							ZeroMemory(outSockets.at(j).msg, sizeof(outSockets.at(j).msg));
						}
					}
					
				}
			}
		}
		
	}
	
	closesocket(in);
	system("pause");


	return 0;
}