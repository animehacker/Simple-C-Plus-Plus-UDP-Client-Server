#ifndef CLIENT_H
#define CLIENT_H
#include <winsock2.h>
#include <thread>
#include <string>
#include <atomic>
#include <mutex>
class Client
{
private:

	std::string clientType;
	std::string gameType;
	std::string mapName;
	int difficultyLevel;
	bool defunct = false;
	bool clientTriggeredDisconnect = false;
	SOCKADDR		   ClientAddr;
public:
	//with default value
	Client();
	//Client::~Client();
	Client(std::string t, std::string g, std::string m, int d); // t = clientType, g = gameType, m = mapeName, d = difficultyLevel
	std::thread keepAlive;
	int counter = 0;
	time_t pingTime;
	int test;
	int				   clientID;
	void sendData(int id);
	void keepClientAlive();
	void runThreaded();
	bool isDefunct();
	void setDefunct(bool set);
	std::string getClientType();
	void Client::setClientType(std::string c);
	void addReply();
	void Client::setClientID(int id);
	int Client::getClientID();
	void setClientTriggeredDisconnect(bool set);
	bool checkClientTriggeredDisconnect();
};
#endif