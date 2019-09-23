// This is a class I wrote to deal with logging functionality. It has a general purpose set of functions that can be adapted for use in many applications
#include <string>
using namespace std;
class Log
{
public:
	static Log& getInstance()
	{
		static Log    instance; // Guaranteed to be destroyed.
							  // Instantiated on first use.
		return instance;
	}
private:
	Log() {}                    // Constructor

	Log(Log const&);              // Don't Implement
	void operator=(Log const&); // Don't implement

public:
	void logEvent(std::string event, char *fileName);
};