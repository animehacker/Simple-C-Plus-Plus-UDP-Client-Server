#include "log.h"
#include "Common.h"
#include <stdio.h>
#include <string>
void Log::operator=(Log const &)
{
}
// Function to log events.
void Log::logEvent(std::string event, char *fileName) {
	FILE *f = fopen(fileName, "a+");
	if (f == NULL) printf("Error opening log file! Have you checked permissions?\n");
	else fprintf(f, event.c_str());
	if (true == Common::getInstance().getVerbosity()) printf("Log: %s\n", event);
	fclose(f);
}
