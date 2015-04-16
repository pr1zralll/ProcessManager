#include "Log.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <time.h>

using namespace std;

string Log::path = "program.log";

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
	return buf;
}

void Log::Push(const char*  type, const  char* message)
{
	ofstream file;
	file.open(path, ios::app|ios::out);
	file << currentDateTime() + " " + type + " " + message << endl;
	file.close();
}
void Log::Clear()
{
	remove(path.c_str());
}

void Log::warning(string message)
{
	Push("warning", message.c_str());
}
void Log::info(string message)
{
	Push("info   ", message.c_str());
}
void Log::error(string message)
{
	Push("error  ", message.c_str());
}
void Log::trace(string message)
{
	Push("trace  ", message.c_str());
}

void Log::setPath(string path)
{
	Log::path = path;
}


string Log::getPath()
{
	return path;
}
