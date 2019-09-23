// winsockudp3.cpp : Defines the entry point for the console application.
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define TRIGGERED 25
#include <winsock2.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <thread>
#include <future>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <cassert>
#include <atomic>
#include "main.h"
#include "socket.h"
#include "Common.h"
#include "Log.h"
// This function checks if the inputted string is all integers.
bool has_only_digits(string str) {
	return (str.find_first_not_of("0123456789") == string::npos);
}
void logHeartbeat(int i) { // i = index of client to be logged. This function logs the heartbeat of a client after a few seconds
	string log = "Heartbeat log: Client ID " + to_string(clientArray[i].getClientID()) + " is still connected.\n";
	Log::getInstance().logEvent(log, "events.log");
}
// This function checks how long the game server has been running.
void logTime() {
	time(&currentTime);
	int seconds = difftime(currentTime, gameStartTime);
	int minutes = seconds / 60;
	seconds = seconds % 60;
	if (PlayerCount>=MinPlayers) Log::getInstance().logEvent("Game server has been running for " + to_string(minutes) + " minutes and " + to_string(seconds) + " seconds.\n", "events.log");
}
void checkDefunct() { // This is the most complicated part of my code. In order to erase a client object without breaking the program we need to first kill all existing client threads then re-initiate them after the erase call.
					  // The re-initalisation will take into account all clients coming after the disconnected client move up by one place.
	for (int i = 1; i <= clientCount; i++) {
		if (clientArray[i].isDefunct() && clientArray[i].getClientType() != "") defunct = i;
	}

	if (defunct != 0) {
		if (defunct != clientCount) {
			for (int i = defunct + 1; i <= clientCount; i++) {
				if (i <= clientCount) {
					if (clientArray[i].getClientType() == "observer") observersExist = true;
					updateIDArray[i - 1] = clientArray[i].getClientID();
					clientArray[i].setClientID(0);
					clientArray[i].keepAlive.join();
				}
			}
		}
		numClientsLeft++;
		Log::getInstance().logEvent("Number of clients which left the game is " + to_string(numClientsLeft) + ".\n", "events.log");
		string clientType = clientArray[defunct].getClientType();
		//Log an entry to the command line to say a client has disconnected.
		string disconnectCause;
		if (clientArray[defunct].checkClientTriggeredDisconnect()) disconnectCause = "Client triggered disconnect.";
		else disconnectCause = "Client timed out.";
		cout << "Event: Client disconnected with ID: " << to_string(clientArray[defunct].getClientID()) << ". Cause: " << disconnectCause << endl;
		// Add a log to the events.log file to say a client has disconnected.
		string event = "Disconnect log: " + clientType + " disconnected with Client ID " + to_string(clientArray[defunct].getClientID()) + ". Cause: " + disconnectCause + "\n";
		Log::getInstance().logEvent(event, "events.log");
		logTime();
		clientArray[defunct].keepAlive.join();
		clientArray.erase(clientArray.begin() + defunct);
		clientID++;
		clientCount--;
		if (clientType == "player") {
			PlayerCount--; // And then tell the first available observer they are now a player/ in game.
			if (observersExist) { switchObserver = true; observersExist = false; }
		}
		else if (clientType == "observer") observerCount--;
		Common::getInstance().gameStatus(PlayerCount, MinPlayers, clientCount, observerCount);
		if (defunct != clientCount + 1) {
			for (int i = defunct; i <= clientCount; i++) {
				clientArray[i].setDefunct(false);
				clientArray[i].setClientID(updateIDArray[i]);
				clientArray[i].addReply();
				clientArray[i].runThreaded();

			}
		}
		defunct = 0;
	}
}
// This function checks if a new client has connected to the server.
void checkNewClient() {
	if (ReceiveBuf == "RequestID" && clientCount < 10) {
		string ClientType;
		if (PlayerCount < MaxPlayers) {
			ClientType = "player";
			PlayerCount++;
		}
		else {
			ClientType = "observer";
			observerCount++;
			Log::getInstance().logEvent("Number of clients which connected as spectators: " + to_string(observerCount) + ".\n", "events.log");
		};
		string event = "Connect log: " + ClientType + " connected with Client ID " + to_string(clientID) + "\n";
		Log::getInstance().logEvent(event, "events.log");
		clientID--;
		clientCount++;
		clientArray.insert(clientArray.begin() + clientCount, Client(ClientType, gameType, mapName, difficultyLevel));
		clientArray[clientCount] = Client(ClientType, gameType, mapName, difficultyLevel);
		clientArray[clientCount].sendData(biggestID + 1);
		clientArray[clientCount].runThreaded();
		// Restart or start the game start date if number of players = 2
		if (PlayerCount==MinPlayers) time(&gameStartTime);
		logTime();
		// These lines have to be commented out because printing too much text to the command line can slow down the program too much. Can be uncommented to show more functionality but less clients can connect. This isn't my fault, Windows just sucks. We still have file logging though!
		//cout << "Event: Client connected with ID: " << biggestID + 1 << endl;
		//Common::getInstance().gameStatus(PlayerCount, MinPlayers, clientCount, observerCount);
	}
}
// This function handles pings from the client.
void checkClientPing() {
	if (has_only_digits(ReceiveBuf)) {
		int pingID = stoi(ReceiveBuf);
		if (pingID > 0 && pingID < 25) {
			if (biggestID == 0) biggestID = clientArray[1].getClientID();
			for (int i = 1; i < 11; i++) {
				if ((stoi(ReceiveBuf) == clientArray.at(i).clientID)) {
					if (clientArray[i].getClientType() == "observer" && switchObserver == true && clientArray[i-1].getClientType() == "player") {
						clientArray[i].setClientType("player");
						PlayerCount++; observerCount--;
						if (PlayerCount == MinPlayers) time(&gameStartTime);
						cout << "Event: Client ID " << clientArray[i].getClientID() << " has switched from observer to player." << endl;
						switchObserver = false;
						Log::getInstance().logEvent("Client ID " + to_string(clientArray[i].getClientID()) + " has switched from observer to player.\n", "events.log");
						Common::getInstance().gameStatus(PlayerCount, MinPlayers, clientCount, observerCount);
					}
					// Check to see if we need to log the heartbeat for this client.
					heartbeatTriggerLog[i]++;
					if (heartbeatTriggerLog[i] == TRIGGERED) logHeartbeat(i);
					// Check to see which client ID is the biggest for this loop.
					if (clientArray[i].getClientID() > biggestID) biggestID = clientArray[i].getClientID();
					clientArray.at(i).addReply();
				}
			}
		}
	}
}
// This function reacts to a disconnect request from the client.
void checkClientTriggeredDisconnect() {
	if (ReceiveBuf.substr(0, 4) == "quit") {
		ReceiveBuf.erase(0, 4);
		int ID = stoi(ReceiveBuf);

		cout << "Disconnect triggered from client ID " << ID << endl;
		for (int i = 1; i < 11; i++) if (ID == clientArray[i].clientID) {
			clientArray[i].setDefunct(true);
			clientArray[i].setClientTriggeredDisconnect(true);
		}
	}
}
void configureGame() { // This function configures the game.
	cout << "Welcome to The Game Server Process by Oliver Church\n\n";
	cout << "---- Game Types ----\n\n";
	cout << "1. Deathmatch\n";
	cout << "2. Capture the Flag\n";
	cout << "3. Blood Diamond\n\n";
	cout << "Choose game type: ";
	cin >> input;
	while (!Common::getInstance().isInt(input, &gameTypeInput, 1, 3)) {
		cout << "Must be an integer between 1 and 3, please write again: ";  cin >> input;
	}
	if (gameTypeInput == 1) gameType = "Deathmatch";
	else if (gameTypeInput == 2) gameType = "Capture the Flag";
	else if (gameTypeInput == 3) gameType = "Blood Diamond";
	cout << "Input map name: ";
	cin >> mapName;
	cout << "Input difficulty level as an integer between 1 and 100: ";
	cin >> input;
	while (!Common::getInstance().isInt(input, &difficultyLevel, 1, 100)) {
		cout << "Must be an integer between 1 and 100, please write again: "; cin >> input;
	}
	cout << "\nNow you can define the minimum and maximum number of players you would like to allow to enter the game. Maximum clients including observers and players is limited to 10.\n\n";
	cout << "Specify the maximum number of players allowed in the game: ";
	cin >> input;
	while (!Common::getInstance().isInt(input, &MaxPlayers, 2, 10)) {
		cout << "Must be an integer between 2 and 10, please write again: "; cin >> input;
	}
	cout << "Specify the minimum number of players required before the game will start: ";
	cin >> input;
	while (!Common::getInstance().isInt(input, &MinPlayers, 2, MaxPlayers)) {
		cout << "Must be an integer between 2 and " << MaxPlayers << ", please write again: "; cin >> input;
	}
	cout << "The server is starting now..\n\n";
	cout << "Maximum players set to " << MaxPlayers << endl;
	cout << "Minimum players set to " << MinPlayers << endl;
	cout << "The server is starting now..\n\n";
	Common::getInstance().writeGameConfig();
	Log::getInstance().logEvent("Server is starting.\n", "events.log");
	Log::getInstance().logEvent("Game config: Game type is " + gameType + ", map name is " + mapName + ", difficulty level is " + to_string(difficultyLevel) + ", minimum players is set to " + to_string(MinPlayers) + ", maximum players is set to " + to_string(MaxPlayers) + ", server IP address is 127.0.0.1, server port is 27015.\n", "events.log");
}
void runGameServer() {
	int SelectTiming = Socket::getInstance().recvfromTimeOutUDP(10, 0);
	int y = 0;
	int x;
	SelectTiming = 1;
	while (1) {
		x = 0;
		switch (SelectTiming) {
		case 0:
			// Timed out, do whatever you want to handle this situation
			printf("Server: Timeout while waiting for client...\n");
			break;
		case -1:
			// Error occurred, maybe we should display an error message?
			printf("Server: Some error encountered with code number: %ld\n", WSAGetLastError());
			break;
		default:
		{
			ReceiveBuf = Socket::getInstance().acceptData();
			// This function checks if a client has been disconnected via ping timeout or client triggered disconnect.
			checkDefunct();
			checkNewClient();
			if (ReceiveBuf == "RequestID" && clientCount >= 10) Socket::getInstance().SendMaxClientsWarning();
			checkClientPing();
			checkClientTriggeredDisconnect();
			int BytesReceived = 0;
		}
		}

		y++;
		int SelectTiming = Socket::getInstance().recvfromTimeOutUDP(2, 0);
	}
	Socket::getInstance().closeSocket();
}
int main(int argc, char **argv)
	
{
	configureGame();
	runGameServer();
		// Back to the system
	return 0;

}

