#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <conio.h>

#define SERVER_PORT_DEFAULT 5500
#define SERVER_ADDR_DEFAULT "127.0.0.1"
#define BUFF_SIZE 2048
#define MESSAGE_QUEUE_SIZE 2048*5+4
#define ENDING_DELIMITER "\r\n"
#define SUCCESS_PREFIX "+"
#define FAILED_PREFIX "-"



#pragma comment(lib, "Ws2_32.lib")

char** splitString(char* string, int size, const char* delimiter, int* elemNumber);

int main(int argc, char* argv[])
{
	// Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
	{
		printf("Winsock 2.2 is not supported.\n");
		return 1;
	}

	// Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create server socket.\n", WSAGetLastError());
		return 1;
	}

	// Set time-out for receiving
	int tv = 10000;		// Time-out interval: 10s
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

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
		SERVER_ADDR = argv[1];
	}

	// Request to connect server
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot connect server.\n", WSAGetLastError());
		return 1;
	}

	printf("Connected server!\n");
	printf("Server address: %s\n", SERVER_ADDR);
	printf("Server port:    %d\n", SERVER_PORT);

	// Communicate with server
	char buff[BUFF_SIZE];
	int ret, messageLen;

	while (true)
	{
		//Send message
		printf("Send to server: ");
		gets_s(buff, BUFF_SIZE);
		strcat_s(buff, ENDING_DELIMITER);
		//strcat_s(buff, "abc\r\n");
		messageLen = strlen(buff);
		if (messageLen == 0) break;

		ret = send(client, buff, messageLen, 0);
		if (ret == SOCKET_ERROR)
			printf("Error %d: Cannot send data.\n", WSAGetLastError());

		//Receive echo message
		ret = recv(client, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
			{
				printf("Time-out!\n");
			}
			else
			{
				printf("Error: %d: Cannot receiver data.\n", WSAGetLastError());
			}
		}
		else if (strlen(buff) > 0)
		{
			buff[ret] = 0;
			printf("Receive from server: %s\n", buff);
		}
	}

	// Close socket
	closesocket(client);

	// Terminate Winsock
	WSACleanup();

	return 0;
}

char** splitString(char* string, int size, const char* delimiter, int* elemNumber)
{
	char** ptr = (char**)malloc(size * sizeof(char*));
	char* p = string;
	char* next_p = NULL;
	int i = 0;

	p = strtok_s(string, delimiter, &next_p);

	while (p != NULL)
	{
		ptr[i++] = p;
		p = strtok_s(NULL, delimiter, &next_p);
	}

	*elemNumber = i;

	return ptr;
}