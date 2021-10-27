#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <conio.h>
#include <process.h>

#define SERVER_PORT_DEFAULT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define MESSAGE_QUEUE_SIZE 2048*5+4
#define ENDING_DELIMITER "\r\n"

#pragma comment(lib, "Ws2_32.lib")

unsigned __stdcall ClientProcess(void* data);

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
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create server socket.\n", WSAGetLastError());
		return 1;
	}

	//Take initial server port
	int SERVER_PORT = SERVER_PORT_DEFAULT;

	if (argc == 2)
	{
		if (atoi(argv[1]) > 0 && atoi(argv[1]) <= 65353)
		{
			SERVER_PORT = atoi(argv[1]);
		}
		else
		{
			printf("Invalid initial port!\n");
			_getch();
			return 1;
		}
	}

	// Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.\n", WSAGetLastError());
		return 1;
	}

	// Listen request from client
	if (listen(listenSock, 10))
	{
		printf("Error %d: Cannot place server socket in state LISTEN.\n", WSAGetLastError());
		return 1;
	}

	printf("Server started!\n");
	printf("Server address: %s\n", SERVER_ADDR);
	printf("Server port:    %d\n", SERVER_PORT);

	// Communicate with client
	while (true)
	{
		// Accept request
		SOCKET connSock = SOCKET_ERROR;

		while (connSock == SOCKET_ERROR)
		{
			connSock = accept(listenSock, NULL, NULL);
		}

		unsigned threadID;
		HANDLE Thread = (HANDLE)_beginthreadex(NULL, 0, &ClientProcess, (void*)connSock, 0, &threadID);
	}

	// Close socket
	closesocket(listenSock);

	// Terminate Winsock
	WSACleanup();

	return 0;
}

unsigned __stdcall ClientProcess(void* data)
{
	SOCKET connSock = (SOCKET)data;

	// Process the client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE],
		clientIP[INET_ADDRSTRLEN];
	int ret = 0,
		clientPort = 0,
		clientAddrLen = sizeof(clientAddr);

	getpeername(connSock, (sockaddr*)&clientAddr, &clientAddrLen);

	clientPort = ntohs(clientAddr.sin_port);
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
	printf("Accept incoming connection from %s:%d\n", clientIP, clientPort);

	char messageQueue[MESSAGE_QUEUE_SIZE];
	int rear = 0;
	strcpy_s(messageQueue, "");

	while (true)
	{
		// Receive message from client
		ret = recv(connSock, buff, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Error %d: Cannot receive data.\n", WSAGetLastError());
			break;
		}
		else if (ret == 0)
		{
			printf("Client[%s:%d] disconnects.\n", clientIP, clientPort);
			break;
		}
		else
		{
			buff[ret] = 0;
			printf("Receive from client[%s:%d] %s\n", clientIP, clientPort, buff);

			rear += ret;

			if (rear <= MESSAGE_QUEUE_SIZE)
			{
				char postfix[] = ".,.,";
				strcat_s(messageQueue, buff);
				strcat_s(messageQueue, postfix);

				int elemNumber = 0;
				char** messages = splitString((char*)&messageQueue, MESSAGE_QUEUE_SIZE, ENDING_DELIMITER, &elemNumber);

				// Check the last one
				char* last = messages[elemNumber - 1];
				int lastLen = strlen(last);

				if (lastLen > 4)
				{
					rear = lastLen - 4;
				}
				else
				{
					rear = 0;
				}
				--elemNumber;
				// Process all messages
				for (int i = 0; i < elemNumber; ++i)
				{
					char returnMessage[BUFF_SIZE];

					strcpy_s(returnMessage, messages[i]);
					strcat_s(returnMessage, "\n");

					printf("Sent to client: %s [%d]\n", returnMessage, strlen(returnMessage));

					ret = send(connSock, returnMessage, strlen(returnMessage), 0);

					if (ret == SOCKET_ERROR)
					{
						printf("Error %d: Cannot send data.\n", WSAGetLastError());
					}
				}
				free(messages);

				if (rear != 0)
				{
					strcpy_s(messageQueue, last);
				}

				messageQueue[rear] = 0;
			}
			else
			{
				printf("Message is longer than maximum %d bytes.\n", MESSAGE_QUEUE_SIZE);
				break;
			}
		}
	}

	// Close socket
	closesocket(connSock);
}

char* calcSum(char* string)
{
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