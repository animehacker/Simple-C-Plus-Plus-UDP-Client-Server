#include <iostream>
#include <iomanip>
#include "Client.h"
#include "Socket.h"
using namespace std;
Client::Client() {} // Default constructor
Client::Client(std::string t, ::string g, std::string m, int d) // g = gameType, m = mapName, d = difficultyLevel
{
	difficultyLevel = d;
	gameType = g;
	mapName = m;
	clientType = t;
	ClientAddr = Socket::getInstance().lastClientAddr;
	//replyFlag.fetch_xor(0);
	//replyCount = 0;
	//printf("Fetched last Client Addr\n");
	//SOCKADDR *tempsock = &ClientAddr;
	SOCKADDR_IN *temp = (struct sockaddr_in *)&ClientAddr;
	char *ip = inet_ntoa(temp->sin_addr);
	//printf("\nSocket Address stored in Client class object is: %s\n", ip);
}

/*
void Client::swap(Client & c) throw()
{
} */
/*
Client::Client(const Client & origin)
{
}  */ 
/*
void Client::swap(Client & c) throw()
{
	std::swap(*this, c);
} */
/*
Client::Client(const Client & origin)
{
	
} */
/*
char* Client::getConfig()
{
	char* c;
	c = (char *)malloc(1024 * sizeof(char));
	gameCfg* gc = &config;
	c = (char *)gc;
	return c;
}
*/

void Client::sendData(int id) {
	string sendBuf = to_string(id) + "," + gameType + "," + mapName + "," + to_string(difficultyLevel) + "," + clientType;
	Socket::getInstance().Send((char *)sendBuf.c_str(),ClientAddr);
	clientID = id;
}
void Client::keepClientAlive(){
	Socket::getInstance().keepAlive(ClientAddr, clientID, this);
}
void Client::runThreaded() {
	std::thread t(&Client::keepClientAlive, this);
	t.swap(keepAlive);
}
bool Client::isDefunct() {
	return defunct;
}
void Client::setDefunct(bool set) {
	defunct = set;
}
std::string Client::getClientType() {
	return clientType;
}
void Client::setClientType(string c) {
	clientType = c;
}
void Client::setClientID(int id) {
	clientID = id;
}
int Client::getClientID() {
	return clientID;
}
void Client::addReply() { // r == Main is updating the client object to announce a ping request has been received from the client
	counter++;
	time(&pingTime);
	test = pingTime;
}
void Client::setClientTriggeredDisconnect(bool set) {
	clientTriggeredDisconnect = set;
}
bool Client::checkClientTriggeredDisconnect() {
	return clientTriggeredDisconnect;
}
