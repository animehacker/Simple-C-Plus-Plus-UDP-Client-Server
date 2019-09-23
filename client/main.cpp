#include <winsock2.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <signal.h>
#include <fstream>
#include <thread>
using namespace std;
struct gameData {
	int clientID;
	int difficultyLevel;
	string mapName;
	string gameType;
	string clientType;
};
SOCKET               SendingSocket;
SOCKADDR_IN          ReceiverAddr, SrcInfo;
gameData			 game;
boolean disconnected = false;
HWND name;
// This function handles a user input to disconnect the client.
void  listenInput() {
	while (1) {
		// Check if escape key has been hit while the window is active
		if (GetAsyncKeyState(VK_ESCAPE) && name == GetForegroundWindow())
		{
			string quit = "quit" + to_string(game.clientID);
			string confirm;
			cout << "Are you sure you want to disconnect from the server? (y/n): ";
			cin >> confirm;
			if (confirm == "y") {
				sendto(SendingSocket, (char *)quit.c_str(), sizeof(quit), 0, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));
				cout << "Disconnected from the server." << endl;
				disconnected = true;
				break;
			}
		}
	}

}
gameData decodeData(std::string s, gameData *g) {
	string delimiter = ",";
	size_t pos = 0;
	string token;
	int tokenCount = 0;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		tokenCount++;
		token = s.substr(0, pos);
		if (tokenCount == 1) g->clientID = stoi(token);
		if (tokenCount == 2) g->gameType = token;
		if (tokenCount == 3) g->mapName = token;
		if (tokenCount == 4) g->difficultyLevel = stoi(token);
		s.erase(0, pos + delimiter.length());
	}
	g->clientType = s;
	return *g;
}
void connectServer() {
	WSADATA              wsaData;
	char                 *SendBuf = "";
	string				 recvData;
	int					 recBufLen = 1024;
	char				 recBuf[1024];
	int                  BufLength = 1024;
	int len;
	int TotalByteSent;
	// Initialize Winsock version 2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Client: WSAStartup failed with error %ld\n", WSAGetLastError());
		// Clean up
		WSACleanup();
		// Exit 
		return;
	}
	// Create a new socket to receive datagrams on.
	SendingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SendingSocket == INVALID_SOCKET)
	{
		printf("Client: Error at socket(): %ld\n", WSAGetLastError());
		// Clean up
		WSACleanup();
		// Exit 
		return;
	}

	// Get the address and port number from config file
	std::ifstream file("serverConfig.txt");
	std::string config;
	if (!file.is_open()) {
		cout << "Could not open server configuration file. Make sure the server is running. Returning to user prompt." << endl;
		return;
	}
	std::getline(file, config);
	size_t pos = 0;
	string delimiter = ":";
	pos = config.find(delimiter);
	string address = config.substr(0, pos);
	config.erase(0, pos + delimiter.length());
	string port = config;
	// Set up a SOCKADDR_IN structure that will identify who we
	// will send datagrams to. For demonstration purposes, let's
	// assume our receiver's IP address is 127.0.0.1 and waiting
	// for datagrams on port 5150.
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(stoi(port));
	ReceiverAddr.sin_addr.s_addr = inet_addr(address.c_str());

	TotalByteSent = sendto(SendingSocket, (char *)"RequestID", sizeof("RequestID"), 0,
		(SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));

	memset(&SrcInfo, 0, sizeof(SrcInfo));
	len = sizeof(SrcInfo);

	getsockname(SendingSocket, (SOCKADDR *)&SrcInfo, &len);


	// Some info on the sender side
	getpeername(SendingSocket, (SOCKADDR *)&ReceiverAddr, (int *)sizeof(ReceiverAddr));

	char ReceivedBuf[1024];

	if (recvfrom(SendingSocket, (char *)ReceivedBuf, sizeof(char[1024]), 0, (struct sockaddr *) &SrcInfo, &len) == -1)
	{
		printf("Not able to establish connection. Make sure the server is running before connecting.\n"); // This block of code will not run unless the server is not up, it's just here for debugging purposes.
	}
	else { // This block of code processes the data received from the server.
		recvData = ReceivedBuf;
		while (recvData == "req") {
			cout << "Sending ID request again." << endl;
			sendto(SendingSocket, (char *)"RequestID", sizeof("RequestID"), 0, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));
			recvfrom(SendingSocket, (char *)ReceivedBuf, sizeof(char[1024]), 0, (struct sockaddr *) &SrcInfo, &len);
			recvData = ReceivedBuf;
		}
		if (recvData != "No free slots, max clients reached.") {
			decodeData(recvData, &game);
			printf("Client connected to gameserver IP: %s\n", inet_ntoa(SrcInfo.sin_addr));
			cout << "Successfully connected to the server. Press the escape key to disconnect from the server. Game data shown below. " << endl;
			cout << endl;
			cout << "Client type: " << "\t\t" << game.clientType << endl;
			cout << "Game type: " << "\t\t" << game.gameType << endl;
			cout << "Map name: " << "\t\t" << game.mapName << endl;
			cout << "Difficulty level: " << "\t" << game.difficultyLevel << endl;
			std::cout << "Client ID: " << "\t\t" << game.clientID << "\n";
			// This function runs in it's own thread to listen for a user hitting the escape key. If a user hits the escape key they are prompted for whether they want to disconnect from the server.
			std::thread t1(listenInput);
			// This while loop handles sending and receiving pings to and from the server.
			while (1) {
				recvfrom(SendingSocket, (char *)ReceivedBuf, sizeof("observer"), 0, (struct sockaddr *) &SrcInfo, &len);

				if (string(ReceivedBuf) == "player" && game.clientType == "observer") {
					cout << "You have switched from being an observer to being a player." << endl; game.clientType = "player";
				}
				sendto(SendingSocket, (char *)to_string(game.clientID).c_str(), sizeof("ping"), 0, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));
				if (disconnected) { t1.join();  break; } // If the listenInput function triggers a disconnect call, rejoin it's running thread back to the Main thread and stop pinging the server.
			}
			// Make sure we reset the disconnected flag to it's default value.
			disconnected = false;

		}
		// If the client is unable to disconnect this will run. For example the maximum number of clients allowed to connect to the server is 10. This will run if an 11th client tries to connect.
		else cout << "Client unable to connect. Reason given by server is: " << recvData << endl;
	}
	// When your application is finished receiving datagrams close the socket.
	if (closesocket(SendingSocket) != 0)
		printf("Client: closesocket() failed! Error code: %ld\n", WSAGetLastError());

	if (WSACleanup() != 0)
		printf("Client: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());
}
int main(int argc, char **argv)
{
	name = GetForegroundWindow();
	string input = "y";
	while (input == "y") {
		// This prompt allows a user to decide whether to connect to the server or leave the program.
		cout << "Press the y key and enter to connect to the server or any other key and enter to exit the program: " << endl;
		cin >> input;
		if (input == "y") {
			cout << "Connecting to the server.." << endl;
			connectServer();
		}
	}

	return 0;
}