#ifndef SOCKET_H
#define SOCKET_H
#include <winsock2.h>
using namespace std;
class Socket
{
private:

	//with default value
	Socket(Socket const&) = delete;              // Don't Implement
	//void operator=(Socket const&); // Don't implement
	WSADATA            wsaData;
	SOCKET             ReceivingSocket;
	SOCKADDR_IN        ReceiverAddr;
	int                Port = 27015;
	struct gameCfg {
		char gameType[48];
		char mapName[48];
		int difficultyLevel;
	};

public:
	//struct wait_until_ready_t wait;
	static Socket& getInstance()
	{
		static Socket    instance; // Guaranteed to be destroyed.
								// Instantiated on first use.
		return instance;
	} 
	bool					ready = false;
	bool					processed = false;
	SOCKADDR			    lastClientAddr;
	// Connect
	Socket();
	std::string acceptData(); // struct wait_until_ready_t *wait
	// TimeOut check
	int recvfromTimeOutUDP(long sec, long usec);
	int keepAlive(SOCKADDR SenderAddr, int clientID, Client *client);
	// Close the socket
	void closeSocket();
	void Send(char sendBuf[1024], SOCKADDR SenderAddr);
	SOCKADDR getLastClientAddr();
	void SendMaxClientsWarning();
};
#endif
