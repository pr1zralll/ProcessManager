
#include <Windows.h>
#include "Log.h"
#pragma once
class Process
{
public:
	Process(TCHAR* name, TCHAR* args);
	Process(int id);
	~Process();
		
    void start();
	void restart();
	void stop();
	DWORD getState();
	int getID();
	HANDLE getHandle();

private: 
	void run();
	STARTUPINFO s_info;
	PROCESS_INFORMATION p_info;
	HANDLE thread;
	TCHAR name[MAX_PATH];
	TCHAR args[MAX_PATH];
	bool running = true;
	static DWORD myFunctionCaller(LPVOID* param);
protected:
	virtual void OnStart();
	virtual void OnCrash();
	virtual void OnStop();
	virtual void OnWork();
};

