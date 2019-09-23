#ifndef COMMON_H
#define COMMON_H
// This is a Singleton class that I use to store and retreive methods that I commonly use in a variety of applications.
// This makes coding new applications more efficient as we are not rewriting too much code.
using namespace std;
class Common
{
public:
	static Common& getInstance()
	{
		static Common    instance; // Guaranteed to be destroyed.
								// Instantiated on first use.
		return instance;
	}
private:
	bool				 verbose;
	Common() {}                    // Constructor? (the {} brackets) are needed here.

								// C++ 03
								// ========
								// Dont forget to declare these two. You want to make sure they
								// are unacceptable otherwise you may accidentally get copies of
								// your singleton appearing.
	//Common(Common const&);              // Don't Implement
	//void operator=(Common const&); // Don't implement

								// C++ 11
								// =======
								// We can use the better technique of deleting the methods
								// we don't want.
public:
	bool isInt(/* in */ char *str,/* out */ int *num, int min, int max);
	void help();
	void printVersion();
	void setVerbosity(bool v);
	bool getVerbosity();
	void gameStatus(int playerCount, int MinPlayers, int clientCount, int observerCount);
	int readInteger(int min, int max);
	void writeGameConfig();
	//bool has_only_digits(string str);
	//Common(Common const&) = delete;
	//void operator=(Common const&) = delete;
	//       Deleted functions should generally
	//       be public as it results in better error messages
	//       due to the compilers behavior to check accessibility
	//       before deleted status
};
#endif