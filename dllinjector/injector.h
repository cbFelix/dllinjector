#pragma once
#ifndef INJECTOR_H
#define INJECTOR_H

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <fstream>
#include <vector>
#include <winternl.h>
#include <sstream>
#include <string>

#include "ntHeaders.h"

using namespace std;

namespace InjectorAPI {
	class Injector {
	public:
		Injector(DWORD processId);
		~Injector();

		void Inject(const char* dllPath);
		void InjectNt(const char* dllPath);
		void Eject(const char* moduleName);

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
		bool isValidDOSHeader();

		IMAGE_DOS_HEADER GetDOSHeader();


	private:
		string dllPath;

	};
}


#endif // INJECTOR_H
