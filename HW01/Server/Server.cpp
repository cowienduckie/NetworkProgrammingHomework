#include "stdio.h"
#include "conio.h"
#include "winsock2.h"
#include "ws2tcpip.h"

#define SERVER_PORT_DEFAULT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 2048

#pragma comment(lib, "Ws2_32.lib")

/*
* @function DomainNameToIPAddr: Translate domain name to IPv4 Address
* @param domainName: A pointer to domain name
* @param msg: A pointer to return message sent to client
*/
void DomainNameToIPAddr(char* domainName, char* msg);

/*
* @function IPAddrToDomainName: Translate domain name to IPv4 Address
* @param hostAddr: A pointer to host IPv4 Address
* @param msg: A pointer to return message sent to client
*/
void IPAddrToDomainName(char* hostAddr, char* msg);

int main(int argc, char** argv)
{
	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
	{
		printf("WinSock 2.2 is not supported\n");
		return 1;
	}

	//Construct socket
	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == INVALID_SOCKET)
	{
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
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

	printf("Server started!\n");
	printf("Server address: %s\n", SERVER_ADDR);
	printf("Server port:    %d\n", SERVER_PORT);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot bind this address.", WSAGetLastError());
		_getch();
		return 1;
	}

	//Communication with client
	sockaddr_in clientAddr;
	char buff[BUFFER_SIZE], clientIP[INET_ADDRSTRLEN];
	int ret = 0, clientPort = 0, clientAddrLen = sizeof(clientAddr);

	while (true)
	{
		//Receive message
		ret = recvfrom(server, buff, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR)
		{
			printf("Error %d: Cannot receive data.", WSAGetLastError());
		}
		else
		{
			buff[ret] = 0;
			char returnMessage[BUFFER_SIZE];
			strcpy_s(returnMessage, BUFFER_SIZE, "+");
			const int PREFIX = 0;
			const char FORWARD_PREFIX = 'F';
			const char REVERSE_PREFIX = 'R';

			if (buff[PREFIX] == FORWARD_PREFIX)			//Translate forward Domain Name -> IP Address
			{
				DomainNameToIPAddr(buff + 1, returnMessage);
			}
			else if (buff[PREFIX] == REVERSE_PREFIX)	//Translate reverse IP Address -> Domain Name
			{
				IPAddrToDomainName(buff + 1, returnMessage);
			}
			else    //Wrong client request
			{
				strcpy_s(returnMessage, BUFFER_SIZE, "-Wrong request!");
			}

			//Return message
			if (strlen(returnMessage) == 1)		//Translation has been failed
			{
				strcpy_s(returnMessage, BUFFER_SIZE, "-Not found information!");
			}

			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			printf("\nReceive from client[%s:%d] %s\n", clientIP, clientPort, buff + 1);

			//Send return message to client
			ret = sendto(server, returnMessage, strlen(returnMessage), 0, (SOCKADDR*)&clientAddr, sizeof(clientAddr));
			if (ret == SOCKET_ERROR)
			{
				printf("Error %d: Cannot send return message\n", WSAGetLastError());
			}
		}
	}

	//Close socket
	closesocket(server);

	//Terminate WinSock
	WSACleanup();

	return 0;
}

void DomainNameToIPAddr(char* domainName, char* msg)
{
	addrinfo* result;   //pointer to the linked-list
						//containing information about the host
	int rc;
	sockaddr_in* address;
	addrinfo hints; //pointer to the linked-list
	hints.ai_family = AF_INET; //only focus on IPv4 address
	memset(&hints, 0, sizeof(hints));
	rc = getaddrinfo(domainName, NULL, &hints, &result);
	// Get the address info
	char ipStr[INET_ADDRSTRLEN];
	if (rc == 0) {
		while (result != NULL)
		{
			address = (struct sockaddr_in*)result->ai_addr;
			inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));

			strcat_s(msg, BUFFER_SIZE, ipStr);
			strcat_s(msg, BUFFER_SIZE, "\n");

			result = result->ai_next;
		}
	}
	else
	{
		printf("getaddrinfo() error: %d\n", WSAGetLastError());
	}

	// free linked-list
	freeaddrinfo(result);
}

void IPAddrToDomainName(char* hostAddr, char* msg)
{
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(80);

	if (!inet_pton(AF_INET, hostAddr, &address.sin_addr))
	{
		strcpy_s(msg, BUFFER_SIZE, "-IP address is not legal!");
		return;
	}

	DWORD dwretval = getnameinfo((sockaddr*)&address,
		sizeof(struct sockaddr),
		hostname,
		NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);

	strcat_s(msg, BUFFER_SIZE, hostname);
}