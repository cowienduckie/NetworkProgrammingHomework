#include "stdio.h"
#include "winsock2.h"
#include "ws2tcpip.h"
#include <conio.h>

#define SERVER_PORT_DEFAULT 5500
#define SERVER_ADDR_DEFAULT "127.0.0.1"
#define BUFFER_SIZE 2048
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char** argv)
{
	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
	{
		printf("WinSock 2.2 is not supported\n");
		return 0;
	}

	//Construct socket
	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create client socket.", WSAGetLastError());
		return 0;
	}

	//Take initial server address and port
	int SERVER_PORT = SERVER_PORT_DEFAULT;
	const char* SERVER_ADDR = SERVER_ADDR_DEFAULT;

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;

	if (argc == 3)
	{
		//Validate port
		if (atoi(argv[2]) > 0 && atoi(argv[2]) <= 65353)
		{
			SERVER_PORT = atoi(argv[2]);
		}
		else
		{
			printf("Invalid initial port!\n");
			_getch();
			return 1;
		}

		//Validate address
		serverAddr.sin_port = htons(SERVER_PORT);

		if (!inet_pton(AF_INET, argv[1], &serverAddr.sin_addr))
		{
			printf("Invalid initial address!\n");
			_getch();
			return 1;
		}
		else
		{
			SERVER_ADDR = argv[1];
		}
	}
	else
	{
		serverAddr.sin_port = htons(SERVER_PORT);
		inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	}
	printf("Client started!\n");
	printf("Server address: %s\n", SERVER_ADDR);
	printf("Server port:    %d\n", SERVER_PORT);

	int timeoutInterval = 10000; //Time-out interval: 10,000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&timeoutInterval), sizeof(int));

	//Communication with client
	char buff[BUFFER_SIZE];
	int ret = 0, messageLen = 0, serverAddrLen = sizeof(serverAddr);
	const char FORWARD_PREFIX = 'F';
	const char REVERSE_PREFIX = 'R';

	while (true)
	{
		int option = -1;
		bool isExit = false;
		char prefix;
		memset(buff, 0, BUFFER_SIZE);

		printf("\n---------------------------------------------\n");
		printf("1. Translate Domain name to IPv4 address\n");
		printf("2. Translate IPv4 address to Domain name\n");
		printf("0. Exit\n\n");
		printf("Choose an option: ");  scanf_s("%d", &option); getchar();
		printf("---------------------------------------------\n");

		switch (option)
		{
		case 1:
			prefix = FORWARD_PREFIX;
			break;
		case 2:
			prefix = REVERSE_PREFIX;
			break;
		case 0:
			isExit = true;
			break;
		default:
			continue;
			break;
		}
		if (isExit) break; //Choose 0

		while (true)
		{
			//Send message
			printf("Send to server: ");
			buff[0] = prefix;
			gets_s(buff + 1, BUFFER_SIZE - 1);
			messageLen = strlen(buff);
			if (messageLen == 1) break;

			ret = sendto(client, buff, messageLen, 0, (sockaddr*)&serverAddr, serverAddrLen);
			if (ret == SOCKET_ERROR)
			{
				printf("Error %d: Cannot send message.", WSAGetLastError());
			}

			//Receive echo message
			ret = recvfrom(client, buff, BUFFER_SIZE, 0, (sockaddr*)&serverAddr, &serverAddrLen);

			if (ret == SOCKET_ERROR)
			{
				if (WSAGetLastError() == WSAETIMEDOUT)
				{
					printf("Time-out!\n");
				}
				else
				{
					printf("Error %d: Cannot receive message.\n", WSAGetLastError());
				}
			}
			else if (strlen(buff) > 1)
			{
				buff[ret] = 0;
				const char* returnState;

				if (buff[0] == '+')
				{
					returnState = "Success";
				}
				else if (buff[0] == '-')
				{
					returnState = "Failed";
				}
				else
				{
					returnState = "Unknown";
				}

				printf("Receive from server: %s\n%s\n\n", returnState, buff + 1);
			}
		}
	}

	//Close socket
	closesocket(client);

	//Terminate WinSock
	WSACleanup();

	return 0;
}