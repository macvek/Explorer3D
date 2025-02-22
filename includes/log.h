#pragma once

#include <string>
#include <list>
using namespace std;

struct MessageLog {
	list<string> history;
	char buffer[120] = {};
	int unreadMessages = 0;

	void printf(const char* format, ...);
};

extern MessageLog Log;

