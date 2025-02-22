#include <log.h>
#include <string>
#include <cstdarg>
#include <iostream>

void MessageLog::printf(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
	va_end(args);

	std::cout << buffer;

	char* start = buffer;
	char* ptr = buffer;

	for (;;) {
		char here = *ptr;
		if (here == '\n' || (here == 0 && ptr - start > 1)) {
			*ptr = 0;
			history.push_back(start);
			++unreadMessages;
			start = ptr + 1;
		}

		if (here == 0) {
			break;
		}
		else {
			++ptr;
		}
	}

	unreadMessages = min(10, unreadMessages);

}

MessageLog Log;