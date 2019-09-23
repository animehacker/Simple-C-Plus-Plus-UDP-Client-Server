#ifndef WINSOCKUDP_H
#define WINSOCKUDP_H
#define MIN_REQUIRED 6
#include "client.h"
using namespace std;
struct wait_until_ready_t { 
	condition_variable cv; 
	mutex m; 
	bool ready; 
	wait_until_ready_t() : 
		ready(false) {} 
	void wait_until_ready() { std::unique_lock<std::mutex> lock(m); cv.wait(lock, [&]() { return ready; }); }
	void set_ready() { ready = true; cv.notify_all(); } 
}; 

string					mapName;
string					gameType;
int						difficultyLevel;
WSADATA					wsaData;
std::atomic<size_t> clientPingCounts[24] = {};
string					ReceiveBuf;
int						MinPlayers;
int						MaxPlayers = 10;
static bool			    verbose;
bool					switchObserver;
static int				inputs[3];
int						defunct = 0;
int						updateIDArray[10];
int						heartbeatTriggerLog[10];
bool					observersExist = false;
static int				PlayerCount = 0;
int						clientID = 24;
int						clientCount = 0;
int						observerCount = 0;
int						biggestID = 0;
int						gameTypeInput;
int						numClientsLeft = 0;
string					gameStatus;
char					input[255];
time_t					gameStartTime;
time_t					currentTime;
// Vector for input values to be stored in
std::vector<Client> clientArray(20); // Each Connected Client needs 2 slosts in the Vector array because of the reshuffle when a client gets disconnected.

#endif