#include "Process.h"
#include <Windows.h>
#include "Log.h"
#include <iostream>
#include <Psapi.h>
#include "tools.h"
#include <tchar.h>
using namespace std;

Process::Process(TCHAR* name, TCHAR* args)
{
	_tcscpy(this->name, name);
	_tcscpy(this->args,args);
	Log::info("init");
}
#include <sstream>
Process::Process(int id)
{
	Log::info("init");
	p_info.hProcess = OpenProcess( PROCESS_ALL_ACCESS, TRUE, id);
	if (p_info.hProcess != NULL)
	{
		p_info.dwProcessId = id;
		TCHAR Buffer[MAX_PATH];
		if (GetModuleFileNameEx(p_info.hProcess, 0, Buffer, MAX_PATH))
		{
			_tcscpy(this->name, Buffer);
			PWSTR buf;
			get_cmd_line(id, buf);
			_tcscpy(this->args, buf);
		}
		else
		{//log
			std::stringstream ss;
			ss << GetLastError();
			
			Log::error("can`t get file name");
			Log::error("last error " + ss.str());
		}
		//monitor
		running = true;
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myFunctionCaller, this, 0, NULL);
		SetThreadPriority(thread, THREAD_MODE_BACKGROUND_BEGIN);
	}
	else
	{
		Log::error("process not found");
	}
}

Process::~Process()
{
	running = false;
	CloseHandle(thread);
	Log::info("end");
}

void Process::run()
{
	while (running)
	{
		switch (getState())
		{
			case 0:// end work
			{
				OnStop();

				if (p_info.hProcess != NULL)
					TerminateProcess(p_info.hProcess, NO_ERROR);

				//*************
				OnStart();
				Log::info("starting process...");
				ZeroMemory(&s_info, sizeof(STARTUPINFO));
				if (CreateProcess(name, args, NULL, NULL, false, 0, NULL, NULL, &s_info, &p_info))
				{
					char procID[10];
					sprintf(procID, "%d", p_info.dwProcessId);
					string s1 = (const char*)procID;
					Log::info("process started " + s1);
				}
				else
					Log::error("can't create process");
				//*************

				break;
			}
			case 259://work
			{
				OnWork();
				break;
			}
			default:
			{
				OnCrash();
				restart();
			}
		}
	}
}
// to run non static class function
DWORD Process::myFunctionCaller(LPVOID* param)
{
	Process* myClass = (Process*)(param);
	myClass->run();
	return 0;
}
void Process::start()
{
	OnStart();
	Log::info("starting process...");
	ZeroMemory(&s_info, sizeof(STARTUPINFO));
	if (CreateProcess(name, args, NULL, NULL, false, 0, NULL, NULL, &s_info, &p_info))
	{
		char procID[10];
		sprintf(procID, "%d", p_info.dwProcessId);
		string s1 = (const char*)procID;
		Log::info("process started " + s1);
	}
	else
		Log::error("can't create process");

	//monitor process
	thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)myFunctionCaller, this, 0, NULL);
	SetThreadPriority(thread, THREAD_MODE_BACKGROUND_BEGIN);
}
void Process::restart()
{
	stop();
	start();
}
void Process::stop()
{
	OnStop();
	if (p_info.hProcess!=NULL)
		TerminateProcess(p_info.hProcess, NO_ERROR);
	running = false;
	WaitForSingleObject(thread,1000);
	if(thread!=NULL)
		CloseHandle(thread);
}
DWORD Process::getState()
{
	DWORD ExitCode;

	GetExitCodeProcess(p_info.hProcess, &ExitCode);

	return ExitCode;
}
int Process::getID()
{
	return p_info.dwProcessId;
}
HANDLE Process::getHandle()
{
	return p_info.hProcess;
}
void Process::OnStart()
{
	Log::trace("onStart");
}
void Process::OnCrash()
{
	Log::trace("onCrash");
}
void Process::OnStop()
{
	Log::trace("onStop");
}
void Process::OnWork()
{
	//Log::trace("onWork");
}

