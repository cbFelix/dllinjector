#pragma once
#ifndef INJECTOR_H
#define INJECTOR_H

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <vector>
#include <winternl.h>
#include <iterator>
#include <algorithm>

using namespace std;

namespace InjectorAPI {
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

	class Injector {
	public:
		Injector(DWORD processId);
		~Injector();

		void Inject(const char* dllPath);
		void Eject(HMODULE hModule);

	private:
		HANDLE hProcess;
		PROCESSENTRY32 processEntry;
	};

	DWORD FindProcessID(wstring processName);
	PROCESSENTRY32 FindProcess(DWORD processId);

	class DllValidator {
	public:
		DllValidator(const char* path);
		~DllValidator();

		string rawData();
		bool isValidDLL();
		IMAGE_DOS_HEADER GetDOSHeader(HANDLE hFile);

	private:
		string dllPath;

	};
}


#endif // INJECTOR_H
