#pragma once
#ifndef INJECTOR_H
#define INJECTOR_H

#include <iostream>
#include <Windows.h>
#include <string>
#include <fstream>
#include <TlHelp32.h>

using namespace std;

typedef NTSTATUS(NTAPI* NtCreateThreadExFunc)(
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN PVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN PVOID lpStartAddress,
	IN PVOID lpParameter,
	IN ULONG Flags,
	IN SIZE_T StackZeroBits,
	IN SIZE_T SizeOfStackCommit,
	IN SIZE_T SizeOfStackReserve,
	OUT PVOID lpBytesBuffer
	);

enum class InjectMethod {
	CreateRemoteThread,
	NtCreateThreadEx,
	ManualMap
};

class Injector {
public:
	Injector();
	~Injector();

	virtual bool inject(DWORD processId, const char* dllPath);
	// Uses the CreateRemoteThread method from the windows.h standard library.

	virtual bool injectNt(DWORD processId, const char* dllPath);
	// Uses the undocumented NtCreateThreadEx method.

	virtual bool injectManualMap(DWORD processId, const char* dllPath);
	// Not implemented.

	bool checkDllFile(const char* dllPath);
	// Checks dll file for validity. Simplified version.

private:
	bool writeDllPath(const char* dllPath) {


		return false;
	}

};

#endif //INJECTOR_H