#include "injector.h"

InjectorAPI::Injector::Injector(DWORD processId) {
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (!hProcess) {
		throw runtime_error("OpenProcess failed. Error: " + to_string(GetLastError()));
	}

	processEntry = FindProcess(processId);
}
InjectorAPI::Injector::~Injector() {
	if (hProcess != INVALID_HANDLE_VALUE) {
		CloseHandle(hProcess);
	}
}

void InjectorAPI::Injector::Inject(const char* dllPath) {
	if (!hProcess) {
		throw runtime_error("The process is no longer available.");
	}

	DllValidator dllValidator(dllPath);
	if (!dllValidator.isValidDLL()) {
		throw runtime_error("Invalid DLL.");
	}

	SIZE_T nDllPath = strlen(dllPath) + 1;
	cout << "Size: " << nDllPath << endl;

	LPVOID allocMem = VirtualAllocEx(hProcess, nullptr, nDllPath, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!allocMem) {
		throw runtime_error("VirtualAllocEx failed. Error: " + GetLastError());
	}

	size_t bytesWrite;
	if (!WriteProcessMemory(hProcess, allocMem, dllPath, nDllPath, &bytesWrite) || bytesWrite != nDllPath) {
		VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
		throw runtime_error("WriteProcessMemory failed. Error: " + GetLastError());
	}

	HMODULE krnl32m = GetModuleHandleA("kernel32.dll");
	if (krnl32m == 0) {
		VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
		throw runtime_error("Failed to get kernel32.dll module handle. Error: " + GetLastError());
	}
	LPVOID lpLoadLibraryAddr = (LPVOID)GetProcAddress(krnl32m, "LoadLibraryA");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpLoadLibraryAddr, allocMem, 0, NULL);
	if (!hThread) {
		VirtualFreeEx(hProcess, allocMem, 0, MEM_RELEASE);
		throw runtime_error("Failed to create remote thread in target process. Error: " + GetLastError());
	}
}

void InjectorAPI::Injector::InjectNt(const char* dllPath) {
	if (!hProcess) {
		throw runtime_error("The process is no longer available.");
	}

	DllValidator dllValidator(dllPath);
	if (!dllValidator.isValidDLL()) {
		throw runtime_error("Invalid DLL.");
	}

	HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
	if (!hNtdll) {
		throw runtime_error("Failed to get ntdll handle. Error 0x" + to_string(GetLastError()));
	}

	auto NtAllocateVirtualMemory = (pNtAllocateVirtualMemory)GetProcAddress(hNtdll, "NtAllocateVirtualMemory");
	if (!NtAllocateVirtualMemory) {
		throw runtime_error("Failed to get address of NtAllocateVirtualMemory.");
	}
	auto NtFreeVirtualMemory = (pNtFreeVirtualMemory)GetProcAddress(hNtdll, "NtFreeVirtualMemory");
	if (!NtAllocateVirtualMemory) {
		throw runtime_error("Failed to get address of NtFreeVirtualMemory.");
	}

	SIZE_T nDllSize = strlen(dllPath) + 1;
	SIZE_T RegionSize = nDllSize;

	PVOID allocMem = nullptr;
	NTSTATUS ntAllocStatus = NtAllocateVirtualMemory(hProcess, &allocMem, 0, &RegionSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!NT_SUCCESS(ntAllocStatus)) {
		throw runtime_error("NtAllocateVirtualMemory failed. NTSTATUS: 0x" + to_string(ntAllocStatus));
	}

	if (nDllSize > RegionSize) {
		throw std::runtime_error("Not enough memory.");
	}

	auto NtWriteVirtualMemory = (pNtWriteVirtualMemory)GetProcAddress(hNtdll, "NtWriteVirtualMemory");
	if (!NtWriteVirtualMemory) {
		NtFreeVirtualMemory(hProcess, &allocMem, 0, MEM_RELEASE);
		throw runtime_error("Failed to get address of NtWriteVirtualMemory.");
	}

	ULONG bytesWritten = 0;
	NTSTATUS ntWriteStatus = NtWriteVirtualMemory(hProcess, allocMem, (PVOID)dllPath, (ULONG)(nDllSize), &bytesWritten);
	if (!NT_SUCCESS(ntWriteStatus)) {
		NtFreeVirtualMemory(hProcess, &allocMem, 0, MEM_RELEASE);
		throw runtime_error("NtWriteVirtualMemory failed. NTSTATUS: 0x" + to_string(ntAllocStatus));
	}

	LPVOID lpLoadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(hNtdll, "NtCreateThreadEx");
	if (!NtCreateThreadEx) {
		NtFreeVirtualMemory(hProcess, &allocMem, 0, MEM_RELEASE);
		throw runtime_error("Failed to get address of NtCreateThreadEx.");
	}

	HANDLE hThread;
	NTSTATUS ntCreateThreadStatus = NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, NULL, hProcess, (LPTHREAD_START_ROUTINE)lpLoadLibraryAddr, allocMem, 0x4, 0, 0, 0, 0);
	if (!NT_SUCCESS(ntCreateThreadStatus)) {
		NtFreeVirtualMemory(hProcess, &allocMem, 0, MEM_RELEASE);
		throw runtime_error("NtCreateThreadEx failed. NTSTATUS: 0x" + to_string(ntAllocStatus));
	}
	if (!hThread) {
		NtFreeVirtualMemory(hProcess, &allocMem, 0, MEM_RELEASE);
		throw runtime_error("Failed to create remote thread in process. Error 0x" + to_string(GetLastError()));
	}
}

void InjectorAPI::Injector::Eject(const char* moduleName) {
	if (!hProcess) {
		throw runtime_error("The process is no longer available.");
	}


}

DWORD InjectorAPI::FindProcessID(wstring processName) {
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (!hSnapshot) {
		throw runtime_error("CreateToolhelp32Snapshot failed. Error: " + GetLastError());
	}

	if (!Process32First(hSnapshot, &pEntry)) {
		CloseHandle(hSnapshot);
		throw runtime_error("Process32First failed. Error: " + GetLastError());
	}

	do {
		if (processName.compare(pEntry.szExeFile) == 0) {
			return pEntry.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &pEntry));
}

PROCESSENTRY32 InjectorAPI::FindProcess(DWORD processId) {
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, processId);
	if (!hSnapshot) {
		throw runtime_error("CreateToolhelp32Snapshot failed. Error: " + GetLastError());
	}

	if (!Process32First(hSnapshot, &pEntry)) {
		CloseHandle(hSnapshot);
		throw runtime_error("Process32First failed. Error: " + GetLastError());
	}

	do {
		if (pEntry.th32ProcessID == processId) {
			return pEntry;
		}
	} while (Process32Next(hSnapshot, &pEntry));
}

InjectorAPI::DllValidator::DllValidator(const char* path) {
	ifstream file;
	file.open(path, ios::binary);

	if (!file) {
		throw runtime_error("Ifstream file.open() failed.");
	}

	dllPath = path;
}

InjectorAPI::DllValidator::~DllValidator() {

}

string InjectorAPI::DllValidator::rawData() {
	ifstream file(dllPath, ios::binary | ios::ate);

	file.seekg(0, ios::end);
	const auto sz = file.tellg();

	if (sz <= 0) {
		throw std::runtime_error("Failed to get file size");
	}

	string buffer;
	buffer.resize(static_cast<size_t>(sz));

	file.seekg(0);
	file.read((char*)buffer.data(), sz);

	return buffer;
}

bool InjectorAPI::DllValidator::isValidDLL() {
	HANDLE hFile = CreateFileA(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	IMAGE_DOS_HEADER dosHeader;
	DWORD bytesRead;
	if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, nullptr) ||
		bytesRead != sizeof(dosHeader)) {
		CloseHandle(hFile);
		return false;
	}

	if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
		CloseHandle(hFile);
		return false;
	}

	SetFilePointer(hFile, dosHeader.e_lfanew, nullptr, FILE_BEGIN);
	IMAGE_NT_HEADERS ntHeaders;
	if (!ReadFile(hFile, &ntHeaders, sizeof(ntHeaders), &bytesRead, nullptr) ||
		bytesRead != sizeof(ntHeaders)) {
		CloseHandle(hFile);
		return false;
	}

	if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
		CloseHandle(hFile);
		return false;
	}

	CloseHandle(hFile);
	return true;
}

IMAGE_DOS_HEADER InjectorAPI::DllValidator::GetDOSHeader() {
	HANDLE hFile = CreateFileA(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		throw runtime_error("CreateFileA failed. Error: " + GetLastError());
	}

	IMAGE_DOS_HEADER dosHeader;
	DWORD bytesRead;
	if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, nullptr) ||
		bytesRead != sizeof(dosHeader)) {
		CloseHandle(hFile);
		throw runtime_error("Failed to get DOS header.");
	}

	return dosHeader;
}

bool InjectorAPI::DllValidator::isValidDOSHeader() {
	HANDLE hFile = CreateFileA(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		throw runtime_error("CreateFileA failed. Error: " + GetLastError());
	}

	IMAGE_DOS_HEADER dosHeader;
	DWORD bytesRead;
	if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, nullptr) ||
		bytesRead != sizeof(dosHeader)) {
		CloseHandle(hFile);
		return false;
	}

	return true;
}
