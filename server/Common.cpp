#include "Common.h"
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;
// Check if an integer is valid and if not return false. If valid then assign the inputted value to a variable in the program.
bool Common::isInt(/* in */ char *str,/* out */ int *num, int min, int max) {
	char *endp;
	long n;
	n = strtol(str, &endp, 0);
	if (!*endp && !errno && min <= n && n <= max) {
		*num = (int)n;
		return true;
	}
	return false;
}
// Hard coded help page.
void Common::help() {
	printf("This is the help file for the game server.");
	exit(1);
}
// Print version information.
void Common::printVersion() {
	printf("Game server version 1.0\n");
	exit(1);
}
void Common::setVerbosity(bool v) {
	verbose = v;
}
bool Common::getVerbosity() {
	return Common::verbose;
}
void Common::gameStatus(int PlayerCount,int MinPlayers,int clientCount,int observerCount) {
	string gameStatus;
	if (PlayerCount > 10) PlayerCount = clientCount - observerCount;
	if (PlayerCount<=1) gameStatus = "Waiting for more clients to connect before the game can start.";
	else if (PlayerCount >= MinPlayers) gameStatus = "Game is running, minimum required players are connected.";
	else gameStatus = "Game is not running, minimum required players not reached.";
	cout << "Current game status: " << gameStatus << endl;
	cout << "Number of clients connected: " << clientCount << endl;
	cout << "Number of players connected: " << PlayerCount << endl;
	cout << "Number of observers connected: " << observerCount << endl;
}
int Common::readInteger(int min, int max) {
	int input;
	do {

		cout << "\nPlease enter a number from " << min << " to " << max << ":" << endl;
		cout << "-> ";

		if (!cin) {
			cout << "Invalid input. Please enter again: ";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}

	} while (!(cin >> input)); 
	return input;
}
void Common::writeGameConfig() {
	ofstream myfile;
	myfile.open("serverConfig.txt");
	myfile << "127.0.0.1:27015";
	myfile.close();
}