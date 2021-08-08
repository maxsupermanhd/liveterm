#include "liveterm.h"

#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <thread>
#include <mutex>
#include <errno.h>

using namespace std;

struct termios LTAttr;

static string LTDefaultPromt = "[~>";

static string LTPromt = LTDefaultPromt;
static mutex LTPromtMutex;
static string LTBuffer = "";

static void (*LTCommander)(string) = NULL;
static mutex LTCommanderMutex;

static thread LTReaderT;
static mutex LTIOMutex;

static int LTReaderControl = 0;
static mutex LTReaderControlMutex;

static bool LTInitDone = false;
static int LTTermWidth = 0;
static int LTTermUsed = 0;


int LTGetReaderControl() {
	int r = -1;
	const std::lock_guard<std::mutex> lock(LTReaderControlMutex);
	r = LTReaderControl;
	return r;
}

void LTSetReaderControl(int s) {
	const std::lock_guard<std::mutex> lock(LTReaderControlMutex);
	LTReaderControl = s;
	return;
}

void LivetermSetCommander(void (*Commander)(std::string cmd)) {
	const std::lock_guard<std::mutex> lock(LTCommanderMutex);
	LTCommander = Commander;
	return;
}

void LivetermSetPromt(std::string p) {
	const std::lock_guard<std::mutex> lock(LTPromtMutex);
	LTPromt = p;
	return;
}

bool LTTermSize() {
	struct winsize size;
	if(ioctl(0, TIOCSWINSZ, (char *)&size)) {
		return true;
	}
	LTTermWidth = size.ws_row;
	return false;
}

void LTPromtUpdate() {
	const std::lock_guard<std::mutex> lock(LTIOMutex);
	printf("\r%*s\r", LTTermUsed+(int)LTPromt.length(), "");
	printf("%s%s", LTPromt.c_str(), LTBuffer.c_str());
	return;
}
void LTPromtUpdateNM() {
	printf("\r%*s\r", LTTermUsed+(int)LTPromt.length(), "");
	printf("%s%s", LTPromt.c_str(), LTBuffer.c_str());
	return;
}

void LTPrintf(const char* format, ...) {
    if(LTInitDone) {
    	const std::lock_guard<std::mutex> lock(LTIOMutex);
        printf("\r%*s\r", (int)LTBuffer.length()+(int)LTPromt.length(), "");
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n%s%s", LTPromt.c_str(), LTBuffer.c_str());
    } else {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
    return;
}

void LTReader() {
    while(1) {
        switch(LTGetReaderControl()) {
            case 0:
            {
                char c;
                if(read(STDIN_FILENO, &c, 1) == 1) {
                    switch(c) {
                        case '\n':
                        {
                        	const std::lock_guard<std::mutex> lock(LTIOMutex);
                        	string cmd = LTBuffer;
                        	LTBuffer = "";
                            LTPromtUpdateNM();
                            if(LTCommander != NULL) {
                            	thread c(LTCommander, cmd);
                            }
                        }
                        break;
                        case 127:
                        {
                        	const std::lock_guard<std::mutex> lock(LTIOMutex);
	                        if(LTBuffer != "") {
	                        	LTBuffer.pop_back();
	                            printf("\b \b");
	                        }
                        }
                        break;
                        default:
                        {
                        	const std::lock_guard<std::mutex> lock(LTIOMutex);
                        	LTBuffer += c;
	                        printf("%c", c);
	                    }
                        break;
                    }
                }
            }
            break;
            case -1:
            return;
            break;
        }
    }
    return;
}

bool LivetermInit(void (*Commander)(string)) {
    LivetermSetCommander(Commander);
    setbuf(stdout, 0);
    setbuf(stdin, 0);
    struct termios tattr;
    if(!isatty(STDIN_FILENO)) {
        return 1;
    }
    if(tcgetattr(STDIN_FILENO, &LTAttr)) {
        return 1;
    }
    if(tcgetattr(STDIN_FILENO, &tattr)) {
        return 1;
    }
    tattr.c_lflag &= ~(ICANON|ECHO|ISIG);
    tattr.c_cc[VMIN] = 0;
    tattr.c_cc[VTIME] = 1;
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr)) {
        return 1;
    }
    LTPromtUpdate();
    // thread newreader(LTReader);
    LTReaderT = thread(LTReader);
	// LTReaderT.detach();
    LTInitDone = true;
    return 0;
}

bool LivetermShutdown() {
    if(LTInitDone) {
        LTPrintf("CT shutdown.\n");
        LTSetReaderControl(-1);
        LTReaderT.join();
        if(tcsetattr(STDIN_FILENO, TCSANOW, &LTAttr)) {
            return 1;
        }
        printf("\n");
        LTInitDone = false;
    }
    return 0;
}











