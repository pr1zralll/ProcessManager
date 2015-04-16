#pragma once
#include <string>
using namespace std;
class Log
{
public:

	static void setPath(string path);
	static string getPath();

	static void warning(string message);
	static void info(string message);
	static void error(string message);
	static void trace(string message);

	static void Clear();

private:

	static string path;
	static void Push(const char* type, const  char* message);
};

