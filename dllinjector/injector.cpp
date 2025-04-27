#include "injector.h"

Injector::Injector() {

}
Injector::~Injector() {

}

bool Injector::checkDllFile(const char* dllPath) {
	ifstream iFile(dllPath);
	if (!iFile) {
		return false;
	}

	return true;
}

bool Injector::inject(DWORD processId, const char* dllPath) {

	HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (!hTargetProcess) {
		DWORD error = GetLastError();
		cerr << "Failed to get process handle. Error: " << error << endl;

		return false;
	}

	if (!checkDllFile(dllPath)) {
		cerr << "Failed to open DLL file." << endl;

		return false;
	}

	size_t nDllPath = strlen(dllPath) + 1;
	LPVOID allocMem = VirtualAllocEx(hTargetProcess, NULL, nDllPath, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!allocMem) {
		DWORD error = GetLastError();
		cerr << "Failed to allocate memory in process. Error: " << error << endl;

		CloseHandle(hTargetProcess);
		return false;
	}

	if (!WriteProcessMemory(hTargetProcess, allocMem, dllPath, nDllPath, NULL)) {
		DWORD error = GetLastError();
		cerr << "Failed to write path to dll in process. Error: " << error << endl;

		VirtualFreeEx(hTargetProcess, allocMem, 0, MEM_RELEASE);
		CloseHandle(hTargetProcess);
		return false;
	}

	LPVOID lpLoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	HANDLE hDLLThread = CreateRemoteThread(hTargetProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpLoadLibraryAddr, allocMem, NULL, NULL);
	if (!hDLLThread) {
		DWORD error = GetLastError();
		cerr << "Failed to create remote thread in process. Error: " << error << endl;

		VirtualFreeEx(hTargetProcess, allocMem, 0, MEM_RELEASE);
		CloseHandle(hDLLThread);
		CloseHandle(hTargetProcess);
		return false;
	}

	WaitForSingleObject(hDLLThread, INFINITE);

	VirtualFreeEx(hTargetProcess, allocMem, 0, MEM_RELEASE);

	return true;
}

bool Injector::injectNt(DWORD processId, const char* dllPath) {
	HANDLE hTargetProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (!hTargetProcess) {
		DWORD error = GetLastError();
		cerr << "Failed to get process handle. Error: " << error << endl;

		return false;
	}

	if (!checkDllFile(dllPath)) {
		cerr << "Failed to open DLL file." << endl;

		return false;
	}

	size_t nDllPath = strlen(dllPath) + 1;
	LPVOID allocMem = VirtualAllocEx(hTargetProcess, NULL, nDllPath, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!allocMem) {
		DWORD error = GetLastError();
		cerr << "Failed to allocate memory in process. Error: " << error << endl;

		CloseHandle(hTargetProcess);
		return false;
	}

	if (!WriteProcessMemory(hTargetProcess, allocMem, dllPath, nDllPath, NULL)) {
		DWORD error = GetLastError();
		cerr << "Failed to write path to dll in process. Error: " << error << endl;

		VirtualFreeEx(hTargetProcess, allocMem, 0, MEM_RELEASE);
		CloseHandle(hTargetProcess);
		return false;
	}

	LPVOID lpLoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	NtCreateThreadExFunc NtCreateThreadEx = (NtCreateThreadExFunc)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
	HANDLE hDLLThread = NULL;
	NTSTATUS status = NtCreateThreadEx(&hDLLThread, THREAD_ALL_ACCESS, NULL, hTargetProcess, (LPTHREAD_START_ROUTINE)lpLoadLibraryAddr, allocMem, 0x4, 0, 0, 0, 0);
	if (!hDLLThread) {
		DWORD error = GetLastError();
		cerr << "Failed to create remote hidden thread in process. Error: " << error << endl;

		VirtualFreeEx(hTargetProcess, allocMem, 0, MEM_RELEASE);
		CloseHandle(hTargetProcess);
		return false;
	}

	WaitForSingleObject(hDLLThread, INFINITE);

	VirtualFreeEx(hTargetProcess, allocMem, 0, MEM_RELEASE);

	return true;
}

bool Injector::injectManualMap(DWORD processId, const char* dllPath) {
	return true;
}