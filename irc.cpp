// irc.cpp
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512 //4096

void sendIrc(SOCKET sock, std::string buffer) {
	send(sock, buffer.c_str(), buffer.size(), 0);
}

int main(){
	std::cout << "Simple IRC \n";
	std::ofstream logfile("test.txt", std::ofstream::app);
	// https://docs.microsoft.com/en-us/windows/win32/winsock/getting-started-with-winsock

	const char* server = "chat.freenode.net"; 
	const char* port = "6667";
	
	std::string User = "Test";
	std::string serverPass = "";
	std::string Nick = "test";
	std::string Channel = "#t";
	std::string channelPass = "";

	WSADATA wsaData;

	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(server, port, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Connect to server.
	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}


	int recvbuflen = DEFAULT_BUFLEN;
	char recvbuf[DEFAULT_BUFLEN];

	// Send an initial buffer
	// Send the connection info
	if(!serverPass.empty())
		sendIrc(ConnectSocket, "PASS " + serverPass + " \r\n");

	sendIrc(ConnectSocket, "NICK " + Nick + " \r\n");
	sendIrc(ConnectSocket, "USER " + User + " 0 * :" + User + " \r\n");
	if (channelPass.empty())
		sendIrc(ConnectSocket, "JOIN " + Channel + " \r\n");
	else
		sendIrc(ConnectSocket, "JOIN " + Channel + " " + channelPass + " \r\n");
	

	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	printf("Bytes Sent: %ld\n", iResult);


	// Receive data until the server closes the connection
	// Parse here
	do {
		memset(&recvbuf, '\0', sizeof(recvbuf)); //added, make sure the buffer is empty (removes the ||| junk) could also use Zeromemory
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			//printf("%s\n", recvbuf);
			logfile << recvbuf;

			std::string stringbuf = std::string(recvbuf); //convert to string to perform operations
			std::cout << stringbuf;
			if (stringbuf.rfind("PING ", 0) == 0) { // starts with
				std::string pong = "PONG " + stringbuf.substr(6, stringbuf.size());
				sendIrc(ConnectSocket, pong + "\r\n");
				printf("%s", pong.c_str());
			}

		}
		else if (iResult == 0)
			printf("Connection closed\n");
		else
			printf("recv failed: %d\n", WSAGetLastError());
	} while (iResult > 0);

	// shutdown then send half of the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	
	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();
	logfile.close();
	return EXIT_SUCCESS;
}


