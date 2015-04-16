#include "tools.h"
#include "Log.h"
int get_cmd_line(DWORD dwId, PWSTR &buf)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwId);
	DWORD err = 0;
	if (hProcess == NULL)
	{
		Log::error("OpenProcess failed " + dwId);
		return GetLastError();
	}

	// determine if 64 or 32-bit processor
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	// determine if this process is running on WOW64
	BOOL wow;
	IsWow64Process(GetCurrentProcess(), &wow);

	// use WinDbg "dt ntdll!_PEB" command and search for ProcessParameters offset to find the truth out
	DWORD ProcessParametersOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x20 : 0x10;
	DWORD CommandLineOffset = si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? 0x70 : 0x40;

	// read basic info to get ProcessParameters address, we only need the beginning of PEB
	DWORD pebSize = ProcessParametersOffset + 8;
	PBYTE peb = (PBYTE)malloc(pebSize);
	ZeroMemory(peb, pebSize);

	// read basic info to get CommandLine address, we only need the beginning of ProcessParameters
	DWORD ppSize = CommandLineOffset + 16;
	PBYTE pp = (PBYTE)malloc(ppSize);
	ZeroMemory(peb, pebSize);

	PWSTR cmdLine;

	if (wow)
	{
		// we're running as a 32-bit process in a 64-bit OS
		PROCESS_BASIC_INFORMATION_WOW64 pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// get process information from 64-bit world
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64QueryInformationProcess64");
		err = query(hProcess, 0, &pbi, sizeof(pbi), NULL);
		if (err != 0)
		{
			Log::error("NtWow64QueryInformationProcess64 failed");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read PEB from 64-bit address space
		_NtWow64ReadVirtualMemory64 read = (_NtWow64ReadVirtualMemory64)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWow64ReadVirtualMemory64");
		err = read(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL);
		if (err != 0)
		{
			Log::error("NtWow64ReadVirtualMemory64 PEB failed");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read ProcessParameters from 64-bit address space
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space
		err = read(hProcess, parameters, pp, ppSize, NULL);
		if (err != 0)
		{
			Log::error("NtWow64ReadVirtualMemory64 Parameters failed");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read CommandLine
		UNICODE_STRING_WOW64* pCommandLine = (UNICODE_STRING_WOW64*)(pp + CommandLineOffset);
		cmdLine = (PWSTR)malloc(pCommandLine->MaximumLength);
		err = read(hProcess, pCommandLine->Buffer, cmdLine, pCommandLine->MaximumLength, NULL);
		if (err != 0)
		{
			Log::error("NtWow64ReadVirtualMemory64 Parameters failed");
			CloseHandle(hProcess);
			return GetLastError();
		}
	}
	else
	{
		// we're running as a 32-bit process in a 32-bit OS, or as a 64-bit process in a 64-bit OS
		PROCESS_BASIC_INFORMATION pbi;
		ZeroMemory(&pbi, sizeof(pbi));

		// get process information
		_NtQueryInformationProcess query = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
		err = query(hProcess, 0, &pbi, sizeof(pbi), NULL);
		if (!err)
		{
			Log::error("NtQueryInformationProcess failed");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read PEB
		if (!ReadProcessMemory(hProcess, pbi.PebBaseAddress, peb, pebSize, NULL))
		{
			Log::error("ReadProcessMemory PEB failed");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read ProcessParameters
		PBYTE* parameters = (PBYTE*)*(LPVOID*)(peb + ProcessParametersOffset); // address in remote process adress space
		if (!ReadProcessMemory(hProcess, parameters, pp, ppSize, NULL))
		{
			Log::error("ReadProcessMemory Parameters failed");
			CloseHandle(hProcess);
			return GetLastError();
		}

		// read CommandLine
		UNICODE_STRING* pCommandLine = (UNICODE_STRING*)(pp + CommandLineOffset);
		cmdLine = (PWSTR)malloc(pCommandLine->MaximumLength);
		if (!ReadProcessMemory(hProcess, pCommandLine->Buffer, cmdLine, pCommandLine->MaximumLength, NULL))
		{
			Log::error("ReadProcessMemory Parameters failed");
			CloseHandle(hProcess);
			return GetLastError();
		}
	}

	buf = cmdLine;
}