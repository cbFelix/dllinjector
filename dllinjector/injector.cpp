#include "injector.h"

InjectorAPI::Injector::Injector(DWORD processId) {
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (!hProcess) {
		throw runtime_error("OpenProcess failed. Error: " + GetLastError());
	}

	processEntry = FindProcess(processId);
}
InjectorAPI::Injector::~Injector() {
	CloseHandle(hProcess);
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
	HANDLE hFile = CreateFileA(dllPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile == INVALID_HANDLE_VALUE) {
		return false;
	}

	IMAGE_DOS_HEADER dosHeader;
	if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), NULL, nullptr)) {
		CloseHandle(hFile);
		return false;
	}

	if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
		CloseHandle(hFile);
		return false;
	}


}


IMAGE_DOS_HEADER InjectorAPI::DllValidator::GetDOSHeader(HANDLE hFile) {
	IMAGE_DOS_HEADER dosHeader;
	if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), NULL, nullptr)) {
		CloseHandle(hFile);
		throw runtime_error("Failed to get DOS header.");
	}
	
	return dosHeader;
}