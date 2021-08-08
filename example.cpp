#include "liveterm.h"

#include <string>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <errno.h>
#include <cstring>

using namespace std;

bool exitFlag = false;
mutex exitFlagMutex;

void myCommander(string a) {
	if(a == "exit") {
		const std::lock_guard<std::mutex> lock(exitFlagMutex);
		exitFlag = true;
		LTPrintf("Exiting...");
	} else {
		LTPrintf("Unknown command [%s], use `exit` to exit.", a.c_str());
	}
	return;
}

void mySpammer() {
	int counter = 0;
	while(1) {
		{
			const std::lock_guard<std::mutex> lock(exitFlagMutex);
			if(exitFlag) {
				LTPrintf("Spammer exits...");
				return;
			}
		}
		this_thread::sleep_for(std::chrono::seconds{rand()%3});
		LTPrintf("Spamming into terminal (%d)", counter);
		counter++;
	}
}

int main() {
	if(LivetermInit(&myCommander)) {
		LTPrintf("Startup error! (%d %s)", errno, strerror(errno));
		return 1;
	}
	
	thread spam(mySpammer);
	
	spam.join();
	
	if(LivetermShutdown()) {
		LTPrintf("Shutdown error! (%d %s)", errno, strerror(errno));
		return 1;
	}
	return 0;
}