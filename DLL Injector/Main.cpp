#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

std::string GetProcessName()
{
	std::string sProcessName{};

	std::cout << "Please enter the target process name:" << std::endl;
	std::cin >> sProcessName;

	if (!strstr(sProcessName.c_str(), ".exe"))
	{
		sProcessName += ".exe";
	}
	return sProcessName;
}

std::string GetDLLName()
{
	std::string sDLLName{};

	std::cout << "Please enter the dll name to be injected:" << std::endl;
	std::cin >> sDLLName;

	if (!strstr(sDLLName.c_str(), ".dll"))
	{
		sDLLName += ".dll";
	}
	return sDLLName;
}

DWORD GetProcessId(const char* cProcessName)
{
	DWORD dwProcessId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap && hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry))
		{
			do
			{
				if (!_stricmp(procEntry.szExeFile, cProcessName))
				{
					dwProcessId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
		CloseHandle(hSnap);
	}
	return dwProcessId;
}

void BypassTrusted(HANDLE hProcess)
{
	HMODULE hModule = LoadLibraryA("ntdll");

	if (hModule)
	{
		LPVOID lpBaseAddress = GetProcAddress(hModule, "NtOpenFile");

		if (lpBaseAddress)
		{
			LPCVOID lpBuffer[5];
			CopyMemory(lpBuffer, lpBaseAddress, 5);
			WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, 5, 0);
		}
	}
}

int main()
{
	std::string sProcessName = GetProcessName();
	std::string sDLLName = GetDLLName();

	char cDLLPath[MAX_PATH];
	GetFullPathName(sDLLName.c_str(), MAX_PATH, cDLLPath, 0);

	DWORD dwProcessId = 0;

	while (!dwProcessId)
	{
		dwProcessId = GetProcessId(sProcessName.c_str());
		Sleep(50);
	}

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwProcessId);

	if (hProcess && hProcess != INVALID_HANDLE_VALUE)
	{
		if (!strcmp(sProcessName.c_str(), "csgo.exe"))
		{
			BypassTrusted(hProcess);
		}

		LPVOID lpBaseAddress = VirtualAllocEx(hProcess, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (lpBaseAddress)
		{
			if (WriteProcessMemory(hProcess, lpBaseAddress, cDLLPath, strlen(cDLLPath) + 1, 0))
			{
				if (CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, lpBaseAddress, 0, 0))
				{
					std::cout << "Successfully injected!" << std::endl;

					Sleep(1000);

					CloseHandle(hProcess);
				}
			}
		}
	}
}