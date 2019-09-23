#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <future>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include "client.h"
#include "socket.h"
Socket::Socket(){
	// Initialize Winsock version 2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) std::printf("Server: WSAStartup failed with error %ld\n", WSAGetLastError());
	else std::printf("Server: The Winsock DLL status is %s.\n", wsaData.szSystemStatus);
	// Create a new socket to receive datagrams on.
	ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);



	if (ReceivingSocket == INVALID_SOCKET)
		{

		std::printf("Server: Error at socket(): %ld\n", WSAGetLastError());
		// Clean up
		WSACleanup();
	}

	else std::printf("Server: socket() is OK!\n");

	// Set up a SOCKADDR_IN structure that will tell bind that we
	// want to receive datagrams from all interfaces using port 5150.
	// The IPv4 family
	ReceiverAddr.sin_family = AF_INET;

	// Port no. 5150
	ReceiverAddr.sin_port = htons(Port);

	// From all interface (0.0.0.0)
	ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);



	// Associate the address information with the socket using bind.
	// At this point you can receive datagrams on your bound socket.

	if (::bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr)) == SOCKET_ERROR){

		std::printf("Server: bind() failed! Error: %ld.\n", WSAGetLastError());
	// Close the socket
	closesocket(ReceivingSocket);
	// Do the clean up
	WSACleanup();
	}
	else std::printf("Server: bind() is OK!\n");

	
	u_long ulNonBlockingMode = 1;
	ioctlsocket(ReceivingSocket, FIONBIO, &ulNonBlockingMode);  
	// Some info on the receiver side...

	getsockname(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, (int *)sizeof(ReceiverAddr));
	std::printf("Server: Receiving IP(s) used: %s\n", inet_ntoa(ReceiverAddr.sin_addr));
	std::printf("Server: Receiving port used: %d\n", htons(ReceiverAddr.sin_port));
	//std::printf("Server: I\'m ready to receive a datagram...\n");
}
std::string Socket::acceptData() { // struct wait_until_ready_t *wait or struct wait_until_ready_t &wait
	int BytesReceived = 0;
	char ReceiveBuf[1024];
	int BufLength = sizeof(ReceiveBuf);
	int SenderAddrSize = sizeof(lastClientAddr);
	SOCKADDR sendingSocket;
	int x = 0;
	while (x < 5 && (BytesReceived == 0 || BytesReceived == -1)) {
		BytesReceived = recvfrom(ReceivingSocket, ReceiveBuf, BufLength, 0, (SOCKADDR *)&lastClientAddr, &SenderAddrSize); x++;
	} 
	//BytesReceived = recvfrom(ReceivingSocket, ReceiveBuf, BufLength, 0, (SOCKADDR *)&lastClientAddr, &SenderAddrSize);
	std::string returnBuf;
	ReceiveBuf[BufLength-1] = '\0';
	returnBuf = ReceiveBuf;
	if (BytesReceived <= 0) { returnBuf = "0";  } // printf("Received empty data. ReceiveBuf is: %s. Bytes Received: %d\n", ReceiveBuf, BytesReceived);
	return returnBuf;
}
int Socket::recvfromTimeOutUDP(long sec, long usec){
	// Setup timeval variable
	struct timeval timeout;
	struct fd_set fds;

	timeout.tv_sec = sec;

	timeout.tv_usec = usec;

	// Setup fd_set structure

	FD_ZERO(&fds);
	FD_SET(ReceivingSocket, &fds);

	// Return value:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, &fds, 0, 0, &timeout);
}
int Socket::keepAlive(SOCKADDR SenderAddr ,int clientID, Client *client)
{
	int replyCount = 0;
	SOCKADDR *tempsock = &SenderAddr;
	SOCKADDR_IN *temp = (SOCKADDR_IN *)tempsock;
	char *ip = inet_ntoa(temp->sin_addr);
	int BytesReceived;
	char ReceiveBuf[1024] = "    ";
	int SOCKET_READ_TIMEOUT_SEC = 1;
	DWORD timeout = SOCKET_READ_TIMEOUT_SEC * 1000;
	int SenderAddrSize = sizeof(SenderAddr);
	if (sendto(ReceivingSocket, "req", sizeof("req"), 0, (struct sockaddr*) (SOCKADDR *)&SenderAddr, SenderAddrSize) != -1) {
	}
	if (setsockopt(ReceivingSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0) {
		perror("Error");
	}

	getpeername(ReceivingSocket, (SOCKADDR *)&SenderAddr, &SenderAddrSize);

	int timeOutCounter = 0;
	BytesReceived = 1;
	sendto(ReceivingSocket, "req", sizeof("req"), 0, (struct sockaddr*) (SOCKADDR *)&SenderAddr, SenderAddrSize);
	string clientType = client->getClientType();
	bool switchedClient = false;
	bool timeOut = false;
	while (!timeOut) {
			if (client->getClientID() == 0) return 0;

			time_t cTime;
			time(&cTime);
			int seconds = difftime(cTime, client->pingTime);

			if (seconds >= 5) timeOut = true;

			if (switchedClient==false && clientType == "observer" && client->getClientType() == "player" ) { sendto(ReceivingSocket, "swi", sizeof("swi"), 0, (struct sockaddr*) (SOCKADDR *)&SenderAddr, SenderAddrSize); switchedClient = true;
			}
			else sendto(ReceivingSocket, client->getClientType().c_str(), sizeof("observer"), 0, (struct sockaddr*) (SOCKADDR *)&SenderAddr, SenderAddrSize);
			std::this_thread::sleep_for(200ms);


	}

	client->setDefunct(true);
	return 0;
}
void Socket::closeSocket(){
	// When your application is finished receiving datagrams close the socket.

	std::printf("Server: Finished receiving. Closing the listening socket...\n");
	if (closesocket(ReceivingSocket) != 0) printf("Server: closesocket() failed! Error code: %ld\n", WSAGetLastError());
	else printf("Server: closesocket() is OK...\n");
	// When your application is finished call WSACleanup.
	printf("Server: Cleaning up...\n");
	if (WSACleanup() != 0) printf("Server: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());
	else	printf("Server: WSACleanup() is OK\n");
}
void Socket::Send(char sendBuf[1024], SOCKADDR SenderAddr) {

	sendto(ReceivingSocket, sendBuf, sizeof(char[1024]), 0, (struct sockaddr*) (SOCKADDR *)&SenderAddr, sizeof(SenderAddr));
}
// If max clients has been reached send this message to the client attempting to connect.
void Socket::SendMaxClientsWarning() {
	sendto(ReceivingSocket, "No free slots, max clients reached.", sizeof(char[1024]), 0, (struct sockaddr*) (SOCKADDR *)&lastClientAddr, sizeof(lastClientAddr));
}
SOCKADDR Socket::getLastClientAddr()
{
	return lastClientAddr;
}