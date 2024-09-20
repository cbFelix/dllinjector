#include <windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <filesystem>


// Определение типа функции для NtCreateThreadEx
typedef NTSTATUS(WINAPI* pNtCreateThreadEx)(
    PHANDLE hThread,
    ACCESS_MASK DesiredAccess,
    LPVOID ObjectAttributes,
    HANDLE ProcessHandle,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    BOOL CreateSuspended,
    ULONG StackZeroBits,
    ULONG SizeOfStack,
    ULONG MaximumStackSize,
    LPVOID pUnkown);

// Преобразование строки ANSI в широкую строку
std::wstring ConvertToWideString(const std::string& str) {
    int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    if (sizeNeeded <= 0) return L"";
    std::wstring wideStr(sizeNeeded, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &wideStr[0], sizeNeeded);
    return wideStr;
}

// Создание временной папки и копирование DLL
std::wstring CreateTempFolderAndCopyDLL(const std::string& dllPath) {
    wchar_t tempPath[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, tempPath)) {
        std::cerr << "Failed to get temp path. Error: " << GetLastError() << std::endl;
        return L"";
    }

    std::wstring tempFolder = tempPath;
    tempFolder += L"TempDLL";

    if (!CreateDirectoryW(tempFolder.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        std::cerr << "Failed to create temp folder. Error: " << GetLastError() << std::endl;
        return L"";
    }

    std::wstring wideDllPath = ConvertToWideString(dllPath);
    std::wstring dllName = wideDllPath.substr(wideDllPath.find_last_of(L"\\") + 1);
    std::wstring tempDllPath = tempFolder + L"\\" + dllName;

    if (!CopyFileW(wideDllPath.c_str(), tempDllPath.c_str(), FALSE)) {
        std::cerr << "Failed to copy DLL to temp folder. Error: " << GetLastError() << std::endl;
        return L"";
    }

    return tempDllPath;
}

// Проверка разрядности процесса
bool IsProcess64Bit(HANDLE hProcess) {
    BOOL isWow64 = FALSE;
    if (!IsWow64Process(hProcess, &isWow64)) {
        std::cerr << "Failed to check process architecture. Error: " << GetLastError() << std::endl;
        return false;
    }
    return !isWow64; // Если процесс не работает через WOW64, он 64-битный
}

// Проверка разрядности текущей системы (32 или 64 бита)
bool IsSystem64Bit() {
    SYSTEM_INFO systemInfo;
    GetNativeSystemInfo(&systemInfo);
    return systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

// Функция удаления временной DLL
void CleanupTempDLL(const std::wstring& dllPath) {
    if (!DeleteFileW(dllPath.c_str())) {
        std::cerr << "Warning: Failed to remove temp DLL. Error: " << GetLastError() << std::endl;
    }
}

// Функция инъекции DLL через NtCreateThreadEx или CreateRemoteThread
bool InjectDLL(DWORD processID, const std::wstring& dllPath) {
    std::cout << "Opening target process..." << std::endl;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
        return false;
    }

    BOOL targetIs64Bit = IsProcess64Bit(hProcess);
    BOOL systemIs64Bit = IsSystem64Bit();

    if (targetIs64Bit != systemIs64Bit) {
        std::cerr << "Error: Mismatch between target process and system architecture!" << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    std::cout << "Allocating memory in target process..." << std::endl;
    size_t pathLength = (dllPath.size() + 1) * sizeof(wchar_t);
    LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, pathLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pRemoteMemory) {
        std::cerr << "Failed to allocate memory in target process. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    std::cout << "Writing DLL path to process memory..." << std::endl;
    if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPath.c_str(), pathLength, NULL)) {
        std::cerr << "Failed to write memory in target process. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    std::cout << "Getting handle to LoadLibraryW..." << std::endl;
    HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
    if (!hKernel32) {
        std::cerr << "Failed to get kernel32.dll handle. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    LPVOID pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryW");
    if (!pLoadLibrary) {
        std::cerr << "Failed to get LoadLibraryW address. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    std::cout << "Injecting DLL..." << std::endl;
    HMODULE hNtDll = GetModuleHandle(L"ntdll.dll");
    pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(hNtDll, "NtCreateThreadEx");

    HANDLE hThread = NULL;
    if (NtCreateThreadEx) {
        std::cout << "Using NtCreateThreadEx for injection..." << std::endl;
        NTSTATUS status = NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, NULL, hProcess,
            (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteMemory, FALSE, NULL, NULL, NULL, NULL);
        if (status != 0) {
            std::cerr << "NtCreateThreadEx failed. Status: " << status << std::endl;
        }
    }

    if (!hThread) {
        std::cerr << "NtCreateThreadEx failed, falling back to CreateRemoteThread." << std::endl;
        hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteMemory, 0, NULL);
        if (!hThread) {
            std::cerr << "CreateRemoteThread failed. Error: " << GetLastError() << std::endl;
            VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
            CloseHandle(hProcess);
            return false;
        }
    }

    std::cout << "Waiting for remote thread to complete..." << std::endl;
    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "DLL injected successfully. Cleaning up..." << std::endl;
    CleanupTempDLL(dllPath);  // Удаление временной DLL

    return true;
}

// Меню инжектора
void ShowMenu() {
    std::cout << "\n====== DLL Injector Menu ======" << std::endl;
    std::cout << "1. Inject DLL into a process" << std::endl;
    std::cout << "2. Exit" << std::endl;
    std::cout << "================================" << std::endl;
}

void initMessage() {
    for (char _ : "DLL Injector by keww:L\n--------------------------------\nThis project is for educational purposes only. \nThe creator does not take any responsibility for any actions taken with this software. \nUse at your own risk.\n\nThe project is protected by MIT LICENSE.") {
        std::cout << _;
        Sleep(10);
    }
}

int main() {
    initMessage();

    int choice = 0;
    while (choice != 2) {
        ShowMenu();
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        if (choice == 1) {
            DWORD processID;
            std::string dllPath;

            std::cout << "Enter the target process ID: ";
            std::cin >> processID;

            std::cout << "Enter the full path of the DLL to inject: ";
            std::cin.ignore();
            std::getline(std::cin, dllPath);

            std::wstring tempDllPath = CreateTempFolderAndCopyDLL(dllPath);
            if (tempDllPath.empty()) {
                std::cerr << "Failed to create temporary folder and copy DLL." << std::endl;
            }
            else {
                if (InjectDLL(processID, tempDllPath)) {
                    std::cout << "DLL injection successful." << std::endl;
                }
                else {
                    std::cerr << "DLL injection failed." << std::endl;
                }
            }
        }
    }
    return 0;
}
